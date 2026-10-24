"""One-shot VREFINT voltage accuracy sweep using PPK2 source mode."""

import argparse
import csv
from datetime import datetime
from decimal import Decimal, InvalidOperation
from pathlib import Path
import re
import signal
import sys
import time

import serial
from ppk2_api.ppk2_api import PPK2_API


FIELDS = ("timestamp", "test_name", "supply_mv", "measured_mv", "error_mv",
          "error_percent", "pass_fail", "firmware", "ppk_port", "uart_port", "reason")
BATTERY_LINE = re.compile(rb"BATTERY_MV=([0-9]{1,5})")


class CheckedPPK2(PPK2_API):
    # 0.9.2 logs and swallows write failures; fail before another power step.
    def _write_serial(self, cmd_tuple):
        data = self._pack_struct(cmd_tuple)
        if self.ser.write(data) != len(data):
            raise serial.SerialException("Incomplete PPK2 command write")
        self.ser.flush()


def output_off(ppk):
    """Always try OFF, including interrupted/error paths; surface failed writes."""
    previous = signal.signal(signal.SIGINT, signal.SIG_IGN)
    try:
        for attempt in range(3):
            try:
                ppk.toggle_DUT_power("OFF")
                return
            except Exception:
                if attempt == 2:
                    raise
                time.sleep(0.1)
    finally:
        signal.signal(signal.SIGINT, previous)


def voltage_mv(text):
    try:
        value = Decimal(text) * 1000
        if not value.is_finite() or not 2600 <= value <= 3000:
            raise ValueError
        if value != value.to_integral_value():
            raise ValueError
        return int(value)
    except (InvalidOperation, ValueError):
        raise argparse.ArgumentTypeError("Voltage must be 2.600..3.000 V in whole mV")


def positive_seconds(text):
    try:
        value = float(text)
        if not 0 < value <= 60:
            raise ValueError
        return value
    except ValueError:
        raise argparse.ArgumentTypeError("Time must be >0 and <=60 seconds")


def read_battery_mv(uart, timeout):
    deadline = time.monotonic() + timeout
    buffer = bytearray()
    overflow = False
    while time.monotonic() < deadline:
        data = uart.read(64)
        for byte in data:
            if byte in (10, 13):
                if not overflow:
                    match = BATTERY_LINE.fullmatch(buffer)
                    if match:
                        value = int(match.group(1))
                        if value > 0:
                            return value
                buffer.clear()
                overflow = False
            elif len(buffer) < 256:
                buffer.append(byte)
            else:
                overflow = True
    return None


def run_sweep(ppk, uart, args, writer):
    passed = failed = 0
    for supply in args.voltages:
        output_off(ppk)
        time.sleep(args.off_wait)
        ppk.set_source_voltage(supply)
        # Flush while OFF, never after ON: the only boot result may arrive early.
        uart.reset_input_buffer()
        measured = None
        reason = "UART_TIMEOUT"
        try:
            ppk.toggle_DUT_power("ON")
            time.sleep(args.boot_wait)
            measured = read_battery_mv(uart, args.timeout)
        finally:
            output_off(ppk)
        error = measured - supply if measured is not None else None
        percent = error / supply * 100 if error is not None else None
        ok = error is not None and abs(error) <= args.max_error_mv
        if measured is not None:
            reason = "" if ok else "OUT_OF_TOLERANCE"
        result = "PASS" if ok else "FAIL"
        row = dict(zip(FIELDS, (
            datetime.now().astimezone().isoformat(timespec="seconds"),
            "battery_voltage_sweep", supply, measured, error,
            f"{percent:.6f}" if percent is not None else "", result,
            args.firmware, args.ppk_port, args.uart_port, reason)))
        writer.writerow(row)
        if measured is None:
            print(f"{supply} mV -> TIMEOUT FAIL")
        else:
            print(f"{supply} mV -> {measured} mV  error={error:+d} mV  ({percent:+.2f}%) {result}")
        passed += int(ok)
        failed += int(not ok)
    return passed, failed


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ppk-port", required=True)
    parser.add_argument("--uart-port", required=True)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--voltages", nargs="+", type=voltage_mv,
                        default=[3000, 2900, 2800, 2700, 2600], metavar="VOLTS")
    parser.add_argument("--off-wait", type=positive_seconds, default=1.0)
    parser.add_argument("--boot-wait", type=positive_seconds, default=1.5)
    parser.add_argument("--timeout", type=positive_seconds, default=3.0)
    parser.add_argument("--max-error-mv", type=int, default=50)
    parser.add_argument("--firmware", default="FW/BatteryVoltageTest/BatteryVoltageTest.ino",
                        help="Flashed firmware identity; supply commit/build ID for traceability")
    parser.add_argument("--csv", type=Path, help="Custom output path (must not already exist)")
    args = parser.parse_args(argv)
    if args.ppk_port.casefold() == args.uart_port.casefold():
        parser.error("PPK2 and UART must use different ports")
    if args.baud <= 0 or args.max_error_mv < 0:
        parser.error("baud must be positive and max-error-mv nonnegative")
    return args


