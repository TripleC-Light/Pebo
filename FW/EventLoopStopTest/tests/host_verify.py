"""Execute ARM compiler constexpr logic checks on host; not a physical race test."""
import argparse
import ast
import json
from pathlib import Path
import subprocess

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--compiler',type=Path,required=True)
    ap.add_argument('--results',type=Path,required=True)
    args=ap.parse_args();folder=Path(__file__).resolve().parent
    command=[str(args.compiler),'-std=gnu++17','-Wall','-Wextra','-Werror','-fsyntax-only',str(folder/'LogicChecks.cpp')]
    proc=subprocess.run(command,capture_output=True,text=True,timeout=40)
    for p in folder.glob('*.py'):ast.parse(p.read_text(encoding='utf-8'))
    report=dict(result='PASS' if proc.returncode==0 else 'FAIL',command=command,output=proc.stdout+proc.stderr,
        coverage=['49 old/new mailbox interleavings in pure model','all 1..60 deadlines at midnight',
                  'saturation','retry wrap and exhaustion','20 valid held-source checks','128 service cap',
                  'repeated lifecycle','partial/range/NUL/overflow input','Python syntax'],
        limitations='Compiler-host evaluated models/policies; PRIMASK/NVIC timing and exact physical race not injected')
    args.results.parent.mkdir(parents=True,exist_ok=True)
    args.results.write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report))
    return proc.returncode

if __name__=='__main__':raise SystemExit(main())
