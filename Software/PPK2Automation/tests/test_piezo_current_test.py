"""Hardware-free regressions for piezo current automation."""
import contextlib
import csv
import io
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest.mock import MagicMock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import piezo_current_test as piezo


class UART:
    def __init__(self, chunks=()):
        self.chunks = iter(chunks)
        self.dtr = True
        self.rts = True

    def read(self, size):
        chunk = next(self.chunks, b"")
        if isinstance(chunk, BaseException):
            raise chunk
        return chunk

    def reset_input_buffer(self):
        pass


class PiezoTests(unittest.TestCase):
    def args(self, *extra):
        return piezo.parse_args(["--ppk-port", "COM10", "--uart-port", "COM14",
                                 "--timeout", "1", *extra])

    def analysis_args(self, tolerance=100):
        return types.SimpleNamespace(pre_margin_ms=100, post_margin_ms=100,
                                     baseline_ms=300, duration_tolerance_ms=tolerance,
                                     edge_search_margin_ms=1000,
                                     edge_threshold_ua=20.0)

    def complete_event(self, seq=1, freq=2000, begin=1.0, end=1.5, during=0, after=0):
        return piezo.PiezoEvent(seq, freq, 500, begin, end, during, after, 0, 0)

    def samples_for_event(self):
        samples = []
        t = 0.65
        while t < 1.7:
            current = 10.0
            if 1.0 <= t <= 1.5:
                current = 110.0 + (10.0 if int((t - 1.0) * 1000) == 250 else 0.0)
            samples.append(piezo.CurrentSample(t, current))
            t += 0.01
        return samples

    def samples_with_shifted_marker(self):
        samples = []
        t = 0.0
        while t <= 2.0:
            current = 12.0
            if 0.70 <= t <= 1.20:
                current = 6012.0
            samples.append(piezo.CurrentSample(t, current))
            t += 0.01
        return samples

    def test_uart_marker_parser_fragmented(self):
        parser = piezo.UartMarkerParser()
        first = parser.feed(b"noise\r\nPIEZO_EVENT_BEGIN seq=1 freq_h", 1.0)
        second = parser.feed(b"z=2000 duration_ms=500\r\nPIEZO_EVENT_END seq=1 freq_hz=2000 lis2_during=0 lis2_after=1 wake_src=0x08 all_int_src=0x20\n", 1.5)
        third = parser.feed(b"PIEZO_TEST_SUMMARY total=9 false_triggers=1 result=FAIL\n", 2.0)
        self.assertEqual(first, [])
        self.assertEqual(second[0][0], "begin")
        self.assertEqual(second[0][1].seq, 1)
        self.assertEqual(second[1][0], "end")
        self.assertEqual(second[1][1]["lis2_after"], 1)
        self.assertEqual(third[0][0], "summary")
        self.assertEqual(third[0][1].false_triggers, 1)

    def test_missing_duplicate_and_incomplete_events(self):
        events = {}
        errors = []
        begin = piezo.parse_marker_line("PIEZO_EVENT_BEGIN seq=1 freq_hz=2000 duration_ms=500", 1.0)[1]
        piezo.merge_marker(events, begin, errors)
        piezo.merge_marker(events, begin, errors)
        end = piezo.parse_marker_line("PIEZO_EVENT_END seq=2 freq_hz=2000 lis2_during=0 lis2_after=0 wake_src=0x00 all_int_src=0x00", 1.5)[1]
        piezo.merge_marker(events, end, errors)
        reasons = errors + piezo.validate_events(events, None)
        self.assertIn("DUPLICATE_SEQ_1", reasons)
        self.assertIn("END_WITHOUT_BEGIN_2", reasons)
        self.assertIn("INCOMPLETE_SEQ_1", reasons)
        self.assertIn("MISSING_SUMMARY", reasons)
        self.assertIn("MISSING_SEQ_9", reasons)

    def test_timeout_validation(self):
        events = {i: self.complete_event(i, 2000 + (i // 4) * 1000, i, i + 0.5)
                  for i in range(1, 9)}
        summary = piezo.PiezoSummary(8, 0, "PASS")
        reasons = piezo.validate_events(events, summary)
        self.assertIn("MISSING_SEQ_9", reasons)
        self.assertIn("SUMMARY_TOTAL", reasons)

    def test_baseline_average_percentile_charge_and_duration(self):
        row = piezo.analyze_event(self.complete_event(), self.samples_for_event(), self.analysis_args())
        self.assertEqual(row["pass_fail"], "PASS")
        self.assertAlmostEqual(row["baseline_avg_uA"], 10.0)
        self.assertGreater(row["active_avg_uA"], 100.0)
        self.assertGreater(row["peak_uA_raw"], row["peak_uA_p99"])
        self.assertAlmostEqual(row["duration_ms"], 500.0, delta=25.0)
        self.assertGreater(row["charge_uC"], 45.0)
        self.assertAlmostEqual(row["charge_uAh"], row["charge_uC"] / 3600.0)

    def test_edge_detection_success_with_shifted_uart_marker(self):
        event = self.complete_event(begin=1.0, end=1.5)
        args = self.analysis_args()
        args.edge_threshold_ua = 1000.0
        row = piezo.analyze_event(event, self.samples_with_shifted_marker(), args)
        self.assertEqual(row["timing_method"], "edge")
        self.assertAlmostEqual(row["duration_ms"], 500.0, delta=15.0)
        self.assertAlmostEqual(row["active_avg_uA"], 6012.0, delta=1.0)
        self.assertAlmostEqual(row["charge_uC"], 3000.0, delta=75.0)

    def test_edge_detection_fallback_to_marker(self):
        event = self.complete_event(begin=1.0, end=1.5)
        args = self.analysis_args()
        args.edge_threshold_ua = 1000.0
        row = piezo.analyze_event(event, self.samples_for_event(), args)
        self.assertEqual(row["timing_method"], "marker")

    def test_suspicious_charge_warning(self):
        event = self.complete_event(begin=1.0, end=1.5)
        args = self.analysis_args()
        samples = [piezo.CurrentSample(0.9, 10.0),
                   piezo.CurrentSample(1.0, 5010.0),
                   piezo.CurrentSample(1.5, 5010.0)]
        with patch.object(piezo, "integrate_charge_uC", return_value=100.0):
            row = piezo.analyze_event(event, samples, args)
        self.assertIn("WARN_SUSPICIOUS_CHARGE", row["reason"])

    def test_negative_noise_is_clamped_for_charge(self):
        samples = [piezo.CurrentSample(0.0, 10.0), piezo.CurrentSample(0.1, 9.0),
                   piezo.CurrentSample(0.2, 20.0)]
        self.assertAlmostEqual(piezo.integrate_charge_uC(samples, 10.0), 0.5)

    def test_duration_tolerance_fail(self):
        with patch.object(piezo, "detect_edges", return_value=None):
            row = piezo.analyze_event(self.complete_event(end=1.8), self.samples_for_event(),
                                      self.analysis_args(tolerance=50))
        self.assertEqual(row["pass_fail"], "FAIL")
        self.assertIn("DURATION_OUT_OF_TOLERANCE", row["reason"])

    def test_lis2_false_trigger_fail(self):
        row = piezo.analyze_event(self.complete_event(during=1), self.samples_for_event(),
                                  self.analysis_args())
        self.assertEqual(row["pass_fail"], "FAIL")
        self.assertIn("LIS2_FALSE_TRIGGER", row["reason"])

    def test_csv_generation(self):
        row = piezo.analyze_event(self.complete_event(), self.samples_for_event(), self.analysis_args())
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "piezo.csv"
            piezo.write_rows(path, [row], self.args("--voltage", "2.8"))
            with path.open(newline="", encoding="utf-8") as file:
                rows = list(csv.DictReader(file))
        self.assertEqual(set(rows[0]), set(piezo.FIELDS))
        self.assertEqual(rows[0]["test_name"], "piezo_current_test")
        self.assertEqual(rows[0]["supply_mv"], "2800")
        self.assertEqual(rows[0]["seq"], "1")

    def test_capture_samples_assigns_monotonic_timestamps(self):
        ppk = MagicMock()
        ppk.get_data.return_value = b"raw"
        ppk.get_samples.return_value = ([1.0, 2.0, 3.0], [])
        with patch.object(piezo.time, "monotonic", return_value=10.0):
            samples = piezo.capture_samples(ppk, 0.0, 1000.0)
        self.assertEqual([sample.current_uA for sample in samples], [1.0, 2.0, 3.0])
        self.assertAlmostEqual(samples[0].timestamp, 9.998)
        self.assertAlmostEqual(samples[-1].timestamp, 10.0)

    def test_output_off_retries_and_failure_surfaces(self):
        ppk = MagicMock()
        ppk.toggle_DUT_power.side_effect = [OSError("temporary"), None]
        with patch.object(piezo.time, "sleep"):
            piezo.output_off(ppk)
        self.assertEqual(ppk.toggle_DUT_power.call_count, 2)

        ppk = MagicMock()
        ppk.toggle_DUT_power.side_effect = OSError("lost")
        with patch.object(piezo.time, "sleep"), self.assertRaises(OSError):
            piezo.output_off(ppk)
        self.assertEqual(ppk.toggle_DUT_power.call_count, 3)

    def test_main_ctrl_c_cleanup_path(self):
        ppk = MagicMock()
        ppk.get_modifiers.return_value = True
        with patch.object(piezo, "PPK2_API", object), \
             patch.object(piezo, "CheckedPPK2", return_value=ppk), \
             patch.object(piezo.serial, "Serial") as serial_factory, \
             patch.object(piezo.time, "sleep"), \
             patch.object(piezo, "clean_ppk_stream"), \
             patch.object(piezo, "collect_run", side_effect=KeyboardInterrupt()), \
             contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            serial_factory.return_value.__enter__.return_value = UART()
            code = piezo.main(["--ppk-port", "COM10", "--uart-port", "COM14"])
        self.assertEqual(code, 130)
        self.assertEqual(ppk.toggle_DUT_power.call_args.args, ("OFF",))
        ppk.ser.close.assert_called_once()

    def test_main_exception_cleanup_path(self):
        ppk = MagicMock()
        ppk.get_modifiers.return_value = True
        with patch.object(piezo, "PPK2_API", object), \
             patch.object(piezo, "CheckedPPK2", return_value=ppk), \
             patch.object(piezo.serial, "Serial") as serial_factory, \
             patch.object(piezo.time, "sleep"), \
             patch.object(piezo, "clean_ppk_stream"), \
             patch.object(piezo, "collect_run", side_effect=RuntimeError("boom")), \
             contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            serial_factory.return_value.__enter__.return_value = UART()
            code = piezo.main(["--ppk-port", "COM10", "--uart-port", "COM14"])
        self.assertEqual(code, 1)
        self.assertEqual(ppk.toggle_DUT_power.call_args.args, ("OFF",))
        ppk.ser.close.assert_called_once()


if __name__ == "__main__":
    unittest.main()
