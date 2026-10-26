"""Bounded awake/STOP RF regression; user moves NFC phone, no NFC protocol."""
import argparse,json,time,winsound
from pathlib import Path
from records_bench import Bench,fields,P

def main():
 a=argparse.ArgumentParser();a.add_argument('--test',choices=['awake','stop'],required=True);a.add_argument('--results',type=Path,required=True);x=a.parse_args();b=Bench(x.results);result='FAIL'
 try:
  b.open();b.cube('-u','0x08000000',32768,b.out/'flash32k.bin','-score','-run');image=(P/'build/NativeDrinkRecordTest.bin').read_bytes();assert (b.out/'flash32k.bin').read_bytes()[:len(image)]==image
  b.send('off');real=b.get('records status','RECORDS REAL ');drink=b.get('drink status','DRINK ');clock=b.get('clock','CLOCK ')
  b.send('run 1');b.rx(3)
  def gpo():return fields(next(s for s in b.send('counters') if s.startswith('COUNT source=gpo ')))
  before=gpo();allrows=[]
  if x.test=='stop':b.send('run 25');b.rx(4)
  b.log('WINDOW_READY',test=x.test,seconds=18,cue='PC two-tone, phone near/remove; bottle stationary')
  winsound.Beep(1600,700);winsound.Beep(1100,700)
  end=time.monotonic()+18;nextquery=0
  while time.monotonic()<end:
   allrows+=b.rx(.15)
   if x.test=='awake' and time.monotonic()>nextquery:
    allrows+=b.send('records list 1');allrows+=b.send('records today');nextquery=time.monotonic()+2
  if x.test=='stop':allrows+=b.rx(5)
  after=gpo();b.send('off');rec=b.get('records status','RECORDS REAL ');now=b.get('drink status','DRINK ');rtc=b.get('clock','CLOCK ')
  assert int(after['isr'])>int(before['isr']) and int(after['confirmed'])>int(before['confirmed']) and after['errors']==before['errors']
  if x.test=='stop':assert int(after['wake'])>int(before['wake']) and any(s.startswith('RUN_END reason=deadline') for s in allrows)
  assert rec==real and now==drink
  for key in ('valid','bkp0','bkp1','bkp2','bkp3','bkp4'):assert rtc[key]==clock[key]
  b.log('RF_PASS',before=before,after=after,record_change=0);result='PASS'
 finally:
  b.close();(b.out/'result.json').write_text(json.dumps(dict(result=result,test=x.test,operator_confirmation='PENDING',scope='RF field wake only, no NFC application protocol'),indent=2))
if __name__=='__main__':main()
