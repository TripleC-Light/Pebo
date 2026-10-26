"""Check real format literals against storage/admission bounds using typed limits."""
import ast,re,json
from pathlib import Path
P=Path(__file__).resolve().parents[1]
s=(P/'src/main.cpp').read_text(encoding='utf8')
formats=[ast.literal_eval('"'+v+'"') for v in re.findall(r'Text::format\(\w+,sizeof\(\w+\),"((?:\\.|[^"\\])*)"',s)]
def size(prefix,values):
 fmt=next(f for f in formats if f.startswith(prefix))
 return len((re.sub(r'%([0-9]*)(?:l)([duX])',r'%\1\2',fmt)%tuple(values)).encode())
M=4294967295
counts=size('COUNT ',['motion']+[M]*6)
shake=size('SHAKE ',[M]*6+[255,M])
base=size('OK mode=',['debug']+[1]*10+[M,M,-1,M,M,'rtc_probe_timeout'])
app=size('APP ',['diagnostic',15,8,1,255,1,1,1,1,320])
drink=size('DRINK calibration=',[1,-32768,-32768,-32768,3,1]+[M]*10)
rec=size('RECORDS ',['SYNTHETIC',M,M,-840,1,32]+[M]*6+[1,32,1156])
today=size('TODAY ',['SYNTHETIC',2099,12,31,M,M,-840,1,M])
day=size('DAY ',['SYNTHETIC',M,M,-840,M,M,M,1])
row=size('RECORD ',['SYNTHETIC']+[M]*7+[-840,1,'UTC_seconds'])
assert counts<160 and shake<192 and base<256 and app<256 and drink<288 and rec<256 and today<256 and day<256 and row<240
assert base+app+drink<900
assert 3*counts+len('END counters\r\n')+shake<900
assert today+4*day+122+60<900
# The largest deferred row plus one asynchronous bounded event still fits reserved tail.
assert row+100<1023
r=dict(result='PASS',count=counts,shake=shake,status=base,app=app,drink=drink,records=rec,today=today,day=day,row=row,status_block=base+app+drink,counters_block=3*counts+14+shake,four_summary_block=today+4*day,admission=900,tx_usable=1023)
(P/'resource-evidence/uart-bounds.json').write_text(json.dumps(r,indent=2));print(json.dumps(r))
