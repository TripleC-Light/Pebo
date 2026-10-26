"""Profile-specific finite hardware checks; no instruments or backup reset."""
import argparse,json,hashlib,time
from pathlib import Path
from records_bench import Bench,P,fields

def main():
 a=argparse.ArgumentParser();a.add_argument('--config',choices=['diagnostic','compact'],required=True);a.add_argument('--test',choices=['capture','auto','calibrate','single','shake','cancel-off','reset','retention','awake','stop','final'],required=True);a.add_argument('--flash',action='store_true');a.add_argument('--results',type=Path,required=True);x=a.parse_args()
 b=Bench(x.results);result='FAIL';image=(P/'build'/x.config/('NativeDrinkRecordTest-'+x.config+'.bin')).read_bytes()
 try:
  if x.flash:
   b.cube('-u','0x08000000',32768,b.out/'before32k.bin','-score','-run');prior=(b.out/'before32k.bin').read_bytes()
   b.cube('reset=HWrst','-w',(P/'build'/x.config/('NativeDrinkRecordTest-'+x.config+'.bin')).resolve(),'0x08000000','-v','-rst','-score','-run',mode='UR')
  b.cube('-u','0x08000000',32768,b.out/'flash32k.bin','-score','-run');actual=(b.out/'flash32k.bin').read_bytes();assert actual[:len(image)]==image
  if x.flash:
   page_end=(len(image)+127)//128*128;assert actual==image+bytes(page_end-len(image))+prior[page_end:]
  b.log('IMAGE',config=x.config,bytes=len(image),sha256=hashlib.sha256(image).hexdigest(),full32k_sha256=hashlib.sha256(actual).hexdigest())
  b.open();b.send('off');assert b.get('status','APP ')['config']==x.config
  before=b.get('records status','RECORDS REAL ');clock=b.get('clock','CLOCK ')
  def initialize():b.send('run 1');b.rx(3)
  def cue():
   import winsound
   winsound.Beep(1600,700);winsound.Beep(1100,700);b.log('CUE',meaning='after second PC tone',operator_confirmation='PENDING')
  def unchanged():
   r=b.get('records status','RECORDS REAL ')
   for k in ('kept','first','last','overwrite','undated'):assert r[k]==before[k],(k,r,before)
   return r
  if x.test=='capture':
   assert x.config=='diagnostic';initialize()
   assert 'CALIBRATION_BEGIN keep_upright_still' in b.send('calibration start');assert 'CALIBRATION_OK ram_only=1' in b.rx(2.7)
   rows=b.send('capture 5')+b.rx(6);assert sum(s.startswith('RAW ') for s in rows)>400
   rows=b.send('trace')+b.rx(3);assert any(s.startswith('TRACE_END') for s in rows);assert sum(s.startswith('SAMPLE ') for s in rows)>0
   assert b.get('drink status','DRINK ')['dropped']=='0';unchanged();b.send('lisregs');b.send('ledregs')
  elif x.test=='auto':
   assert x.config=='compact'
   for c in ('capture 5','trace','records test enter','records test add','records test day','records test exit','ledregs','lisregs','probe','observe','counters clear'):
    assert 'ERR invalid' in b.send(c),c
   assert 'ERR too_long' in b.send('x'*80);assert 'ERR invalid' in b.send(b'records\x00 status')
   b.uart.write(b'records sta');assert not b.rx(.1);b.uart.write(b'tus\n');assert any(s.startswith('RECORDS REAL ') for s in b.rx(.2))
   assert 'ERR invalid' in b.send('records list 32');assert 'ERR invalid' in b.send('records clear')
   for n in (10,60):
    b.send('run '+str(n));rows=b.rx(n-1)
    # Concurrent bounded queries near deadline exercise deferred automatic counters.
    rows+=b.send('status')+b.send('records today')+b.send('counters')+b.rx(3)
    assert any(s.startswith('RUN_END reason=deadline') for s in rows)
    assert sum(s=='END counters' for s in rows)>=1
    unchanged()
   b.send('records today');b.send('records list');b.send('help')
  elif x.test=='calibrate':
   initialize();assert 'CALIBRATION_BEGIN keep_upright_still' in b.send('calibration start');assert 'CALIBRATION_OK ram_only=1' in b.rx(2.7)
  elif x.test in ('single','shake','cancel-off'):
   initialize();assert b.get('drink status','DRINK ')['calibration']=='1';initial=b.get('drink status','DRINK ')
   if x.test=='shake':b.send('progress 5');b.rx(3.3)
   n=40 if x.test=='single' else 25
   if x.test!='cancel-off':b.send('run '+str(n))
   if x.test!='shake':b.send('progress 0')
   cue();rows=[];end=time.monotonic()+n+3;cancelled=False
   while time.monotonic()<end:
    new=b.rx(.1);rows+=new
    if x.test=='cancel-off' and not cancelled and any(s.startswith('DRINK_CANDIDATE ') for s in new):
     rows+=b.send('off');cancelled=True;end=time.monotonic()+11
   if x.test=='cancel-off':assert cancelled;b.send('run 1');rows+=b.rx(14)
   b.send('off');now=b.get('drink status','DRINK ');rec=b.get('records status','RECORDS REAL ')
   expected=int(x.test=='single');assert int(rec['last'])-int(before['last'])==expected;assert int(now['events'])-int(initial['events'])==expected
   if x.test in ('single','cancel-off'):assert int(now['candidates'])-int(initial['candidates'])==1
   if x.test=='shake':assert sum(s=='SHAKE_CONFIRMED' for s in rows)==1;assert now['candidates']==initial['candidates']
   assert now['merge']=='0';b.send('records today');b.send('records list');b.log('COUNTS_PASS',operator_confirmation='PENDING')
  elif x.test=='retention':
   for c in ('mode debug','off','run 1'):b.send(c);b.rx(3 if c=='run 1' else .1);unchanged()
  elif x.test=='reset':
   assert int(before['kept'])>0 and clock['valid']=='1';b.uart.close();b.uart=None;b.cube('-rst','-score','-run');b.open();b.send('off')
   rec=b.get('records status','RECORDS REAL ');assert rec['kept']=='0' and rec['last']=='0';after=b.get('clock','CLOCK ')
   for k in ('valid','bkp0','bkp1','bkp2','bkp3','bkp4'):assert clock[k]==after[k]
   assert int(after['utc'])>=int(clock['utc'])
  elif x.test in ('awake','stop'):
   initialize()
   def gpo():return fields(next(s for s in b.send('counters') if s.startswith('COUNT source=gpo ')))
   old=gpo();rows=[]
   if x.test=='stop':b.send('run 25');b.rx(4)
   cue();end=time.monotonic()+18
   while time.monotonic()<end:
    rows+=b.rx(.2)
    if x.test=='awake':rows+=b.send('records today');rows+=b.send('records list');b.rx(.5)
   if x.test=='stop':rows+=b.rx(5)
   new=gpo();assert int(new['isr'])>int(old['isr']) and int(new['confirmed'])>int(old['confirmed']) and new['errors']==old['errors']
   if x.test=='stop':assert int(new['wake'])>int(old['wake']);assert any(s.startswith('RUN_END reason=deadline') for s in rows)
   unchanged();b.log('RF_PASS',before=old,after=new,operator_confirmation='PENDING')
  elif x.test=='final':
   b.send('mode debug');s=b.get('status','OK ');assert s['mode']=='debug' and s['lpd']=='1' and s['rtc']==s['lis']==s['st']=='0/0'
   app=b.get('status','APP ');assert app['pwm']==app['display']==app['sampling']=='0';b.send('clock');b.send('records today')
  result='PASS'
 finally:
  b.close();(b.out/'result.json').write_text(json.dumps(dict(result=result,test=x.test,config=x.config,operator_confirmation='PENDING' if x.test in ('single','shake','cancel-off','awake','stop') else 'NOT_REQUIRED',scope='EMPTY or stationary; actual intake unvalidated'),indent=2))
 print('RESULT',result)
if __name__=='__main__':main()
