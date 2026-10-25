"""Explicit bounded debug samples, NOT evidence of STOP/GPO wake."""
import argparse
import json
from pathlib import Path
import time
import serial
from serial.tools import list_ports

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--port',required=True)
    ap.add_argument('--results',type=Path,required=True)
    args=ap.parse_args()
    port=next((p for p in list_ports.comports() if p.device==args.port),None)
    if not port or (port.vid,port.pid)!=(0x067b,0x2303):ap.error('Expected Prolific')
    rows=[]
    with serial.Serial(args.port,115200,timeout=.1,write_timeout=1) as uart:
        uart.dtr=uart.rts=False
        for command in ['status','probe','probe','probe','status']:
            uart.write((command+'\n').encode());end=time.monotonic()+2;line=bytearray()
            while time.monotonic()<end:
                c=uart.read(1);line.extend(c)
                if c==b'\n':break
            row=dict(command=command,rx=line.decode(errors='replace').strip(),monotonic=time.monotonic())
            rows.append(row);print(json.dumps(row),flush=True);time.sleep(.1)
    args.results.parent.mkdir(parents=True,exist_ok=True)
    args.results.write_text(json.dumps(dict(scope='debug samples only, not IRQ/STOP proof',rows=rows),indent=2),encoding='utf-8')
    return 0 if all(r['rx'].startswith(('OK ','PROBE ')) for r in rows) else 1

if __name__=='__main__':raise SystemExit(main())
