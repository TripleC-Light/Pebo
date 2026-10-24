# Battery Voltage + PPK2 Automation

Standalone Pebo v0.1 voltage accuracy test. It does not estimate battery percentage,
measure current, or modify drinking / RTC / NFC / motion firmware.

## Windows setup

Python 3.9 or later. From the repository root:

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r Software\PPK2Automation\requirements.txt
.\.venv\Scripts\python.exe -m serial.tools.list_ports -v
```

Dependencies: distribution `pyserial==3.5` imports as `serial`;
distribution `ppk2-api==0.9.2` imports
`from ppk2_api.ppk2_api import PPK2_API`.
No Nordic desktop application is needed while running this script.
Close Power Profiler and any terminal using either COM port.

## Connections and firmware

- PPK2: connect its USB data port to the PC. Use source-meter output VOUT to
  Pebo VCC_3V0 and GND to Pebo GND; remove the CR2032 and any parallel supply.
- UART: MCU PA9 (TX) to USB-UART RX, and common GND. Leave adapter TX, VCC,
  DTR and RTS disconnected. The test sends only; PA10 is MCU RX.
- Verify the adapter accepts DUT TX levels throughout 2.6..3.0 V.
  Disconnect ST-LINK and other powered signal sources during power cycling.
  Check VDD actually discharges while output is OFF before calling this a cold boot.
- Flash `FW/BatteryVoltageTest/BatteryVoltageTest.ino` explicitly using the
  existing STM32duino / ST-LINK workflow. This script never flashes firmware.
  Select Generic STM32L031G6Ux and LTO, UART generic, USB none.

Build only (Arduino CLI must be on PATH; verify its path on each computer):

```powershell
$batteryBuildPath = Join-Path $env:TEMP "Pebo-BatteryVoltageTest-debug-on"
arduino-cli compile --fqbn "STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX,xserial=generic,usb=none,opt=oslto,dbg=none,rtlib=nano,upload_method=swdMethod" --build-path $batteryBuildPath FW/BatteryVoltageTest
```

On this computer the CLI is at
`G:\我的雲端硬碟\Application\Arduino-cli\arduino-cli.exe`;
STM32duino 3.0.0 is under the user's Arduino15 directory.
To compile debug OFF, append
`--build-property "build.extra_flags=-DPEBO_BATTERY_DEBUG=0"`.
Default `PEBO_BATTERY_DEBUG=1`; default `PEBO_BATTERY_SAMPLES=32`
(overridable at compile time, 1..4096).

Keep the build output path entirely ASCII. On this Windows setup, the linker
fails when the output path contains Chinese characters; the sketch can remain
in Google Drive. If the user's TEMP path contains non-ASCII characters, choose
another writable ASCII-only folder. Use a separate -debug-off folder when
building debug OFF.

Verified builds on 2026-10-13 with STM32duino 3.0.0 and ARM GCC 14.2.1-1.1:
Debug ON uses 13856 bytes flash / 1780 bytes RAM; Debug OFF uses 13692 bytes
flash / 1780 bytes RAM. Both fit 32 KB flash / 8 KB RAM. ELF string checks
confirm BATTERY_MV in both and debug fields only in Debug ON.
STM32duino owns the ADC MSP hooks; the sketch does not override them.
The core's default ADC pin is NC and no analogRead call changes it, so
MSP initialization does not reconfigure GPIO. The five-point source-setpoint HIL sweep passed using the 15 s OFF profile
below. PA4 High was verified by a user DMM reading of 2.805 V alongside VDD=2.805 V
in a controlled ON window (UART 2788 mV). Five-point independent absolute
rail accuracy remains pending; current-capture conditions and qualification
limits are in docs/handoff.md.

## Measurement implementation

Verified against the installed STM32duino 3.0.0 ST STM32L0 driver headers:

- `stm32l0xx_hal_adc.h`: `ADC_CHANNEL_VREFINT = ADC_CHANNEL_17`.
- `stm32l0xx_ll_adc.h`: `VREFINT_CAL_ADDR` is the factory 12-bit word
  at 0x1FF80078; `VREFINT_CAL_VREF=3000` mV.
  The sketch dereferences the symbol; it contains no hard-coded address.
- ADC1 uses 12-bit right alignment, software single conversions, PCLK/4,
  160.5-cycle sampling. Low-frequency mode is selected when ADC clock <2.8 MHz.
  ADC offset self-calibration precedes configuring the internal channel.
- `HAL_ADC_ConfigChannel` enables `ADC_CCR_VREFEN`. Wait
  `LL_ADC_DELAY_VREFINT_STAB_US` (10 us in this core), discard the first
  conversion, then average 32 conversions.
- Formula: `VDDA_mV = 3000 * factory_cal / mean_VREFINT_raw`.
  The sketch uses the sum and 64-bit arithmetic, retaining the fractional
  mean and rounding only the final mV; `VREF_RAW` prints the rounded mean.
- This measures VDDA/VREF+, reported as VDD/battery only for this board's
  shared supply. Supply wiring drops and the actual VDDA/VDD relationship
  remain part of physical verification.
- Wait 250 ms at boot, print once at 115200, stop/deinitialize/reset ADC,
  disable its internal buffer/path and clock, flush/end UART, quiet unused
  GPIO, disable unused clocks and SysTick, then STOP with low-power regulator.
  PA4 / ST25_LPD stays GPIO Output High, never Analog/No-Pull.
  SWD pins PA13/PA14 are preserved. No timed wake is configured.
- ADC/calibration failure prints `ERROR=<reason>`, then performs cleanup
  and STOP; no fabricated `BATTERY_MV` is printed.

Debug ON emits BOOT, VREF_RAW, VREF_CAL, BATTERY_MV, SAMPLE_COUNT.
Debug OFF retains BOOT and `BATTERY_MV=<integer>` on success.
UART is retained for automation in both modes; neither is a battery-life
production configuration. Post-measurement current must be measured.

## Run

From the repository root:

```powershell
.\.venv\Scripts\python.exe Software\PPK2Automation\battery_voltage_sweep.py --ppk-port COM5 --uart-port COM8 --off-wait 15 --timeout 5
.\.venv\Scripts\python.exe Software\PPK2Automation\battery_voltage_sweep.py --ppk-port COM5 --uart-port COM8 --voltages 3.0 2.9 2.8 2.7 2.6 --max-error-mv 50 --off-wait 15 --boot-wait 1.5 --timeout 5 --firmware "BatteryVoltageTest commit/build-id"
```

Default points: 3000, 2900, 2800, 2700, 2600 mV. Custom points are whole
millivolts within this test's 2.600..3.000 V range; no silent PPK2 clamping.
`--baud` defaults to 115200 and must match firmware.
`--off-wait` defaults to 1 second, `--boot-wait` to 1.5 seconds,
`--timeout` to 3 seconds (additional UART read deadline after boot wait).
Waits are positive and capped at 60 seconds.

Observed on this Pebo bench (2026-10-13): the original 1 s OFF interval gave
one successful boot followed by four timeouts. With --off-wait 15 --timeout 5,
all five points booted and passed the +/-50 mV source-setpoint criterion.
The examples use this verified bench profile; the CLI default remains 1 s.
Longer waits restoring boot is observed; the VDD decay waveform and exact
cause have not been measured. Verify discharge on each new setup.

Each point: OFF -> discharge wait -> set voltage -> clear old UART data
while OFF -> ON -> boot wait -> read buffered UART -> OFF -> record result.
UART opens before power ON. No input flush occurs after ON.
The parser assembles CR/LF-delimited bytes across serial reads, accepts only
a complete `BATTERY_MV=<positive integer>` line, ignores debug/noise/oversized
lines, and bounds each point with a monotonic timeout.
A timeout writes FAIL and continues with the next point.

API sequence: `PPK2_API(port, timeout=0.1, write_timeout=1)`,
output OFF, `stop_measuring()`, drain old data with `get_data()` for 2 s,
reset the serial input buffer, then `get_modifiers()` (validate metadata),
`use_source_meter()`,
`set_source_voltage(integer_mV)`, `toggle_DUT_power("ON"/"OFF")`.
The small `CheckedPPK2` subclass reuses these methods but overrides the
verified 0.9.2 `_write_serial` hook: the upstream hook swallows write errors;
our hook propagates failures, checks short writes, and flushes commands.
No current-capture API is used.

`Error_mV = measured_mv - supply_mv`;
`Error_percent = Error_mV / supply_mv * 100`.
Inclusive acceptance: `abs(Error_mV) <= --max-error-mv` (default 50).
The comparison uses the PPK2 requested setpoint, not an independently
measured rail voltage; use a DMM/scope at VDDA to assess absolute accuracy.

## Results

Default output is relative to the script, regardless of working directory:

`Software/PPK2Automation/results/battery_voltage_YYYYMMDD_HHMMSS_microseconds.csv`

Use `--csv PATH` to select another file. Existing files are never overwritten.
Each completed point is written immediately; a failed connection creates a
header-only file, and an interrupted sweep preserves completed rows.
Fields: timestamp (local ISO 8601 with UTC offset), test_name, supply_mv,
measured_mv, error_mv, error_percent, pass_fail, firmware, ppk_port,
uart_port, reason. Missing measurements/errors are blank.
`firmware` is an operator-supplied label, not read back from the device.

Final output includes PASS/FAIL point counts, RESULT and CSV path.
Any timeout, excessive error, incomplete sweep, serial error or cleanup failure
makes RESULT FAIL. Exit codes: 0 pass, 1 failure, 2 invalid arguments,
130 Ctrl+C. An aborted sweep can show FAIL: 0 when no completed point failed;
RESULT still reports FAIL.

## Output shutdown and troubleshooting

Both each powered point and the full session use finally cleanup to command
OFF, including exception/Ctrl+C. OFF is retried three times, with SIGINT
temporarily ignored during shutdown so a second Ctrl+C cannot skip it.
The serial transport closes only after OFF has been attempted.

PPK2 commands have no output-state acknowledgement. Software cannot guarantee
physical OFF if USB disconnects, the instrument fails, or the process is killed.
An OFF write failure is reported prominently and makes the run fail:
disconnect DUT supply manually and verify VDD. A hardware interlock is needed
for a physical OFF guarantee under those failures.

- Import failure: install requirements using the exact Python running the script.
- Port busy/wrong: list ports, close Power Profiler/UART terminal, verify distinct ports.
- Metadata invalid: verify the PPK2 USB data port and connection.
  A previous current capture can leave binary packets ahead of metadata and
  trigger a UTF-8 decode error in 0.9.2. Initialization now stops capture and
  drains old bytes before requesting metadata; a regression covers this case.
- Timeout: verify flashed sketch, PA9-to-RX wiring, GND, baud and supply;
  increase boot/read waits. ADC ERROR lines do not count as measurements.
- Only the first point responds: inspect OFF-state VDD and signal back-power;
  increase off-wait after fixing wiring. Buffer flushing cannot force a reset.
- Large error: check setpoint vs actual VDDA, grounding, wiring drops,
  VREF calibration/raw output and temperature. No percentage algorithm is involved.

## Software-only verification and handoff

```powershell
.\.venv\Scripts\python.exe -m unittest discover -s Software\PPK2Automation\tests -v
.\.venv\Scripts\python.exe -m py_compile Software\PPK2Automation\battery_voltage_sweep.py
.\.venv\Scripts\python.exe Software\PPK2Automation\battery_voltage_sweep.py --help
```

Record CLI/core/Python/package versions, firmware build ID, COM ports,
board revision, wiring, requested and independently measured rail voltage,
CSV and measured sleep current in `docs/handoff.md` / `docs/power-budget.md`.
Do not call 2.6..3.0 V boot, +/-50 mV accuracy or STOP current verified until HIL runs.

Sources: [ST STM32L0 HAL/LL driver](https://github.com/STMicroelectronics/stm32l0xx-hal-driver)
and [PPK2 API source/examples](https://github.com/IRNAS/ppk2-api-python).

## Piezo current and LIS2 interaction test

Implementation:

- Firmware: `FW/PiezoLIS2InteractionTest/PiezoLIS2InteractionTest.ino`
- Automation: `Software/PPK2Automation/piezo_current_test.py`

Flash the firmware explicitly before running the script. It is a standalone
bring-up sketch and does not replace the passed baseline, LIS2 wake, RTC, ST25,
battery-voltage or LED test sketches.

Run one supply voltage per invocation:

```powershell
python Software\PPK2Automation\piezo_current_test.py ^
  --ppk-port COM10 ^
  --uart-port COM14 ^
  --voltage 2.8 ^
  --off-wait 15
