"""Finite original ST25GpoWakeTest UART capture; user-controlled supply only."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time
import serial
from serial.tools import list_ports


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--port', default='COM5')
    ap.add_argument('--programmer', type=Path, required=True)
    ap.add_argument('--stlink', required=True)
    ap.add_argument('--image', type=Path, required=True)
    ap.add_argument('--results', type=Path, required=True)
    ap.add_argument('--flash', action='store_true')
    ap.add_argument('--uart-only', action='store_true', help='Continue prior verified flash without debugger access')
    ap.add_argument('--seconds', type=int, default=35, choices=range(10, 61))
    args = ap.parse_args()
    if args.flash and args.uart_only:
        ap.error('flash and uart-only are mutually exclusive')
    port = next((p for p in list_ports.comports() if p.device == args.port), None)
    if not port or (port.vid, port.pid) != (0x067b, 0x2303):
        ap.error('Expected verified Prolific target UART')
    image = args.image.read_bytes()
    if not 0 < len(image) <= 32768:
        ap.error('Invalid image size')
    args.results.mkdir(parents=True, exist_ok=False)
    rows = []
    outcome = 'CAPTURE_ONLY'

    def record(kind, **data):
        row = dict(kind=kind, monotonic=time.monotonic(), **data)
        rows.append(row)
        print(json.dumps(row), flush=True)

    def cube(*options):
        command = [str(args.programmer), '-c', 'port=SWD', f'sn={args.stlink}',
                   'mode=UR' if args.flash else 'mode=HOTPLUG', 'freq=4000', *options]
        proc = subprocess.run(command, capture_output=True, text=True,
                              errors='replace', timeout=30)
        record('cube', command=command, exit=proc.returncode,
               output=proc.stdout + proc.stderr)
        if proc.returncode or re.search(r'^\s*Error:', proc.stdout + proc.stderr, re.M):
            raise RuntimeError('Cube operation failed')
        return proc.stdout

    try:
        record('identity', source_commit='11e2f6e', source_blob='0efdbd2be75b99bfcf87d0d1c2cf6bf2d6da1cd5',
               image_sha256=hashlib.sha256(image).hexdigest(), port=vars(port),
               power='USER_CONTROLLED', instruments='NOT_OPENED')
        if not args.uart_only and not re.search(r'Device ID\s*:\s*0x425', cube(), re.I):
            raise RuntimeError('Unexpected MCU')
        with serial.Serial(args.port, 115200, timeout=.05, write_timeout=1) as uart:
            uart.dtr = uart.rts = False
            if args.flash:
                cube('-w', str(args.image.resolve()), '0x08000000', '-v', '-rst', '-score', '-run')
            elif not args.uart_only:
                copy = args.results.resolve() / 'readback.bin'
                cube('-u', '0x08000000', str(len(image)), str(copy), '-score', '-run')
                if copy.read_bytes() != image:
                    raise RuntimeError('Different target image')
            record('WINDOW', seconds=args.seconds, action='approach_then_remove_on_operator_prompt')
            deadline = time.monotonic() + args.seconds
            buffer = bytearray()
            while time.monotonic() < deadline:
                c = uart.read(1)
                if c:
                    buffer.extend(c)
                if c == b'\n':
                    record('RX', text=buffer.decode('ascii', errors='replace').strip())
                    buffer.clear()
            if buffer:
                record('RX_PARTIAL', text=buffer.decode('ascii', errors='replace'))
        record('WINDOW_END', note='No software interrupt injected; no status reads outside sketch')
    except Exception as exc:
        outcome = 'ERROR'
        record('ERROR', error=repr(exc))
    finally:
        args.results.joinpath('session.json').write_text(json.dumps(dict(
            result=outcome, records=rows,
            limitations='Original WAKE line combines ISR flag with pin level; no explicit ISR count. '
                        'No UART command parser. Absence of UART wake is not hardware failure proof.',
            power='unchanged, user controlled'), indent=2, ensure_ascii=False), encoding='utf-8')
    return int(outcome == 'ERROR')


if __name__ == '__main__':
    raise SystemExit(main())
