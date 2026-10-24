"""Hardware-free regressions for one-shot UART capture and output cleanup."""
import contextlib
import csv
import io
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import battery_voltage_sweep as sweep


class UART:
    def __init__(self, chunks=()):
        self.chunks = iter(chunks)
        self.resets = 0

    def read(self, size):
        chunk = next(self.chunks, b"")
        if isinstance(chunk, BaseException):
            raise chunk
        return chunk

    def reset_input_buffer(self):
        self.resets += 1


class SweepTests(unittest.TestCase):
    def args(self, *extra):
        return sweep.parse_args(["--ppk-port", "COM5", "--uart-port", "COM8",
                                 "--timeout", "0.005", *extra])

    def writer(self):
        output = io.StringIO()
        writer = csv.DictWriter(output, fieldnames=sweep.FIELDS)
        writer.writeheader()
        return output, writer

    def test_fragmented_noise_and_crlf(self):
        uart = UART([b"BOOT BatteryVoltageTest\r\nVREF_RAW=1500\n",
                     b"xBATTERY_MV=2800\nBATTERY_MV=bad\nBATTERY_MV=0\n",
                     b"BATTERY_", b"MV=2817\r", b"\n"])
        self.assertEqual(sweep.read_battery_mv(uart, .1), 2817)

    def test_oversized_line_and_no_newline(self):
        uart = UART([b"x" * 300 + b"BATTERY_MV=9999\n", b"BATTERY_MV=2800\n"])
        self.assertEqual(sweep.read_battery_mv(uart, .1), 2800)
        self.assertIsNone(sweep.read_battery_mv(UART([b"BATTERY_MV=2800"]), .005))

    def test_default_sweep_timeout_continues_and_csv(self):
        args = self.args()
        output, writer = self.writer()
        ppk = MagicMock()
        values = [3000, None, 2851, 2650, 2600]
        with patch.object(sweep.time, "sleep"), patch.object(sweep, "read_battery_mv",
                side_effect=values), contextlib.redirect_stdout(io.StringIO()):
            counts = sweep.run_sweep(ppk, UART(), args, writer)
        self.assertEqual(counts, (3, 2))
        self.assertEqual([call.args[0] for call in ppk.set_source_voltage.call_args_list],
                         [3000, 2900, 2800, 2700, 2600])
        rows = list(csv.DictReader(io.StringIO(output.getvalue())))
        self.assertEqual(rows[1]["reason"], "UART_TIMEOUT")
        self.assertEqual(rows[1]["measured_mv"], "")
        self.assertEqual(rows[2]["error_mv"], "51")
        self.assertEqual(rows[3]["pass_fail"], "PASS")
        self.assertEqual(set(rows[0]), set(sweep.FIELDS))
        self.assertEqual(ppk.toggle_DUT_power.call_args.args, ("OFF",))

    def test_flush_is_before_on_and_buffered_boot_is_kept(self):
        events = []
        uart = UART([b"BATTERY_MV=3000\n"])
        uart.reset_input_buffer = lambda: events.append("flush")
        ppk = MagicMock()
        ppk.toggle_DUT_power.side_effect = lambda state: events.append(state)
        output, writer = self.writer()
        with patch.object(sweep.time, "sleep"), contextlib.redirect_stdout(io.StringIO()):
            counts = sweep.run_sweep(ppk, uart, self.args("--voltages", "3.0"), writer)
        self.assertEqual(counts, (1, 0))
        self.assertEqual(events, ["OFF", "flush", "ON", "OFF"])

    def test_exception_and_ctrl_c_after_on_turn_off(self):
        for error in (sweep.serial.SerialException("lost UART"), KeyboardInterrupt()):
            with self.subTest(error=type(error).__name__):
                ppk = MagicMock()
                output, writer = self.writer()
                with patch.object(sweep.time, "sleep"), self.assertRaises(type(error)):
                    sweep.run_sweep(ppk, UART([error]), self.args(), writer)
                self.assertEqual(ppk.toggle_DUT_power.call_args.args, ("OFF",))

    def test_output_off_retries(self):
        ppk = MagicMock()
        ppk.toggle_DUT_power.side_effect = [OSError("temporary"), None]
        with patch.object(sweep.time, "sleep"):
            sweep.output_off(ppk)
        self.assertEqual(ppk.toggle_DUT_power.call_count, 2)

    def test_output_off_failure_surfaces(self):
        ppk = MagicMock()
        ppk.toggle_DUT_power.side_effect = OSError("disconnected")
        with patch.object(sweep.time, "sleep"), self.assertRaises(OSError):
            sweep.output_off(ppk)
        self.assertEqual(ppk.toggle_DUT_power.call_count, 3)

    def test_checked_transport_detects_short_write_and_exception(self):
        ppk = sweep.CheckedPPK2.__new__(sweep.CheckedPPK2)
        ppk.ser = MagicMock()
        ppk.ser.write.return_value = 1
        with self.assertRaises(sweep.serial.SerialException):
            ppk.toggle_DUT_power("OFF")
        ppk.ser.write.return_value = 2
        ppk.toggle_DUT_power("OFF")
        ppk.ser.flush.assert_called_once()
        ppk.ser.write.side_effect = OSError("transport lost")
        with self.assertRaises(OSError):
            ppk.toggle_DUT_power("ON")

    def test_cli_bounds_and_options(self):
        args = self.args("--voltages", "2.8", "2.6", "--max-error-mv", "10")
        self.assertEqual(args.voltages, [2800, 2600])
        self.assertEqual(args.max_error_mv, 10)
        for value in ("nan", "inf", "3.1", "2.599", "2.8001", "bad"):
            with self.subTest(value=value), self.assertRaises(Exception):
                sweep.voltage_mv(value)
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.args("--uart-port", "com5")

    def test_main_partial_csv_and_ctrl_c_cleanup(self):
        ppk = MagicMock()
        ppk.get_modifiers.return_value = True
        uart = UART()
        console = io.StringIO()
        with tempfile.TemporaryDirectory() as directory:
            csv_path = Path(directory) / "result.csv"
            with patch.object(sweep, "CheckedPPK2", return_value=ppk), \
                 patch.object(sweep.serial, "Serial") as serial_factory, \
                 patch.object(sweep.time, "sleep"), \
                 patch.object(sweep, "read_battery_mv", side_effect=[3000, KeyboardInterrupt()]), \
                 contextlib.redirect_stdout(console), contextlib.redirect_stderr(io.StringIO()):
                serial_factory.return_value.__enter__.return_value = uart
                code = sweep.main(["--ppk-port", "COM5", "--uart-port", "COM8",
                                   "--csv", str(csv_path)])
            with csv_path.open(newline="") as file:
                rows = list(csv.DictReader(file))
            self.assertEqual(len(rows), 1)
            self.assertEqual(rows[0]["pass_fail"], "PASS")
        self.assertEqual(code, 130)
        self.assertIn("PASS: 1", console.getvalue())
        self.assertIn("RESULT: FAIL", console.getvalue())
        self.assertEqual(ppk.toggle_DUT_power.call_args.args, ("OFF",))
        ppk.ser.close.assert_called_once()

    def test_previous_current_stream_is_drained_before_metadata(self):
        ppk = MagicMock()
        state = {"streaming": True, "buffer": [b"\x80\xff", b"\x80"]}
        def stop():
            state["streaming"] = False
        def data():
            return state["buffer"].pop(0) if state["buffer"] else b""
        def metadata():
            if state["streaming"] or state["buffer"]:
                raise UnicodeDecodeError("utf-8", b"\x80", 0, 1, "stale current data")
            return True
        ppk.stop_measuring.side_effect = stop
        ppk.get_data.side_effect = data
        ppk.get_modifiers.side_effect = metadata
        with tempfile.TemporaryDirectory() as directory, \
             patch.object(sweep, "CheckedPPK2", return_value=ppk), \
             patch.object(sweep.serial, "Serial") as serial_factory, \
             patch.object(sweep.time, "sleep"), \
             patch.object(sweep, "read_battery_mv", return_value=2800), \
             contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            serial_factory.return_value.__enter__.return_value = UART()
            code = sweep.main(["--ppk-port", "COM5", "--uart-port", "COM8",
                               "--voltages", "2.8", "--csv", str(Path(directory) / "stream.csv")])
        self.assertEqual(code, 0)
        self.assertFalse(state["streaming"])
        self.assertEqual(state["buffer"], [])
        self.assertEqual(ppk.toggle_DUT_power.call_args.args, ("OFF",))

    def test_initialization_failure_still_turns_off(self):
        ppk = MagicMock()
        ppk.get_modifiers.return_value = False
        with tempfile.TemporaryDirectory() as directory, \
             patch.object(sweep, "CheckedPPK2", return_value=ppk), \
             contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
            code = sweep.main(["--ppk-port", "COM5", "--uart-port", "COM8",
                               "--csv", str(Path(directory) / "failed.csv")])
        self.assertEqual(code, 1)
        self.assertEqual(ppk.toggle_DUT_power.call_args.args, ("OFF",))
        ppk.ser.close.assert_called_once()

    def test_existing_csv_is_preserved(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "existing.csv"
            path.write_text("keep this result", encoding="utf-8")
            with patch.object(sweep, "CheckedPPK2") as factory, \
                 contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                code = sweep.main(["--ppk-port", "COM5", "--uart-port", "COM8",
                                   "--csv", str(path)])
            self.assertEqual(code, 1)
            self.assertEqual(path.read_text(encoding="utf-8"), "keep this result")
            factory.assert_not_called()


if __name__ == "__main__":
    unittest.main()