```

The script uses PPK2 source mode, starts continuous current acquisition with
`start_measuring()`, reads raw stream bytes with `get_data()`, and decodes
current samples with `get_samples()`. ppk2-api 0.9.2 does not attach per-sample
timestamps and does not expose a simultaneous DUT voltage waveform through this
path. The script therefore timestamps samples from the PC monotonic clock at
batch receive time, spacing decoded samples by `--sample-rate-hz` (default
100000, matching the PPK2 stream rate used for these captures). UART markers
use the same monotonic clock. Event windows are cut from
`PIEZO_EVENT_BEGIN` to `PIEZO_EVENT_END`, with `--pre-margin-ms 100` and
`--post-margin-ms 100` defaults. If a clear current edge is found near the
expected 500 ms beep, `timing_method=edge`; otherwise the marker window is used
and `timing_method=marker`.

For each beep, the quiet baseline window is `--baseline-ms 300` before the
begin marker. The CSV records baseline average, active average, baseline-
subtracted delta average, raw peak, 99th percentile peak, duration, and charge.
Charge is integrated as:

`charge_uC = integral(max(current_uA - baseline_avg_uA, 0) * dt_seconds)`

`charge_uAh = charge_uC / 3600`. Negative noise below baseline is clamped to
zero for charge integration. PASS/FAIL uses p99 peak for current sanity instead
of single-sample raw peak. Large current is not a failure by itself because no
final product current limit has been set yet.

The firmware emits machine-readable markers only:

- `PIEZO_EVENT_BEGIN seq=1 freq_hz=2000 duration_ms=500`
- `PIEZO_EVENT_END seq=1 freq_hz=2000 lis2_during=0 lis2_after=0 wake_src=0x00 all_int_src=0x00`
- `PIEZO_TEST_SUMMARY total=9 false_triggers=0 result=PASS`

Functional PASS requires all 9 begin/end pairs, complete seq 1..9, and the
summary. LIS2 PASS requires zero false triggers: any `lis2_during=1` or
`lis2_after=1` fails LIS2_RESULT. Current sanity PASS requires active average
and p99 peak above baseline and duration within `500 +/- --duration-tolerance-ms`
(default 100 ms). Overall RESULT requires all three to pass.

Output CSV files are written to:

`Software/PPK2Automation/results/piezo_current_YYYYMMDD_HHMMSS.csv`

Fields: timestamp, test_name, seq, frequency_hz, supply_mv,
baseline_avg_uA, active_avg_uA, delta_avg_uA, peak_uA_raw, peak_uA_p99,
duration_ms, charge_uC, charge_uAh, min_voltage_mv, voltage_sag_mv,
lis2_during, lis2_after, false_trigger, timing_method, pass_fail, reason,
firmware, ppk_port, uart_port. `min_voltage_mv` and `voltage_sag_mv` are left
blank because ppk2-api 0.9.2 does not provide DUT voltage waveform samples
from this acquisition path; console prints `VOLTAGE_SAG=NOT_AVAILABLE_FROM_API`.
Use an oscilloscope at VDD to verify true instantaneous voltage sag.

The script attempts PPK2 DUT OFF in finally, including exception and Ctrl+C
paths. It cannot guarantee physical OFF if USB disconnects, the instrument
fails, or the process is killed; verify VDD or disconnect the DUT manually in
that situation.

Software-only verification:

```powershell
python -m py_compile Software\PPK2Automation\piezo_current_test.py
python -m unittest Software\PPK2Automation\tests\test_piezo_current_test.py -v
```

This repository's existing automation tests use `unittest`. If `pytest` is
installed in the active environment, the same test file can also be run through
pytest; do not add pytest solely for this test.
