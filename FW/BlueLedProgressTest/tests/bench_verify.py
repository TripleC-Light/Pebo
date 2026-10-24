"""Explicit Pebo-only flash/UART/register check; PPK2 supplies power, never samples current."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import struct
import sys
import time

import serial
from serial.tools import list_ports

REPO = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO / 'Software' / 'PPK2Automation'))
from battery_voltage_sweep import CheckedPPK2, output_off

GPIOA = 0x50000000
TIM2 = 0x40000000
TIM22 = 0x40011400
PINS = (5, 6, 7, 8)
PATTERNS = ('LOOO', 'LOOO', 'HOOO', 'HLOO', 'HHOO', 'HHLO', 'HHHO', 'HHHL', 'HHHH')


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--ppk-port', required=True)
    ap.add_argument('--uart-port', required=True)
    ap.add_argument('--stlink', required=True)
    ap.add_argument('--programmer', type=Path, required=True)
    ap.add_argument('--image', type=Path, required=True)
    ap.add_argument('--results', type=Path, required=True)
    ap.add_argument('--confirm-sole-supply', action='store_true', required=True)
    ap.add_argument('--visual', action='store_true')
    ap.add_argument('--verify-existing', action='store_true', help='Compare existing flash to image instead of rewriting')
    ap.add_argument('--swd-replies', type=Path, metavar='ARM_NM', help='Diagnostic: verify MCU TX ring via SWD; PC UART RX remains unverified')
    args = ap.parse_args()
    if not args.image.is_file() or not 0 < args.image.stat().st_size <= 32768:
        ap.error('Expected existing <=32KB standalone binary')
    ports = {p.device: p for p in list_ports.comports()}
    if args.ppk_port not in ports or ports[args.ppk_port].vid != 0x1915:
        ap.error('PPK port must be a currently enumerated Nordic interface')
    if args.uart_port not in ports or ports[args.uart_port].vid != 0x067B:
        ap.error('Expected the currently verified Prolific UART')
    args.results.mkdir(parents=True, exist_ok=False)
    records = []
    ppk = None
    status = 'FAIL'
    off_sent = False
    exit_code = 1
    tx_symbols = {}
    last_tx = 0
    if args.swd_replies:
        symbols = subprocess.run([str(args.swd_replies), '-S', '-C', str(args.image.with_suffix('.elf'))],
                                 capture_output=True, text=True, check=True, timeout=10).stdout
        for line in symbols.splitlines():
            match = re.fullmatch(r'([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+\w\s+(tx|txWrite|txRead)', line)
            if match:
                tx_symbols[match[2]] = int(match[1], 16)
        if set(tx_symbols) != {'tx', 'txWrite', 'txRead'}:
            ap.error('Required symbols absent from the matching ELF')

    def record(kind, **data):
        row = dict(kind=kind, monotonic=time.monotonic(), **data)
        records.append(row)
        print(json.dumps(row), flush=True)

    def cube(*commands):
        result = subprocess.run([str(args.programmer), *commands],
                                capture_output=True, text=True, errors='replace', timeout=25)
        record('cube', arguments=list(commands), exit=result.returncode, output=result.stdout + result.stderr)
        if result.returncode or re.search(r'^\s*Error:', result.stdout + result.stderr, re.MULTILINE):
            raise RuntimeError('CubeProgrammer failed')
        return result.stdout

    def connection(mode='HOTPLUG'):
        return ['-c', 'port=SWD', f'sn={args.stlink}', f'mode={mode}', 'freq=4000']

    def regs(address, count):
        path = args.results.resolve() / f'registers-{len(records)}-{address:08x}.bin'
        cube(*connection(), '-u', hex(address), str(count * 4), str(path), '-score', '-run')
        data = path.read_bytes()
        if len(data) != count * 4:
            raise RuntimeError('Incomplete register read; do not infer hardware state')
        values = {address + 4 * i: value for i, value in enumerate(struct.unpack(f'<{count}I', data))}
        record('registers', values={hex(k): hex(v) for k, v in values.items()})
        return values

    def gpio(expected):
        values = regs(GPIOA, 10)
        for i, pin in enumerate(PINS):
            mode = (values[GPIOA] >> (pin * 2)) & 3
            level = (values[GPIOA + 0x14] >> pin) & 1
            assert mode == (2 if expected[i] == 'L' else 1), (expected, pin, mode)
            if expected[i] != 'L':
                assert level == int(expected[i] == 'H'), (expected, pin, level)
            else:
                af = (values[GPIOA + 0x20 + 4 * (pin // 8)] >> (4 * (pin % 8))) & 15
                assert af == 5, (pin, af)
        record('PASS GPIO', pattern=expected)

    try:
        ppk = CheckedPPK2(args.ppk_port, timeout=0.1, write_timeout=1)
        output_off(ppk)
        ppk.stop_measuring()
        for _ in range(20):
            time.sleep(0.1)
            ppk.get_data()
        ppk.ser.reset_input_buffer()
        if not ppk.get_modifiers() or not ppk.modifiers.get('HW'):
            raise RuntimeError('PPK metadata invalid; output remains OFF')
        record('PPK metadata', hardware=ppk.modifiers['HW'], port=args.ppk_port)
        ppk.use_source_meter()
        ppk.set_source_voltage(2800)
        with serial.Serial(args.uart_port, 115200, timeout=0.1, write_timeout=1) as uart:
            uart.dtr = False
            uart.rts = False
            ppk.toggle_DUT_power('ON')
            record('POWER ON', millivolts=2800)
            time.sleep(0.5)
            probe = cube(*connection())
            if not re.search(r'Device ID\s*:\s*0x425', probe, re.I):
                raise RuntimeError('Not the expected STM32L031 family; no flash')
            if args.verify_existing:
                readback = args.results.resolve() / 'existing-firmware.bin'
                cube(*connection(), '-u', '0x08000000', str(args.image.stat().st_size), str(readback))
                if readback.read_bytes() != args.image.read_bytes():
                    raise RuntimeError('Existing firmware differs; not running an unverified image')
                cube(*connection(), '-rst', '-score', '-run')
            else:
                cube(*connection('UR'), 'reset=HWrst', '-w', str(args.image), '0x08000000', '-v', '-rst', '-score', '-run')
            time.sleep(0.3)
            uart.reset_input_buffer()

            def send(data, expected):
                nonlocal last_tx
                uart.write(data)
                deadline = time.monotonic() + (0.2 if args.swd_replies else 2)
                line = bytearray()
                while time.monotonic() < deadline:
                    byte = uart.read(1)
                    if byte:
                        line.extend(byte)
                        if byte == b'\n':
                            break
                text = line.decode('ascii', errors='replace').strip()
                record('UART', tx=data.hex(), rx=text)
                if args.swd_replies:
                    head = regs(tx_symbols['txWrite'], 1)[tx_symbols['txWrite']] & 0xFFFF
                    words = regs(tx_symbols['tx'], 64)
                    ring = b''.join(words[tx_symbols['tx'] + i * 4].to_bytes(4, 'little') for i in range(64))
                    reply = bytes(ring[i % 256] for i in range(last_tx, last_tx + (head - last_tx) % 256))
                    last_tx = head
                    mcu_text = reply.decode('ascii', errors='replace').strip()
                    record('MCU TX via SWD', text=mcu_text, pc_rx_verified=text == expected)
                    assert mcu_text == expected, (mcu_text, expected)
                else:
                    assert text == expected, (text, expected)

            def level(value):
                send(f'{value}\n'.encode(), f'OK level level={value} pattern={PATTERNS[value]}')

            gpio('OOOO')
            send(b'help\n', 'OK help commands=0..8,show,off,help')
            for value in range(1, 9):
                level(value)
                gpio(PATTERNS[value])
                if value & 1:
                    level(value)
                    timer = TIM2 if value in (1, 7) else TIM22
                    channel = 2 if value == 5 else 1
                    values = regs(timer, 15)
                    assert values[timer] & 1
                    ratio = values[timer + 0x34 + 4 * (channel - 1)] / (values[timer + 0x2C] + 1)
                    assert 0.24 <= ratio <= 0.26, ratio
                    record('PASS PWM', progress=value, duty=ratio)
            level(8)
            time.sleep(3.15)
            gpio('OOOO')
            assert not regs(TIM2, 1)[TIM2] & 1
            assert not regs(TIM22, 1)[TIM22] & 1
            record('PASS timeout and timers stopped')
            send(b'show\n', 'OK show level=8 pattern=HHHH')
            send(b'off\n', 'OK off level=8 pattern=OOOO')
            gpio('OOOO')
            send(b'show\n', 'OK show level=8 pattern=HHHH')
            for invalid in (b'9\n', b'-1\n', b'01\n', b'nope\n', b'\n', b'sh\x00w\n'):
                send(invalid, 'ERR invalid')
            send(b'x' * 100 + b'show\n', 'ERR too_long')
            send(b'show\r\n', 'OK show level=8 pattern=HHHH')
            uart.write(b'sh')
            assert uart.read(1) == b''
            send(b'ow\n', 'OK show level=8 pattern=HHHH')
            level(0)
            level(6)
            gpio('HHHO')
            level(0)
            send(b'off\n', 'OK off level=0 pattern=OOOO')
            time.sleep(0.25)
            gpio('OOOO')
            level(0)
            time.sleep(0.45)
            gpio('OOOO')
            record('PASS zero cancellation and completion; flash count requires visual check')
            send(b'show\n', 'OK show level=0 pattern=LOOO')
            if args.visual:
                print('VISUAL: explicit UART demo 0..8, each 3.3 seconds; observe two zero flashes and low/high/order.', flush=True)
                for value in range(9):
                    level(value)
                    time.sleep(3.3)
            send(b'off\n', f'OK off level={8 if args.visual else 0} pattern=OOOO')
            status = 'PARTIAL_PC_UART_RX_UNVERIFIED' if args.swd_replies else 'PASS'
            exit_code = 2 if args.swd_replies else 0
    except KeyboardInterrupt:
        record('ERROR', message='Interrupted')
        exit_code = 130
    except Exception as error:
        record('ERROR', message=repr(error))
    finally:
        if ppk is not None:
            try:
                output_off(ppk)
                off_sent = True
                record('POWER OFF sent')
            except Exception as error:
                status = 'FAIL'
                exit_code = 1
                record('POWER OFF FAILED: disconnect supply manually', message=repr(error))
            finally:
                ppk.ser.close()
        with (args.results / 'session.json').open('w', encoding='utf-8') as file:
            json.dump(dict(status=status, off_sent=off_sent, records=records,
                           physical_off_measured=False, current_measurement=False), file, indent=2)
    print(f'RESULT {status}; OFF_SENT={off_sent}', flush=True)
    return exit_code


if __name__ == '__main__':
    sys.exit(main())
