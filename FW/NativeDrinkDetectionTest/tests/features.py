"""Descriptive measured-data features only; not a second recognition model."""
import argparse
import json
import math
from pathlib import Path

ap=argparse.ArgumentParser(description=__doc__)
ap.add_argument('raw',type=Path)
ap.add_argument('--reference',default='15,-1017,-32')
ap.add_argument('--result',type=Path,required=True)
a=ap.parse_args()
rows=json.loads(a.raw.read_text());ref=list(map(int,a.reference.split(',')))
normref=math.sqrt(sum(v*v for v in ref))
bins={}
for row in rows:
    t,*xyz=row;n=math.sqrt(sum(v*v for v in xyz));key=t//500
    b=bins.setdefault(key,dict(start_ms=key*500,samples=0,min_norm=4096,max_norm=0,max_tilt_degrees=0,dynamic_samples=0))
    b['samples']+=1;b['min_norm']=min(b['min_norm'],round(n,1));b['max_norm']=max(b['max_norm'],round(n,1))
    if n:
        angle=math.degrees(math.acos(max(-1,min(1,sum(x*y for x,y in zip(xyz,ref))/(n*normref)))))
        b['max_tilt_degrees']=max(b['max_tilt_degrees'],round(angle,1))
    if n<750 or n>1300:b['dynamic_samples']+=1
result=dict(source=str(a.raw),samples=len(rows),first_ms=rows[0][0],last_ms=rows[-1][0],bins=list(bins.values()),
            limits='Raw acceleration angles during dynamics are NOT reliable gravity estimates; descriptive training data only')
a.result.write_text(json.dumps(result,indent=2),encoding='utf-8')
print(json.dumps(result))
