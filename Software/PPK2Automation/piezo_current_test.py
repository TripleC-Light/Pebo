"""Automated piezo current and LIS2 false-trigger test using PPK2 streaming."""

import argparse
import csv
from collections import defaultdict
from dataclasses import dataclass
from datetime import datetime
from decimal import Decimal, InvalidOperation
from pathlib import Path
import re
import signal
import sys
import time

try:
    import serial
except ModuleNotFoundError:  # Parser/analysis tests do not require pyserial.
    class _MissingSerialModule:
        class SerialException(Exception):
            pass

        def Serial(self, *args, **kwargs):
            raise ModuleNotFoundError("pyserial is not installed")

    serial = _MissingSerialModule()

try:
    from ppk2_api.ppk2_api import PPK2_API
except ModuleNotFoundError:  # Tests can exercise parser/analysis without hardware deps.
    PPK2_API = None


FIELDS = (
    "timestamp", "test_name", "seq", "frequency_hz", "supply_mv",
    "baseline_avg_uA", "active_avg_uA", "delta_avg_uA", "peak_uA_raw",
    "peak_uA_p99", "duration_ms", "charge_uC", "charge_uAh",
    "min_voltage_mv", "voltage_sag_mv", "lis2_during", "lis2_after",
    "false_trigger", "timing_method", "pass_fail", "reason", "firmware",
    "ppk_port", "uart_port",
)

BEGIN_RE = re.compile(
    r"^PIEZO_EVENT_BEGIN seq=(?P<seq>\d+) freq_hz=(?P<freq>\d+) "
    r"duration_ms=(?P<duration>\d+)$"
)
END_RE = re.compile(
    r"^PIEZO_EVENT_END seq=(?P<seq>\d+) freq_hz=(?P<freq>\d+) "
    r"lis2_during=(?P<during>[01]) lis2_after=(?P<after>[01]) "
    r"wake_src=0x(?P<wake>[0-9A-Fa-f]{2}) all_int_src=0x(?P<all>[0-9A-Fa-f]{2})$"
)
SUMMARY_RE = re.compile(
    r"^PIEZO_TEST_SUMMARY total=(?P<total>\d+) false_triggers=(?P<false>\d+) "
    r"result=(?P<result>PASS|FAIL)$"
)


@dataclass
class CurrentSample:
    timestamp: float
    current_uA: float


@dataclass
class PiezoEvent:
    seq: int
    frequency_hz: int
    duration_ms_expected: int
    begin_time: float
    end_time: float = None
    lis2_during: int = None
    lis2_after: int = None
    wake_src: int = 0
    all_int_src: int = 0


@dataclass
class PiezoSummary:
    total: int
    false_triggers: int
    result: str


class CheckedPPK2(PPK2_API if PPK2_API is not None else object):
    # ppk2-api 0.9.2 logs and swallows write failures; fail before another power step.
    def _write_serial(self, cmd_tuple):
        data = self._pack_struct(cmd_tuple)
        if self.ser.write(data) != len(data):
            raise serial.SerialException("Incomplete PPK2 command write")
        self.ser.flush()


class UartMarkerParser:
    def __init__(self):
        self.buffer = bytearray()

    def feed(self, data, timestamp):
        markers = []
        for byte in data:
            if byte in (10, 13):
                if self.buffer:
                    marker = parse_marker_line(self.buffer.decode("ascii", errors="ignore"), timestamp)
                    if marker is not None:
                        markers.append(marker)
                self.buffer.clear()
            elif len(self.buffer) < 256:
                self.buffer.append(byte)
            else:
                self.buffer.clear()
        return markers


def parse_marker_line(line, timestamp):
    match = BEGIN_RE.fullmatch(line)
    if match:
        return ("begin", PiezoEvent(
            seq=int(match.group("seq")),
            frequency_hz=int(match.group("freq")),
            duration_ms_expected=int(match.group("duration")),
            begin_time=timestamp,
        ))
    match = END_RE.fullmatch(line)
    if match:
        return ("end", {
            "seq": int(match.group("seq")),
            "frequency_hz": int(match.group("freq")),
            "end_time": timestamp,
            "lis2_during": int(match.group("during")),
            "lis2_after": int(match.group("after")),
            "wake_src": int(match.group("wake"), 16),
            "all_int_src": int(match.group("all"), 16),
        })
    match = SUMMARY_RE.fullmatch(line)
    if match:
        return ("summary", PiezoSummary(
            total=int(match.group("total")),
            false_triggers=int(match.group("false")),
            result=match.group("result"),
        ))
    return None


