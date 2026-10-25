"""Replay measured HIL trace through actual C++ detector, using constexpr GCC evaluation."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("trace",type=Path)
    ap.add_argument("--compiler",type=Path,required=True)
    ap.add_argument("--expected",choices=["confirmed","rejected"],required=True)
    ap.add_argument("--result",type=Path,required=True)
    args=ap.parse_args()
    rows=json.loads(args.trace.read_text())
    assert 5<=len(rows)<=160 and all(len(r)==4 for r in rows)
    assert all(0<=r[0]<1500 and all(-2048<=v<=2047 for v in r[1:]) for r in rows)
    assert all(a[0]<b[0] for a,b in zip(rows,rows[1:]))
    header=(Path(__file__).resolve().parents[1]/"src/ShakeLogic.h").as_posix()
    fixture='#include "'+header+'"\nconstexpr int data[][4]={'+','.join('{'+','.join(map(str,r))+'}' for r in rows)+'};\n'
    fixture+='constexpr bool replay(){Shake::Detector d;d.begin(0);unsigned hits=0;for(auto &r:data)if(d.feed({int16_t(r[1]),int16_t(r[2]),int16_t(r[3])},r[0]))++hits;return hits=='
    fixture+=('1' if args.expected=="confirmed" else '0')+';}\nstatic_assert(replay(),"Measured trace mismatch");\n'
    with tempfile.TemporaryDirectory(prefix="Pebo-trace-") as temp:
        source=Path(temp)/"replay.cpp";source.write_text(fixture,encoding="utf-8")
        p=subprocess.run([str(args.compiler),"-std=c++17","-Wall","-Wextra","-Werror","-fsyntax-only",str(source)],capture_output=True,text=True,timeout=30)
    report=dict(result="PASS" if p.returncode==0 else "FAIL",samples=len(rows),expected=args.expected,
                trace=str(args.trace),compiler_output=p.stdout+p.stderr,method="Actual detector constexpr on measured XYZ/timestamps, not synthetic data")
    args.result.write_text(json.dumps(report,indent=2),encoding="utf-8");print(json.dumps(report))
    return int(p.returncode!=0)
if __name__=="__main__":raise SystemExit(main())
