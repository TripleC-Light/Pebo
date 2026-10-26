"""Finite full-diagnostic record pipeline; shared selected-device transport."""
import argparse,hashlib,json,re
from pathlib import Path
from records_bench import Bench,P,fields

def main():
 a=argparse.ArgumentParser();a.add_argument('--results',type=Path,required=True);a.add_argument('--flash',action='store_true');a.add_argument('--config',choices=['diagnostic'],required=True);x=a.parse_args()
 b=Bench(x.results);result='FAIL';image=P/'build/diagnostic/NativeDrinkRecordTest-diagnostic.bin'
 try:
  b.open();before=b.get('clock','CLOCK ');b.send('off');b.uart.close();b.uart=None
  b.cube('-u','0x08000000','32768',b.out/'before32k.bin','-score','-run')
  if x.flash:b.cube('reset=HWrst','-w',image.resolve(),'0x08000000','-v','-rst','-score','-run',mode='UR')
  b.cube('-u','0x08000000','32768',b.out/'flash32k.bin','-score','-run')
  readback=(b.out/'flash32k.bin').read_bytes();assert readback[:image.stat().st_size]==image.read_bytes()
  b.log('image',bytes=image.stat().st_size,sha256=hashlib.sha256(image.read_bytes()).hexdigest(),flash32k_sha256=hashlib.sha256(readback).hexdigest())
  expected=image.read_bytes();prior=(b.out/'before32k.bin').read_bytes();page_end=(len(expected)+127)//128*128
  assert readback==expected+bytes(page_end-len(expected))+prior[page_end:]
  b.open();assert b.get('status','APP ')['config']=='diagnostic'
  if not x.flash:b.send('run 1');b.rx(3)
  app=b.get('status','APP ');assert app['version']=='NativeDrinkRecordTest-v1'
  assert 'ERR test_requires_off_idle' in b.send('records test enter')
  b.send('off');before_real=b.get('records status','RECORDS REAL ');before_drink=b.get('drink status','DRINK ')
  assert 'TEST_BEGIN SYNTHETIC RTC_unchanged' in b.send('records test enter')
  assert 'ERR record_test_active' in b.send('run 1')
  assert 'TEST_ADDED SYNTHETIC candidates=2 intake_unproven=1' in b.send('records test add')
  days=[fields(s) for s in b.send('records today') if s.startswith('DAY SYNTHETIC ')]
  assert len(days)==2 and sorted((d['events'],d['current']) for d in days)==[('0','1'),('1','0')]
  for _ in range(39):assert 'TEST_ADDED SYNTHETIC candidates=2 intake_unproven=1' in b.send('records test add',.05)
  status=b.get('records status','RECORDS SYNTHETIC ')
  assert (status['kept'],status['first'],status['last'],status['overwrite'])==('32','9','40','8')
  days=[fields(s) for s in b.send('records today') if s.startswith('DAY SYNTHETIC ')]
  assert next(d for d in days if d['current']=='1')['events']=='39'
  pages=[]
  for i in range(32):
   ls=b.send('records list '+str(i));assert any(s=='LIST_END SYNTHETIC' for s in ls)
   e=fields(next(s for s in ls if s.startswith('RECORD SYNTHETIC ')));assert int(e['seq'])==i+9;pages.append(e)
  assert 'ERR invalid' in b.send('records list 32')
  assert 'ERR invalid' in b.send('records clear')
  assert 'ERR too_long' in b.send('x'*80)
  assert 'ERR invalid' in b.send(b'records\x00 status')
  b.uart.write(b'records sta');assert not b.rx(.1);b.uart.write(b'tus\n');assert any(s.startswith('RECORDS SYNTHETIC ') for s in b.rx(.2))
  b.send('records test day');b.send('records test add');b.send('records today')
  assert any(s.startswith('CLEARED SYNTHETIC ') for s in b.send('records clear confirm'))
  assert b.get('records status','RECORDS SYNTHETIC ')['kept']=='0'
  b.send('records test exit')
  assert b.get('records status','RECORDS REAL ')==before_real
  assert b.get('drink status','DRINK ')==before_drink
  after=b.get('clock','CLOCK ')
  for key in ('valid','bkp0','bkp1','bkp2','bkp3','bkp4'):assert before[key]==after[key],(key,before,after)
  assert int(after['utc'])>=int(before['utc'])
  b.send('run 1');lines=b.rx(4);assert any(s.startswith('RUN_END reason=deadline') for s in lines)
  b.send('ledregs');b.send('lisregs');b.send('records status');result='PASS'
 finally:
  b.close();(b.out/'result.json').write_text(json.dumps(dict(result=result,kind='SYNTHETIC_RECORD_PIPELINE_NOT_DRINK_RECOGNITION'),indent=2))
 print('RESULT',result)
if __name__=='__main__':main()