def merge_marker(events, event, errors):
    seq = event.seq if isinstance(event, PiezoEvent) else event["seq"]
    if isinstance(event, PiezoEvent):
        if seq in events and events[seq].begin_time is not None:
            errors.append(f"DUPLICATE_SEQ_{seq}")
        events[seq] = event
        return
    if seq not in events:
        errors.append(f"END_WITHOUT_BEGIN_{seq}")
        events[seq] = PiezoEvent(seq, event["frequency_hz"], 0, None)
    current = events[seq]
    if current.end_time is not None:
        errors.append(f"DUPLICATE_END_{seq}")
    if current.frequency_hz != event["frequency_hz"]:
        errors.append(f"FREQ_MISMATCH_{seq}")
    current.end_time = event["end_time"]
    current.lis2_during = event["lis2_during"]
    current.lis2_after = event["lis2_after"]
    current.wake_src = event["wake_src"]
    current.all_int_src = event["all_int_src"]


def validate_events(events, summary, expected_count=9):
    reasons = []
    expected = set(range(1, expected_count + 1))
    actual = set(events)
    for seq in sorted(expected - actual):
        reasons.append(f"MISSING_SEQ_{seq}")
    for seq in sorted(actual - expected):
        reasons.append(f"UNEXPECTED_SEQ_{seq}")
    for seq in sorted(actual & expected):
        event = events[seq]
        if event.begin_time is None or event.end_time is None:
            reasons.append(f"INCOMPLETE_SEQ_{seq}")
    if summary is None:
        reasons.append("MISSING_SUMMARY")
    elif summary.total != expected_count:
        reasons.append("SUMMARY_TOTAL")
    return reasons


def percentile(values, pct):
    if not values:
        return None
    ordered = sorted(values)
    if len(ordered) == 1:
        return ordered[0]
    position = (len(ordered) - 1) * pct / 100.0
    lower = int(position)
    upper = min(lower + 1, len(ordered) - 1)
    fraction = position - lower
    return ordered[lower] + (ordered[upper] - ordered[lower]) * fraction


def average(values):
    return sum(values) / len(values) if values else None


def samples_between(samples, start, end):
    return [sample for sample in samples if start <= sample.timestamp <= end]


def detect_edges(samples, baseline, expected_duration_ms, edge_threshold_uA):
    if len(samples) < 4 or baseline is None:
        return None
    threshold = baseline + edge_threshold_uA
    high = [sample.current_uA > threshold for sample in samples]
    if not any(high):
        return None

    segments = []
    index = 0
    while index < len(samples):
        if not high[index]:
            index += 1
            continue
        start = index
        while index + 1 < len(samples) and high[index + 1]:
            index += 1
        end = index
        duration_ms = (samples[end].timestamp - samples[start].timestamp) * 1000.0
        segments.append((abs(duration_ms - expected_duration_ms), start, end))
        index += 1
    if not segments:
        return None
    _, start_index, end_index = min(segments)
    if end_index <= start_index:
        target_end = samples[start_index].timestamp + expected_duration_ms / 1000.0
        end_index = min(range(start_index, len(samples)),
                        key=lambda i: abs(samples[i].timestamp - target_end))
    return samples[start_index].timestamp, samples[end_index].timestamp


def integrate_charge_uC(samples, baseline):
    if len(samples) < 2 or baseline is None:
        return 0.0
    charge = 0.0
    previous = samples[0]
    previous_delta = max(previous.current_uA - baseline, 0.0)
    for sample in samples[1:]:
        delta = max(sample.current_uA - baseline, 0.0)
        dt = sample.timestamp - previous.timestamp
        if dt > 0:
            charge += ((previous_delta + delta) / 2.0) * dt
        previous = sample
        previous_delta = delta
    return charge


