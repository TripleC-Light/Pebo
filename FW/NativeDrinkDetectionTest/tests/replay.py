"""Compile measured raw XYZ through the exact shared C++ coordination/detectors."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('raw',type=Path)
    ap.add_argument('--reference',default='15,-1017,-32')
    ap.add_argument('--expected-candidates',type=int,required=True)
    ap.add_argument('--expected-shakes',type=int,required=True)
    ap.add_argument('--compiler',type=Path,required=True)
    ap.add_argument('--result',type=Path,required=True)
    a=ap.parse_args()
    rows=json.loads(a.raw.read_text());ref=list(map(int,a.reference.split(',')))
    assert len(ref)==3 and 1<=len(rows)<=3100
    assert all(len(r)==4 and 0<=r[0]<30000 and all(-2048<=v<=2047 for v in r[1:]) for r in rows)
    assert all(x[0]<y[0] for x,y in zip(rows,rows[1:]))
    header=(Path(__file__).resolve().parents[1]/'src/DrinkLogic.h').as_posix()
    source=f'#include "{header}"\nconstexpr int data[][4]={{'+','.join('{'+','.join(map(str,r))+'}' for r in rows)+'};\n'
    source+='struct R{unsigned candidates=0,shakes=0;};\nconstexpr R replay(){R r;Shake::Detector s;s.begin(0);Drink::Detector d;Shake::Gate g;bool armed=true;'
    source+='d.begin({{'+','.join(map(str,ref))+'},true},0);'
    source+='for(auto &v:data){auto out=Drink::coordinate(s,d,g,armed,{int16_t(v[1]),int16_t(v[2]),int16_t(v[3])},v[0]);if(out==2)++r.shakes;if(out==4)++r.candidates;}return r;}\n'
    source+=f'constexpr auto result=replay();static_assert(result.candidates=={a.expected_candidates},"candidate mismatch");static_assert(result.shakes=={a.expected_shakes},"shake mismatch");\n'
    with tempfile.TemporaryDirectory(prefix='Pebo-drink-replay-') as directory:
        f=Path(directory)/'replay.cpp';f.write_text(source,encoding='utf-8')
        p=subprocess.run([str(a.compiler),'-std=c++17','-Wall','-Wextra','-Werror','-fsyntax-only',str(f)],capture_output=True,text=True,timeout=60)
    report=dict(result='PASS' if not p.returncode else 'FAIL',samples=len(rows),reference=ref,
                expected_candidates=a.expected_candidates,expected_shakes=a.expected_shakes,
                source=str(a.raw),compiler_output=p.stdout+p.stderr,method='Actual shared C++ coordinate and detectors; measured empty-bottle data, not intake proof')
    a.result.write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report))
    return int(bool(p.returncode))
if __name__=='__main__':raise SystemExit(main())