def main(argv=None):
    args = parse_args(argv)
    csv_path = args.csv or (Path(__file__).resolve().parent / "results" /
                           datetime.now().strftime("battery_voltage_%Y%m%d_%H%M%S_%f.csv"))
    ppk = None
    passed = failed = 0
    fatal = False
    csv_created = False
    exit_code = 1
    try:
        # Open CSV before enabling power; line buffering preserves each completed row.
        csv_path.parent.mkdir(parents=True, exist_ok=True)
        with csv_path.open("x", newline="", encoding="utf-8", buffering=1) as file:
            csv_created = True
            writer = csv.DictWriter(file, fieldnames=FIELDS)
            writer.writeheader()
            ppk = CheckedPPK2(args.ppk_port, timeout=0.1, write_timeout=1)
            output_off(ppk)
            # A previous current capture can leave binary packets ahead of metadata.
            ppk.stop_measuring()
            for _ in range(20):
                time.sleep(0.1)
                ppk.get_data()
            ppk.ser.reset_input_buffer()
            if not ppk.get_modifiers():
                raise RuntimeError("PPK2 metadata invalid; check PPK2 port")
            ppk.use_source_meter()
            with serial.Serial(args.uart_port, args.baud, timeout=0.1) as uart:
                uart.dtr = False
                uart.rts = False
                passed, failed = run_sweep(ppk, uart, args, writer)
            exit_code = int(failed != 0)
    except KeyboardInterrupt:
        fatal = True
        exit_code = 130
        print("Interrupted; sweep incomplete.", file=sys.stderr)
    except Exception as error:
        fatal = True
        print(f"ERROR: {error}", file=sys.stderr)
    finally:
        if ppk is not None:
            try:
                output_off(ppk)
            except Exception as error:
                fatal = True
                exit_code = 1
                print(f"OUTPUT OFF FAILED: {error}; disconnect DUT supply manually.", file=sys.stderr)
            finally:
                try:
                    ppk.ser.close()
                except Exception as error:
                    fatal = True
                    exit_code = 1
                    print(f"PPK2 port close failed: {error}", file=sys.stderr)
    # Counts come from persisted rows even when the sweep was interrupted.
    if fatal and csv_created:
        with csv_path.open(newline="", encoding="utf-8") as file:
            rows = list(csv.DictReader(file))
        passed = sum(row["pass_fail"] == "PASS" for row in rows)
        failed = sum(row["pass_fail"] == "FAIL" for row in rows)
    print("\nBattery Voltage Sweep Summary")
    print(f"PASS: {passed}\nFAIL: {failed}")
    print(f"RESULT: {'PASS' if exit_code == 0 and not fatal else 'FAIL'}")
    print(f"CSV: {csv_path.resolve()}")
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
