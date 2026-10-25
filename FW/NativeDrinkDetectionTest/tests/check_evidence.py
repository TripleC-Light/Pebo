"""Audit saved final-image HIL invariants, separate from operator recognition claims."""
import json
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parent/'results'
SHA='09cdb9cc0d952a759e24917fd94fcdd09b01b44ad0b790af84c78a20238c3293'
checks=[]
def fields(s):return dict(re.findall(r'(\w+)=(\S+)',s))
def load(name):
    result=json.loads((ROOT/name/'result.json').read_text())
    assert result['result']=='PASS',name
    assert result['image_sha256']==SHA,name
    lines=[r['text'] for r in result['records'] if r['kind']=='RX']
    def last(prefix):return fields(next(s for s in reversed(lines) if s.startswith(prefix)))
    status,app=last('OK mode='),last('APP ')
    assert status['mode']=='debug' and status['rtc']==status['lis']==status['st']=='0/0',name
    assert app['display']==app['sampling']==app['pwm']=='0',name
    return lines,last

for name in ['20261026-returnfix-flash','20261026-auto','20261026-calibration','20261026-controls']:
    load(name);checks.append(dict(test=name,result='PASS transport and cleanup'))
for name in ['20261026-shake5a','20261026-shake5b']:
    lines,last=load(name);drink=last('DRINK ')
    assert last('SHAKE ')['confirmed']=='5'
    assert drink['candidates']==drink['events']==drink['errors']=='0'
    checks.append(dict(test=name,result='PASS five shake outputs/no drink',operator='See operator-notes.md for optics/trial confirmation'))
for name in ['20261026-three1-retry','20261026-three2','20261026-three3']:
    lines,last=load(name);drink=last('DRINK ')
    outputs=[fields(s) for s in lines if s.startswith('DRINK_CONFIRMED ')]
    candidates=[s for s in lines if s.startswith('DRINK_CANDIDATE ')]
    assert len(outputs)==1 and len(candidates)==3,name
    assert drink['candidates']=='3' and drink['events']=='1' and drink['errors']==drink['dropped']=='0'
    out=outputs[0];assert out['count']=='3' and int(out['output'])-int(out['last'])==10000
    assert not any(s=='SHAKE_CONFIRMED' for s in lines)
    replay=json.loads((ROOT/name/'replay.json').read_text());assert replay['result']=='PASS'
    checks.append(dict(test=name,result='PASS three candidates/one deferred output/exact C++ replay',samples=replay['samples']))
for name in ['20261026-awake-rf','20261026-stop-rf']:
    lines,last=load(name);gpo=last('COUNT source=gpo ');drink=last('DRINK ')
    assert int(gpo['isr'])>0 and int(gpo['confirmed'])>0 and gpo['errors']=='0'
    assert drink['candidates']==drink['events']=='0'
    if name.endswith('stop-rf'):
        assert int(gpo['wake'])>0 and last('RUN_END ')['unknownWake']=='0'
    checks.append(dict(test=name,result='PASS actual GPO evidence',gpo=gpo))
name='20261026-mixed'
lines,last=load(name);gpo=last('COUNT source=gpo ');motion=last('COUNT source=motion ')
assert int(gpo['isr'])>0 and int(gpo['wake'])>0 and int(gpo['confirmed'])>0
assert int(motion['isr'])>0 and int(motion['wake'])>0
out=[fields(s) for s in lines if s.startswith('DRINK_CONFIRMED ')]
candidate=[fields(s) for s in lines if s.startswith('DRINK_CANDIDATE ')]
assert len(out)==len(candidate)==1
assert 10000<=int(out[0]['output'])-int(out[0]['last'])<=11000
assert int(out[0]['stops'])>int(candidate[0]['stops'])
assert last('RUN_END ')['reason']=='deadline' and last('RUN_END ')['unknownWake']=='0'
checks.append(dict(test=name,result='PASS motion/GPO STOP wake, deferred merge across STOP and RTC deadline'))
for name in ['build-returnfix-20261026.json','build-final-repeat-20261026.json']:
    build=json.loads((ROOT/name).read_text());assert build['image_sha256']==SHA
report=dict(result='PASS',image_sha256=SHA,checks=checks,
            limits='Audits actual logs, not optics/trial execution/population accuracy. Operator confirmation separately recorded. No power/stack proof.')
(ROOT/'evidence-audit.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report,indent=2))
