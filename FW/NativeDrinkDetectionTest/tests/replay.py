"""Compile measured raw XYZ through the exact shared C++ coordination/detectors."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile
import struct
import hashlib

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
    source+='struct R{unsigned candidates=0,shakes=0,changes=0;unsigned transitions[3100][3]={};};\nconstexpr R replay(){R r;Shake::Detector s;s.begin(0);Drink::Detector d;Shake::Gate g;bool armed=true;'
    source+='d.begin({{'+','.join(map(str,ref))+'},true},0);'
    source+='for(auto &v:data){auto old=d.phase;auto out=Drink::coordinate(s,d,g,armed,{int16_t(v[1]),int16_t(v[2]),int16_t(v[3])},v[0]);if(out==2)++r.shakes;if(out==4)++r.candidates;if(old!=d.phase||out){r.transitions[r.changes][0]=v[0];r.transitions[r.changes][1]=unsigned(d.phase);r.transitions[r.changes++][2]=out;}}return r;}\n'
    source+=f'constexpr auto result=replay();static_assert(result.candidates=={a.expected_candidates},"candidate mismatch");static_assert(result.shakes=={a.expected_shakes},"shake mismatch");\n'
    source+='static_assert(sizeof(unsigned)==4,"Replay encoding");extern const R measured __attribute__((section(".pebo_replay")))=result;\n'
    transitions=[]
    with tempfile.TemporaryDirectory(prefix='Pebo-drink-replay-') as directory:
        f=Path(directory)/'replay.cpp';f.write_text(source,encoding='utf-8')
        obj=Path(directory)/'replay.o';binary=Path(directory)/'replay.bin'
        p=subprocess.run([str(a.compiler),'-std=c++17','-Wall','-Wextra','-Werror','-c',str(f),'-o',str(obj)],capture_output=True,text=True,timeout=60)
        if not p.returncode:
            objcopy=a.compiler.with_name('arm-none-eabi-objcopy.exe' if a.compiler.suffix=='.exe' else 'arm-none-eabi-objcopy')
            subprocess.run([str(objcopy),'-O','binary','--only-section=.pebo_replay',str(obj),str(binary)],check=True,capture_output=True,timeout=20)
            encoded=binary.read_bytes();candidates,shakes,count=struct.unpack_from('<III',encoded)
            assert (candidates,shakes)==(a.expected_candidates,a.expected_shakes) and count<=len(rows)
            phases=['Idle','Tilt','Held','Return','ShakeBlocked']
            for i in range(count):
                ms,phase,event=struct.unpack_from('<III',encoded,12+i*12)
                transitions.append(dict(ms=ms,phase=phases[phase],event=event))
    report=dict(result='PASS' if not p.returncode else 'FAIL',samples=len(rows),reference=ref,
                expected_candidates=a.expected_candidates,expected_shakes=a.expected_shakes,
                source=str(a.raw),compiler_output=p.stdout+p.stderr,
                method='Actual shared C++ constexpr coordinate/detectors; extracted compiled state transitions, not intake proof',
                source_sha256=hashlib.sha256(Path(header).read_bytes()).hexdigest(),
                raw_sha256=hashlib.sha256(a.raw.read_bytes()).hexdigest(),transitions=transitions,
                quality=dict(first_ms=rows[0][0],last_ms=rows[-1][0],
                             max_gap_ms=max((y[0]-x[0] for x,y in zip(rows,rows[1:])),default=0),
                             range_endpoint_axes=sum(v in (-2048,2047) for r in rows for v in r[1:])))
    a.result.write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report))
    return int(bool(p.returncode))
if __name__=='__main__':raise SystemExit(main())
