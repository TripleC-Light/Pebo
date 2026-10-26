import argparse,json,subprocess,sys
from pathlib import Path
P=Path(__file__).resolve().parents[1]
a=argparse.ArgumentParser();a.add_argument('--toolchain-bin',type=Path,required=True);x=a.parse_args()
items=json.loads((P/'resource-evidence/replay-batch.json').read_text());out=P/'resource-evidence/replay';out.mkdir(exist_ok=True)
allresults=[]
for config in ['diagnostic','compact']:
 for item in items:
  result=out/(config+'-'+item['name']+'.json')
  c=[sys.executable,str(P/'tests/resource_replay.py'),item['raw'],'--config',config,'--reference',','.join(map(str,item['reference'])),'--expected-candidates',str(item['candidates']),'--expected-shakes',str(item['shakes']),'--compiler',str(x.toolchain_bin/'arm-none-eabi-g++.exe'),'--result',str(result)]
  r=subprocess.run(c,capture_output=True,text=True,timeout=90);assert r.returncode==0,(r.stdout+r.stderr)
  data=json.loads(result.read_text());allresults.append(dict(config=config,name=item['name'],result=data['result'],samples=data['samples'],raw_sha256=data['raw_sha256']))
  print(config,item['name'],data['result'],flush=True)
for item in items:
 a=json.loads((out/('diagnostic-'+item['name']+'.json')).read_text());b=json.loads((out/('compact-'+item['name']+'.json')).read_text());assert a['transitions']==b['transitions']
(P/'resource-evidence/replay-summary.json').write_text(json.dumps(dict(result='PASS',clips_per_profile=len(items),runs=allresults,scope='saved EMPTY motion, not real intake'),indent=2))
