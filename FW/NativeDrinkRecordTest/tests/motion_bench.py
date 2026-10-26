"""Small empty-bottle integration windows; no recognition tuning or water."""
import argparse,json,time
from datetime import datetime,timezone
from pathlib import Path
from records_bench import Bench,fields,P

def main():
 a=argparse.ArgumentParser();a.add_argument('--test',choices=['calibrate','single','three','shake','cancel-off','reset','rtc-sync','retention'],required=True);a.add_argument('--results',type=Path,required=True);a.add_argument('--audio',action='store_true');a.add_argument('--clear-before',action='store_true');x=a.parse_args()
 b=Bench(x.results);result='FAIL';operator='NOT_CONFIRMED';b.open()
 try:
  b.cube('-u','0x08000000',32768,b.out/'flash32k.bin','-score','-run');image=(P/'build/NativeDrinkRecordTest.bin').read_bytes();assert (b.out/'flash32k.bin').read_bytes()[:len(image)]==image
  b.send('off');
  if x.clear_before:assert any(s.startswith('CLEARED REAL ') for s in b.send('records clear confirm'))
  before=b.get('records status','RECORDS REAL ');clock=b.get('clock','CLOCK ')
  if x.test=='rtc-sync':
   assert b.get('status','OK ')['mode']=='debug'
   now=datetime.now(timezone.utc);b.log('RTC_SYNC',source='Windows UTC, NTP not independently verified',utc=now.isoformat())
   assert 'OK setutc' in b.send('setutc '+now.strftime('%Y-%m-%d %H:%M:%S'))
   after=b.get('clock','CLOCK ');assert after['valid']=='1'
   records=b.get('records status','RECORDS REAL ');assert records['undated']==before['undated'] and records['kept']==before['kept'];assert records['ctx']!=before['ctx']
   b.send('records list');b.send('records today')
  elif x.test=='reset':
   assert int(before['kept'])>0 and clock['valid']=='1'
   b.uart.close();b.uart=None;b.cube('-rst','-score','-run');b.open();b.send('off')
   after=b.get('clock','CLOCK ');rec=b.get('records status','RECORDS REAL ')
   assert rec['kept']=='0' and rec['last']=='0' and rec['undated']=='0' and after['valid']=='1'
   for k in ('bkp0','bkp1','bkp2','bkp3','bkp4'):assert clock[k]==after[k]
   assert int(after['utc'])>=int(clock['utc'])
  elif x.test=='retention':
   for cmd in ('mode debug','off','run 1'):
    b.send(cmd);b.rx(3 if cmd=='run 1' else .1)
    rec=b.get('records status','RECORDS REAL ')
    for k in ('kept','first','last','overwrite','undated'):assert rec[k]==before[k]
   b.send('records today');b.send('ledregs');b.send('lisregs')
  else:
   # A brief run initializes sources; its natural deadline leaves debug.
   b.send('run 1');b.rx(3)
   if x.test=='calibrate':
    assert 'CALIBRATION_BEGIN keep_upright_still' in b.send('calibration start')
    assert 'CALIBRATION_OK ram_only=1' in b.rx(2.7)
    assert b.get('drink status','DRINK ')['calibration']=='1'
   else:
    assert b.get('drink status','DRINK ')['calibration']=='1'
    initial=b.get('drink status','DRINK ')
    if x.test=='shake':
     shake_before=b.get('counters','SHAKE ')
     if b.get('status','APP ')['progress']!='5':b.send('progress 5');b.rx(3.2)
    duration=40 if x.test in ('single','three') else 25
    b.log('WINDOW_ARMED',test=x.test,seconds=duration,cue='manual progress0 double flash; not recognition')
    b.send('progress 0' if x.test=='cancel-off' else 'run '+str(duration)+('' if x.test=='shake' else '\nprogress 0'),.1)
    if x.audio:
     import winsound
     winsound.Beep(1600,700);winsound.Beep(1100,700)
     b.log('PC_AUDIO_CUE',code=0,meaning='start motion after second pure tone; audibility requires operator confirmation')
    rows=[];until=time.monotonic()+duration+3;cancelled=False
    while time.monotonic()<until:
     new=b.rx(.1);rows+=new
     if x.test=='cancel-off' and not cancelled and any(s.startswith('DRINK_CANDIDATE ') for s in new):
      rows+=b.send('off');cancelled=True;until=time.monotonic()+11
    if x.test=='cancel-off' and cancelled:
     b.send('run 1');rows+=b.rx(14)
    b.send('off');current=b.get('drink status','DRINK ');rec=b.get('records status','RECORDS REAL ')
    b.send('records today');b.send('records list '+str(max(0,int(rec['kept'])-1)));b.send('ledregs');b.send('lisregs');b.send('counters')
    expected=1 if x.test in ('single','three') else 0
    assert int(rec['last'])-int(before['last'])==expected
    assert int(current['events'])-int(initial['events'])==expected
    if x.test=='single':assert int(current['candidates'])-int(initial['candidates'])==1
    if x.test=='three':assert int(current['candidates'])-int(initial['candidates'])==3
    if x.test=='cancel-off':assert cancelled and current['merge']=='0' and int(current['candidates'])-int(initial['candidates'])==1
    if x.test=='shake':
     shake_after=b.get('counters','SHAKE ');assert int(shake_after['confirmed'])-int(shake_before['confirmed'])==1
     assert int(current['candidates'])==int(initial['candidates'])
    b.log('AUTOMATED_COUNTS_PASS',test=x.test,operator_confirmation='PENDING',before=before,after=rec)
  result='PASS'
 finally:
  b.close();(b.out/'result.json').write_text(json.dumps(dict(result=result,test=x.test,bottle='EMPTY',operator_confirmation=operator,image_scope='exact current image readback; no water/no intake acceptance'),indent=2))
 print('RESULT',result)
if __name__=='__main__':main()