def quiet_baseline(samples):
    if not samples:
        return None
    currents = sorted(sample.current_uA for sample in samples)
    count = max(1, len(currents) // 5)
    return average(currents[:count])


def analyze_event(event, samples, args):
    pre = args.pre_margin_ms / 1000.0
    post = args.post_margin_ms / 1000.0
    search_margin = args.edge_search_margin_ms / 1000.0
    marker_start = event.begin_time - pre
    marker_end = event.end_time + post
    search_start = event.begin_time - search_margin
    search_end = event.end_time + search_margin
    baseline_start = event.begin_time - args.baseline_ms / 1000.0
    baseline_samples = samples_between(samples, baseline_start, event.begin_time)
    baseline = average([sample.current_uA for sample in baseline_samples])
    search_samples = samples_between(samples, search_start, search_end)
    if baseline is None:
        baseline = quiet_baseline(search_samples)

    timing_method = "marker"
    active_start = event.begin_time
    active_end = event.end_time
    edges = detect_edges(search_samples, baseline, event.duration_ms_expected,
                         args.edge_threshold_ua)
    if edges is None and baseline_samples:
        quiet = quiet_baseline(search_samples)
        retry_edges = detect_edges(search_samples, quiet, event.duration_ms_expected,
                                   args.edge_threshold_ua)
        if retry_edges is not None:
            baseline = quiet
            edges = retry_edges
    if edges is not None:
        active_start, active_end = edges
        timing_method = "edge"
    elif event.end_time is not None:
        active_start = marker_start
        active_end = marker_end
    active_samples = samples_between(samples, active_start, active_end)
    active_currents = [sample.current_uA for sample in active_samples]
    active_avg = average(active_currents)
    raw_peak = max(active_currents) if active_currents else None
    p99_peak = percentile(active_currents, 99)
    duration_ms = (active_end - active_start) * 1000.0 if active_end and active_start else None
    charge_uC = integrate_charge_uC(active_samples, baseline)

    fail_reasons = []
    warnings = []
    if baseline is None:
        fail_reasons.append("NO_BASELINE")
    if active_avg is None:
        fail_reasons.append("NO_ACTIVE_SAMPLES")
    if baseline is not None and active_avg is not None and active_avg <= baseline:
        fail_reasons.append("AVG_NOT_ABOVE_BASELINE")
    if baseline is not None and p99_peak is not None and p99_peak <= baseline:
        fail_reasons.append("P99_NOT_ABOVE_BASELINE")
    if duration_ms is None or abs(duration_ms - event.duration_ms_expected) > args.duration_tolerance_ms:
        fail_reasons.append("DURATION_OUT_OF_TOLERANCE")
    if event.lis2_during or event.lis2_after:
        fail_reasons.append("LIS2_FALSE_TRIGGER")
    if (active_avg is not None and duration_ms is not None and
            active_avg > 1000 and duration_ms > 300 and charge_uC < 300):
        warnings.append("WARN_SUSPICIOUS_CHARGE")

    return {
        "seq": event.seq,
        "frequency_hz": event.frequency_hz,
        "baseline_avg_uA": baseline,
        "active_avg_uA": active_avg,
        "delta_avg_uA": active_avg - baseline if baseline is not None and active_avg is not None else None,
        "peak_uA_raw": raw_peak,
        "peak_uA_p99": p99_peak,
        "duration_ms": duration_ms,
        "charge_uC": charge_uC,
        "charge_uAh": charge_uC / 3600.0,
        "min_voltage_mv": None,
        "voltage_sag_mv": None,
        "lis2_during": event.lis2_during,
        "lis2_after": event.lis2_after,
        "false_trigger": int(bool(event.lis2_during or event.lis2_after)),
        "timing_method": timing_method,
        "pass_fail": "PASS" if not fail_reasons else "FAIL",
        "reason": ";".join(fail_reasons + warnings),
    }


def output_off(ppk):
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


def capture_samples(ppk, start_time, sample_rate_hz):
    data = ppk.get_data()
    receive_time = time.monotonic()
    if not data:
        return []
    values, _ = ppk.get_samples(data)
    if not values:
        return []
    interval = 1.0 / sample_rate_hz
    first_time = receive_time - (len(values) - 1) * interval
    return [CurrentSample(first_time + index * interval, float(value))
            for index, value in enumerate(values)]


def collect_run(ppk, uart, args):
    parser = UartMarkerParser()
    events = {}
    marker_errors = []
    summary = None
    samples = []
    deadline = time.monotonic() + args.timeout

    while time.monotonic() < deadline:
      samples.extend(capture_samples(ppk, time.monotonic(), args.sample_rate_hz))
      data = uart.read(256)
      now = time.monotonic()
      for kind, marker in parser.feed(data, now):
          if kind == "summary":
              summary = marker
          else:
              merge_marker(events, marker, marker_errors)
      if summary is not None and len(events) >= 9 and all(events[s].end_time is not None for s in range(1, 10)):
          break
      time.sleep(args.poll_interval)

    # Drain the last serial/current bytes that may already be buffered.
    for _ in range(5):
        samples.extend(capture_samples(ppk, time.monotonic(), args.sample_rate_hz))
        for kind, marker in parser.feed(uart.read(256), time.monotonic()):
            if kind == "summary":
                summary = marker
            else:
                merge_marker(events, marker, marker_errors)
        time.sleep(args.poll_interval)
    return events, summary, samples, marker_errors


def clean_ppk_stream(ppk):
    ppk.stop_measuring()
    for _ in range(20):
        time.sleep(0.1)
        ppk.get_data()
    ppk.ser.reset_input_buffer()


def voltage_mv(text):
    try:
        value = Decimal(text) * 1000
        if not value.is_finite() or not 800 <= value <= 5000:
            raise ValueError
        if value != value.to_integral_value():
            raise ValueError
        return int(value)
    except (InvalidOperation, ValueError):
        raise argparse.ArgumentTypeError("Voltage must be 0.800..5.000 V in whole mV")


def positive_seconds(text):
    try:
        value = float(text)
        if not 0 < value <= 300:
            raise ValueError
        return value
    except ValueError:
        raise argparse.ArgumentTypeError("Time must be >0 and <=300 seconds")


def nonnegative_ms(text):
    try:
        value = int(text)
        if value < 0:
            raise ValueError
        return value
    except ValueError:
        raise argparse.ArgumentTypeError("Milliseconds must be nonnegative")


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ppk-port", required=True)
    parser.add_argument("--uart-port", required=True)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--voltage", type=voltage_mv, default=2800)
    parser.add_argument("--off-wait", type=positive_seconds, default=15.0)
    parser.add_argument("--timeout", type=positive_seconds, default=30.0)
    parser.add_argument("--pre-margin-ms", type=nonnegative_ms, default=100)
    parser.add_argument("--post-margin-ms", type=nonnegative_ms, default=100)
    parser.add_argument("--baseline-ms", type=nonnegative_ms, default=300)
    parser.add_argument("--duration-tolerance-ms", type=nonnegative_ms, default=100)
    parser.add_argument("--edge-search-margin-ms", type=nonnegative_ms, default=1000)
    parser.add_argument("--edge-threshold-ua", type=float, default=1000.0)
    parser.add_argument("--sample-rate-hz", type=float, default=100000.0)
    parser.add_argument("--poll-interval", type=float, default=0.01)
    parser.add_argument("--firmware", default="FW/PiezoLIS2InteractionTest/PiezoLIS2InteractionTest.ino")
    parser.add_argument("--csv", type=Path, help="Custom output path (must not already exist)")
    args = parser.parse_args(argv)
    if args.ppk_port.casefold() == args.uart_port.casefold():
        parser.error("PPK2 and UART must use different ports")
    if (args.baud <= 0 or args.sample_rate_hz <= 0 or args.poll_interval <= 0 or
            args.edge_threshold_ua <= 0):
        parser.error("baud, sample-rate-hz, poll-interval and edge-threshold-ua must be positive")
    return args


def format_number(value, digits=3):
    return "" if value is None else f"{value:.{digits}f}"


def write_rows(csv_path, rows, args):
    csv_path.parent.mkdir(parents=True, exist_ok=True)
    with csv_path.open("x", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=FIELDS)
        writer.writeheader()
        for row in rows:
            output = {field: "" for field in FIELDS}
            output.update({
                "timestamp": datetime.now().astimezone().isoformat(timespec="seconds"),
                "test_name": "piezo_current_test",
                "supply_mv": args.voltage,
                "firmware": args.firmware,
                "ppk_port": args.ppk_port,
                "uart_port": args.uart_port,
            })
            output.update(row)
            for key in ("baseline_avg_uA", "active_avg_uA", "delta_avg_uA",
                        "peak_uA_raw", "peak_uA_p99", "duration_ms",
                        "charge_uC", "charge_uAh"):
                output[key] = format_number(output[key], 6 if key == "charge_uAh" else 3)
            writer.writerow(output)


def print_summary(rows, marker_reasons, summary):
    print("\nSummary")
    print(f"events={len(rows)}/9")
    false_triggers = sum(row["false_trigger"] for row in rows)
    print(f"false_triggers={false_triggers}")
    by_freq = defaultdict(list)
    for row in rows:
        by_freq[row["frequency_hz"]].append(row)
    for frequency in sorted(by_freq):
        group = by_freq[frequency]
        mean_active = average([row["active_avg_uA"] for row in group if row["active_avg_uA"] is not None])
        mean_p99 = average([row["peak_uA_p99"] for row in group if row["peak_uA_p99"] is not None])
        mean_charge = average([row["charge_uC"] for row in group if row["charge_uC"] is not None])
        freq_false = sum(row["false_trigger"] for row in group)
        print(f"{frequency}Hz: mean_active={format_number(mean_active)}uA "
              f"mean_p99={format_number(mean_p99)}uA mean_charge={format_number(mean_charge)}uC "
              f"false_triggers={freq_false}")
    lis2_pass = false_triggers == 0 and summary is not None and summary.false_triggers == 0
    current_pass = all(row["pass_fail"] == "PASS" for row in rows)
    functional_pass = not marker_reasons and summary is not None and len(rows) == 9
    result = functional_pass and lis2_pass and current_pass
    print(f"UART_RESULT={'PASS' if functional_pass else 'FAIL'}")
    print(f"LIS2_RESULT={'PASS' if lis2_pass else 'FAIL'}")
    print(f"CURRENT_SANITY_RESULT={'PASS' if current_pass else 'FAIL'}")
    print("VOLTAGE_SAG=NOT_AVAILABLE_FROM_API")
    print(f"RESULT={'PASS' if result else 'FAIL'}")
    if marker_reasons:
        print(f"REASON={';'.join(marker_reasons)}")
    return result


def main(argv=None):
    args = parse_args(argv)
    csv_path = args.csv or (Path(__file__).resolve().parent / "results" /
                            datetime.now().strftime("piezo_current_%Y%m%d_%H%M%S.csv"))
    print("Piezo Current Test")
    print(f"Supply: {args.voltage} mV")
    if PPK2_API is None:
        print("ERROR: ppk2-api is not installed", file=sys.stderr)
        return 1

    ppk = None
    exit_code = 1
    try:
        ppk = CheckedPPK2(args.ppk_port, timeout=0.1, write_timeout=1)
        output_off(ppk)
        time.sleep(args.off_wait)
        clean_ppk_stream(ppk)
        if not ppk.get_modifiers():
            raise RuntimeError("PPK2 metadata invalid; check PPK2 port")
        ppk.use_source_meter()
        ppk.set_source_voltage(args.voltage)

        with serial.Serial(args.uart_port, args.baud, timeout=0.01) as uart:
            uart.dtr = False
            uart.rts = False
            uart.reset_input_buffer()
            ppk.start_measuring()
            ppk.toggle_DUT_power("ON")
            events, summary, samples, marker_errors = collect_run(ppk, uart, args)
            ppk.stop_measuring()
            output_off(ppk)

        marker_reasons = marker_errors + validate_events(events, summary)
        rows = []
        for seq in range(1, 10):
            if seq not in events or events[seq].begin_time is None or events[seq].end_time is None:
                continue
            row = analyze_event(events[seq], samples, args)
            rows.append(row)
            print(f"\nseq={row['seq']} {row['frequency_hz']}Hz")
            print(f"baseline={format_number(row['baseline_avg_uA'])}uA")
            print(f"avg={format_number(row['active_avg_uA'])}uA")
            print(f"p99={format_number(row['peak_uA_p99'])}uA")
            print(f"raw_peak={format_number(row['peak_uA_raw'])}uA")
            print(f"duration={format_number(row['duration_ms'], 0)}ms")
            print(f"charge={format_number(row['charge_uC'])}uC")
            print(f"LIS2={'FAIL' if row['false_trigger'] else 'PASS'}")
        write_rows(csv_path, rows, args)
        passed = print_summary(rows, marker_reasons, summary)
        print(f"CSV={csv_path.resolve()}")
        exit_code = 0 if passed else 1
    except KeyboardInterrupt:
        print("Interrupted; test incomplete.", file=sys.stderr)
        exit_code = 130
    except Exception as error:
        print(f"ERROR: {error}", file=sys.stderr)
        exit_code = 1
    finally:
        if ppk is not None:
            try:
                try:
                    ppk.stop_measuring()
                except Exception:
                    pass
                output_off(ppk)
            except Exception as error:
                print(f"OUTPUT OFF FAILED: {error}; disconnect DUT supply manually.", file=sys.stderr)
                exit_code = 1
            finally:
                try:
                    ppk.ser.close()
                except Exception as error:
                    print(f"PPK2 port close failed: {error}", file=sys.stderr)
                    exit_code = 1
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
