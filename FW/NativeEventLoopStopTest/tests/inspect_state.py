"""Read RTC/backup and MCU routes only while debug/off; no flash/reset/power."""
import argparse
import json
from pathlib import Path
import re
import subprocess


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--programmer',type=Path,required=True)
    ap.add_argument('--stlink',required=True)
    ap.add_argument('--results',type=Path,required=True)
    ap.add_argument('--reference',type=Path)
    args=ap.parse_args();rows=[];words={};result='FAIL'
    try:
        for address,size in [('0x40021050','4'),('0x40002800','100'),
                             ('0x50000000','40'),('0x40010008','4')]:
            cmd=[str(args.programmer),'-c','port=SWD',f'sn={args.stlink}',
                 'mode=HOTPLUG','freq=4000','-r32',address,size,'-score','-run']
            proc=subprocess.run(cmd,capture_output=True,text=True,errors='replace',timeout=20)
            rows.append(dict(command=cmd,exit=proc.returncode,output=proc.stdout+proc.stderr))
            if proc.returncode or 'Error:' in proc.stdout+proc.stderr:raise RuntimeError('Read failed')
            for line in proc.stdout.splitlines():
                match=re.fullmatch(r'0x([0-9A-Fa-f]+) : ([0-9A-Fa-f ]+)',line.strip())
                if match:
                    for i,value in enumerate(match[2].split()):words[int(match[1],16)+i*4]=int(value,16)
        csr=words[0x40021050]
        assert csr&0x30000==0x10000 and csr&0x40000 and csr&0x200 and not csr&0x400
        assert words[0x40002810]==0x007f00ff
        assert not words[0x40002808]&((1<<10)|(1<<14))
        assert not words[0x4000280c]&(1<<10)
        assert (words[0x50000000]>>8)&3==1 and words[0x50000014]&16
        assert (words[0x5000000c]>>6)&3==0
        if args.reference:
            before=json.loads(args.reference.read_text())['registers']
            for address in [0x40002810,*range(0x40002850,0x40002864,4)]:
                assert words[address]==int(before[hex(address)],16),hex(address)
        result='PASS'
    except Exception as exc:rows.append(dict(error=repr(exc)))
    args.results.parent.mkdir(parents=True,exist_ok=True)
    args.results.write_text(json.dumps(dict(result=result,registers={hex(k):hex(v) for k,v in words.items()},
        rows=rows,limits='No valid-time assertion; backup equality plus source audit, not power-loss retention'),indent=2),encoding='utf-8')
    print(result,json.dumps({hex(k):hex(v) for k,v in words.items()}))
    return int(result!='PASS')


if __name__=='__main__':raise SystemExit(main())
