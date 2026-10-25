"""Read MCU route after run1, then off. Route proof is NOT physical GPO proof."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import time
import serial

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--port',required=True)
    ap.add_argument('--programmer',type=Path,required=True)
    ap.add_argument('--stlink',required=True)
    ap.add_argument('--results',type=Path,required=True)
    args=ap.parse_args();rows=[];words={};result='FAIL'
    def line(uart,timeout=4):
        end=time.monotonic()+timeout;data=bytearray()
        while time.monotonic()<end:
            c=uart.read(1);data.extend(c)
            if c==b'\n':break
        text=data.decode(errors='replace').strip();rows.append(dict(rx=text));return text
    with serial.Serial(args.port,115200,timeout=.05,write_timeout=1) as uart:
        uart.dtr=uart.rts=False
        try:
            uart.write(b'run 1\n');assert line(uart).startswith('RUN_BEGIN')
            assert line(uart)=='RUN_READY rtc_probe=1'
            assert line(uart).startswith('RUN_END reason=deadline')
            for _ in range(3):assert line(uart).startswith('COUNT')
            assert line(uart)=='END counters'
            uart.write(b'probe\n');assert line(uart).startswith('PROBE')
            for address,size in [('0x50000000','40'),('0x50000400','40'),
                                 ('0x40010008','4'),('0x40010400','24'),('0xe000e100','4')]:
                command=[str(args.programmer),'-c','port=SWD',f'sn={args.stlink}',
                    'mode=HOTPLUG','freq=4000','-r32',address,size,'-score','-run']
                proc=subprocess.run(command,capture_output=True,text=True,errors='replace',timeout=20)
                rows.append(dict(command=command,exit=proc.returncode,output=proc.stdout+proc.stderr))
                if proc.returncode or 'Error:' in proc.stdout+proc.stderr:raise RuntimeError('Read failed')
                for text in proc.stdout.splitlines():
                    match=re.fullmatch(r'0x([0-9a-fA-F]+) : ([0-9a-fA-F ]+)',text.strip())
                    if match:
                        for i,value in enumerate(match[2].split()):words[int(match[1],16)+4*i]=int(value,16)
            assert (words[0x50000000]>>6)&3==0 and (words[0x5000000c]>>6)&3==0
            assert (words[0x50000000]>>8)&3==1 and words[0x50000014]&(1<<4)
            assert words[0x50000400]&3==0
            assert words[0x40010008]&15==1 and (words[0x40010008]>>12)&15==0
            assert words[0x40010400]&9==9 and words[0x40010408]&9==9
            # STM32L031 EXTI0_1 IRQ5 / EXTI2_3 IRQ6, verified CMSIS enum.
            assert words[0xe000e100]&0x60==0x60
            result='PASS_MCU_ROUTES_ONLY'
        except Exception as exc:rows.append(dict(error=repr(exc)))
        finally:
            uart.write(b'off\n');rows.append(dict(cleanup=line(uart)))
    args.results.parent.mkdir(parents=True,exist_ok=True)
    args.results.write_text(json.dumps(dict(result=result,registers={hex(k):hex(v) for k,v in words.items()},
        rows=rows),indent=2),encoding='utf-8')
    print(result,json.dumps({hex(k):hex(v) for k,v in words.items()}))
    return 0 if result.startswith('PASS') else 1

if __name__=='__main__':raise SystemExit(main())
