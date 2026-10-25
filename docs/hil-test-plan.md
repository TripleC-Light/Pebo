# Bench automation plan

## 2026-10-25: Native HAL feasibility comparison, finite HIL PASS

Independent FW/NativeEventLoopStopTest B, no Arduino core link/RTOS; old A
unchanged/rebuilt SHAa5cd6df4...31480Flash/2860RAM. B retains diagnostics and
640appTX/64RX/64TX; GCC14.2.1/newlibnano4.4/HAL1.10.7/Os/LTO/GC. B18164Flash/
1352RAM; no slimC. Command/section/largest-symbol report under tests/results/
build-report-20261025.json. Build twice same e7e4e403...; purelogic+strongvector
checksPASS, no backup-reset/date writes. All raw PC RX evidence in native tests/results.

Existing bench_verify.py reused unchanged: flash/verify/readback/UART PASS,
baseline01/02 run10/60/off/re-run1/parser PASS before/after actual stimuli,
motion01 30s ISR/wake/confirmed26,STOP53; awakeRF01 45s ISR/confirmed1,wake0;
STOPRF01 60s ISR/wake/confirmed2,IT10,STOP62; mixed01 60s motion31/GPO1,
STOP89. Errors/unknownWake0,no quarantine; RTCdeadline/debug/UART restored.
Pre-armFIELD0 forRF, no softwareIRQ injection or debugger during timedSTOP.
Source counts merge events; motion extra unknown servicechecks not extra events.

Read-only before/after/final snapshots: LSEready/source,PRER007F00FF,PA4High,
WUT off, all5BKP words0 unchanged. Nonzero valid-schema retention, RTCaccuracy,
stackpeak/libc/IRQ depth, clock/I2C fault injection, precise race/simultaneous,
waveforms and power NOT tested. Minimumstack1024 is not peak safety proof.
User supplies2.8V, no instruments/PPK2/COM6/7 opened, optionbytes/RDP/mass erase
untouched. Currentboard nativeB e7e4e403... off/debug/UARTclosed. Historical RF
failure cause stillOPEN. See nativeREADME for limitations/commands/recommendation;
this is feasibility evidence, not formal product migration or battery acceptance.

## 2026-10-25 follow-up: historical and integrated RF regression PASS

This supersedes earlier stage4 RF non-reproduction as current functional status,
not as a deletion/reclassification of failed evidence. Unchanged ST25GpoWakeTest
source11e2f6e compiled/flashed/verified, three35s windows WAKE/IT10/EH0C/clear0
thenSTOP. Original ISR is boolean; GPO0 in same print implies flag branch true.
Unchanged integrated a5cd6df4... awake45 ISR2/confirm2/wake0; RF60 ISR/wake/
confirm2,STOP62; baseline10/60/off/re-run1/parser PASS; motion30 ISR/wake/confirm27;
mixed60 motion14/GPO2 ISR/wake/confirm,STOP75, noerrors/unknownWake/quarantine.
All windows finite; integrated RF/baseline/motion/mixed pre-arm FIELD0 checked,
original first boot EH08 (later original rounds operator-kept-away, not fresh
field samples). No debug during STOP; fullRX/readback saved
under FW/EventLoopStopTest/tests/results/rf-*. Host checks PASS. One first
HOTPLUG restore failed (tool only), under-reset flash used; retained original log.
No firmware changes or proven old root cause; only tool/capture/docs changes.
Phone detection/FIELD are not substituted for actual IRQ counts. RF removal,
VDCG waveform, exact race/simultaneous faults, power and long-run qualification
remain unverified. Historical4.43uA not current integrated power. Finaloff/debug,
UARTclosed, user supply unchanged, instruments not opened. Full differences and
source provenance: FW/EventLoopStopTest/RF_REGRESSION.md; canonical handoff updated.

## 2026-10-25: Event loop STOP stage 4 partial acceptance

Evidence: FW/EventLoopStopTest/tests/results. Final image a5cd6df4...:
bench-20261025-02 PASS baseline run10/60, deadline, UART recovery, parser,
off/restart; motion-20261025-03 PASS20s,35STOP,17 motion ISR/wake/confirmed,
34 service checks/errors0/no isolation. Host checks PASS interleaving model,
saturation, retry bounds/wrap, lifecycle, parser, midnight deadlines.
routes-20261025 PASS MCU GPIO/EXTI/NVIC only, not electrical GPO proof.
RF01/mixed01 NOT PASSED: GPO ISR0/wake0. User phone detected tag; debug FIELD1
twice (one I2C failure) does not satisfy RF interrupt acceptance. Mixed01 used
older64-service cap; final128 policy retested motion only. Retain all failures.
Exact simultaneous/race injection, sensor/I2C faults, VDCG-off signal, field
falling withLPDHigh, waveform and current unverified. No measurement equipment
opened; user supply only. Final off/debug, UART closed. RTC calendar invalid
explicitly, periodic deadline functional; no backup/date reset. No whole-stage
PASS until RF and combined stimuli succeed. Flash31480B/RAM2860B; see README.

2026-10-25 UART follow-up: user-controlled supply, no PPK/instrument access.
ProlificCOM5/115200, LF resync then7/7 real PC RX replies PASS for help/8/off/
showCRLF/invalid/overlong/off. Earlier RX blocker resolved, cause not established;
run05 partial stays historical. Evidence new BlueLedProgressTest tests/results/
uart-manual-power-20261025-02/session.json. No reflash/reset/full bench rerun.

## 2026-10-25: Standalone eight-step blue LED display

FW/BlueLedProgressTest: ARM compile/constexpr logic checks and flash verify PASS.
User authorized PPK2 supply only (no current measurement), confirmed sole source,
cell removed/common GND. Run05 uses SWD to verify GPIO/AF1..8, four25% low PWM
routes, timeout off/TIM2+TIM22 stopped, command parser/retention and42 MCU replies.
PC UART RX remains empty, so overall PARTIAL_PC_UART_RX_UNVERIFIED; never equate
debugger TX-ring evidence with PC receipt. Source and all run logs retained under
new tests/results. Human repeated playback confirms low/high,zero double flash,
physical fill order PA5->PA6->PA7->PA8. Compiler covers millis wrap and exact
sub120ms transitions/cancellation; no automated optical/waveform acceptance.
PPK OFF sent and ports closed after each run, no independent physical OFF voltage.
Next: resolve UART return path and rerun normal bench; details in canonical handoff.

This is a test plan, not a claim that instrument control scripts, drivers, or test fixtures already exist.

| Instrument | Intended role | Setup prerequisite |
| --- | --- | --- |
| ST-LINK | Flash and debug STM32 | Verify model, connections, target Vref, and CLI |
| USB-UART | Structured development logs | Verify voltage levels, port, baud, and power-off isolation |
| Nordic PPK2 | DUT supply, cycle, current capture | Verify source mode, wiring, limits, and control interface |
| Rigol DHO924 | Voltage and timing waveforms | Verify USB/LAN interface, commands, probes, and grounding |
| PN532 | Automated NFC reader/writer | Verify module interface, library, antenna placement, and RF path |

## Suggested first regression set

1. Build and flash a known firmware image; capture version and reset reason.
2. Cold boot at a nominal voltage; verify initialization and no unintended reset.
3. Check STOP/sleep current after all interfaces are idle.
4. Trigger a motion interrupt; verify wake and return to low power state.
5. Replay labeled positive/negative motion traces and compare event counts.
6. Measure I²C clock, rise/fall time, and logic levels with the assembled board.
7. Read NFC identity and a coherent status snapshot with PN532.
8. Exercise valid and malformed NFC time-sync requests; verify RTC and acknowledgement.
9. Interrupt an NFC or status update; verify data recovery/read consistency.
10. Verify piezo frequency, duration, pattern, and supply droop.
11. Check sleep and startup at several source voltages within validated operating limits.
12. Compare PN532 results with manual iPhone and Android reading/writing.

Define numeric acceptance limits from datasheets, the schematic, and product requirements before automating PASS/FAIL. A previous chat's sample thresholds and GPIO names were illustrations, not approved limits or wiring.

## Test safety and reproducibility

- Record firmware commit, schematic/PCB revision, fitted NFC variant, instrument firmware, connections, supply voltage, and raw waveform/current artifacts.
- Ensure powered instruments do not feed an unpowered DUT through signal pins; verify power-off VDD before calling a test “cold boot.”
- Document scope ground reference and any differential measurement method for piezo drive.
- Run software-only checks where possible. Mark HIL as skipped unless actual connected equipment has been exercised.

## Implementation order

Once the board and instrument interfaces are available: discover devices, make a manual known-good measurement, implement a minimal capture/read script, then add assertions and unattended regression. Keep real instrument commands in `tools/` and runnable test cases in `tests/` after their interfaces are confirmed.

## Standalone battery-voltage sweep (2026-10-13)

Implementation: `FW/BatteryVoltageTest/BatteryVoltageTest.ino` and
`Software/PPK2Automation/battery_voltage_sweep.py`.
Setup, build, API verification, CLI and troubleshooting:
`Software/PPK2Automation/README.md`.

Default source setpoints: 3000, 2900, 2800, 2700, 2600 mV; configurable
within 2600..3000 mV. Acceptance: absolute MCU-to-source-setpoint error
<=50 mV (inclusive, configurable). Timeout marks that point FAIL and
continues. A completed point is preserved in CSV; an incomplete/aborted
session returns RESULT FAIL. Error percentage uses the source setpoint
as denominator. This is an accuracy test, not a battery-capacity estimate.

Use PPK2 as the only supply, remove the cell, connect only UART RX-to-PA9
and GND, disconnect adapter TX/VCC/control pins and ST-LINK, and confirm
DUT VDD discharges while OFF. Measure actual VDDA with DMM/scope to
separate source/wiring error from ADC error. Validate PA4 Output High and
ADC/UART shutdown with current capture after the one-shot result.
Never replace one of the passed bring-up sketches to perform this test.

Software-only tests passed (13, including the current-stream cleanup regression).
Firmware Debug ON/OFF builds passed with
STM32duino 3.0.0 / LTO; see docs/handoff.md for memory usage and build
path requirements. Flash/verify/start subsequently passed using ST-LINK;
UART reported BATTERY_MV=2768 at a 2800 mV setpoint with ST-LINK attached.
The full five-point sweep subsequently passed with --off-wait 15 --timeout 5
(maximum setpoint error 26 mV); the 1 s OFF run had four UART timeouts.
A post-measurement current capture gave approximately 3.16 uA mean, with
PPK2 metadata Calibrated=0 and independent current calibration unverified.
User DMM follow-up at the 2.8 V setpoint measured VDD and PA4/ST25_LPD
both at 2.805 V during a controlled ON window (UART 2788 mV).
PA4 physical High is confirmed for that post-measurement state. Independent
five-point absolute rail accuracy, timed VDD decay, current calibration and
Debug OFF physical execution remain pending. See docs/handoff.md for details.
PPK2 OFF commands are issued from finally on exceptions/Ctrl+C and
retried, but the API has no output-state acknowledgement. Verify physical
OFF independently; a lost USB link cannot guarantee shutdown.

## Piezo + LIS2 interaction current test (2026-10-14)

Implementation added:

- `FW/PiezoLIS2InteractionTest/PiezoLIS2InteractionTest.ino`
- `Software/PPK2Automation/piezo_current_test.py`
- `Software/PPK2Automation/tests/test_piezo_current_test.py`

Purpose: verify single-ended piezo drive, 2/3/4 kHz playback, LIS2DW12 false
motion interrupt behavior during and shortly after each beep, and automated
PPK2 current/charge measurement without the Power Profiler GUI.

Firmware behavior:

- PA0 drives Piezo A through R20 as a 50 percent square wave.
- PA1 stays safe LOW; no differential drive is used.
- PA0 PWM uses TIM2 channel 1 on the STM32L031G6Ux generic variant.
- Frequencies: 2000, 3000 and 4000 Hz.
- Each frequency runs 3 repeats, 500 ms beep, 1500 ms gap.
- LIS2DW12 configuration follows the passed LIS2 wake test: 12.5 Hz low-power,
  wake-up detect, latched active-high INT1 on PB0.
- Before each beep, old LIS2 interrupt sources are cleared. During the beep
  and for a 300 ms post-beep observation window, firmware checks the PB0 ISR
  flag, INT1 level, `WAKE_UP_SRC` and `ALL_INT_SRC`.
- Any new LIS2 interrupt during either window is reported as a false trigger.
- UART markers are machine-readable `PIEZO_EVENT_BEGIN`,
  `PIEZO_EVENT_END`, and `PIEZO_TEST_SUMMARY` lines.
- At test end: piezo off, LIS2 sources clear, UART/I2C shut down, PA4/ST25_LPD
  restored Output High, unused peripherals/clocks quieted, then STOP.

Automation behavior:

- PPK2 sequence: DUT OFF, wait `--off-wait` (default 15 s), set voltage,
  clear old stream and UART data, start acquisition, DUT ON, read all 9 UART
  events, stop acquisition, DUT OFF in finally.
- PPK2 current stream uses ppk2-api 0.9.2 `start_measuring()`, `get_data()`,
  and `get_samples()`. The API does not provide per-sample timestamps or DUT
  voltage waveform samples in this path, so sample timestamps are assigned
  using PC monotonic receive time and the configured sample rate
  (`--sample-rate-hz`, default 100000). UART markers use the same clock.
- Event analysis uses 100 ms pre/post marker margins by default and attempts
  current-edge alignment around the expected 500 ms active window. If edge
  detection is not reliable, marker timing is retained and recorded.
- Baseline is the 300 ms quiet window before each begin marker.
- Charge is baseline-subtracted trapezoidal integration of
  `max(I_uA - baseline_uA, 0) * dt_seconds`, reported as uC and uAh.
- Peak is recorded both as raw single-sample peak and p99; automated current
  sanity uses p99.
- Voltage sag columns remain blank and console reports
  `VOLTAGE_SAG=NOT_AVAILABLE_FROM_API`; use an oscilloscope for instantaneous
  VDD sag.

Software-only validation passed on this session: Python syntax and 12 unittest
regressions covering parser, incomplete/missing/duplicate events, timeout,
baseline/average/p99/charge/duration checks, LIS2 false-trigger FAIL, CSV, and
Ctrl+C/exception cleanup. Firmware build passed with STM32duino 3.0.0 and LTO:
25500 / 32768 bytes flash, 2160 / 8192 bytes static RAM.

Initial HIL started on 2026-10-14. COM7 was identified as PPK2 control and
COM8 as USB-UART. The first flashed firmware produced 9 UART events but the
user heard only one beep; this exposed a PWM re-arm bug where PA0 was returned
to GPIO LOW after the first beep and later `setPWM()` calls did not restore
timer alternate function. Firmware was fixed to force TIM2_CH1 PWM mode on PA0
for every beep.

Post-fix HIL at 2.8 V produced 9/9 UART events and the user confirmed hearing
9 beeps. The run still failed acceptance: LIS2 false triggers occurred on
6 of 9 events, and current sanity failed on two 3000 Hz events because active
average was not above baseline. CSV:
`Software/PPK2Automation/results/piezo_current_20261014_164204.csv`.
No oscilloscope VDD sag measurement was taken. Do not tune LIS2 thresholds from
this result automatically; use it as evidence for the next controlled false-
trigger investigation.

## 2026-10-15: Minimal Android NFC-V discovery check

Independent probe com.pebo.nfcprobe20261015 is installed on Sony XQ-DQ72.
Keep phone unlocked, NFC enabled and probe Activity foreground. Remove other
NFC tags; bring only Pebo antenna near phone, hold about 2 seconds, move away
and retap. Preserve existing board supply/wiring; Codex does not operate power
or flash MCU for this check. Record actual power/assembly/antenna geometry
separately; none is inferred from historical PC test records.

Software PASS: assembleDebug (34 tasks), APK signature check, install and
Activity launch; initial private status READY/count=0. Physical PASS requires
a current-session onTagDiscovered callback with NfcV and user attribution to
the isolated Pebo target. Repeated callbacks are not by themselves proof of
unique tags or quantified read-range/reliability. No callback during one short
window is pending/not detected in those conditions, not a proven hardware fail.
UI UID uses Android Tag.id byte order; host status omits full UID. No NDEF or
memory reads/writes/custom RF commands. Pebo application data and MCU update
path remain untested. See canonical handoff for current physical outcome and
probe directory/logs. Manufacturer DS13519 Rev 8 pages 1/3 directly read.

2026-10-15 physical result: PASS. Current-session NFC_V_DETECTED callback, 8-byte UID and NfcV technology recorded; user confirmed success on the selected Pebo target. Power/assembly conditions remain unverified; no data-memory operation. Evidence: temporary probe physical-detection.log; canonical details: docs/handoff.md.

## 2026-10-15: ST25DV single-block engineering read checklist

Same local project/App ID extended in place (versionCode 2), not a new project.
20 unit tests PASS, debug build/signature/update install/launch PASS.
Hardware reads remain pending until operator completes:

1. Default block 0: Prepare, retap isolated Pebo, retain TX/RX and validated
   4 data bytes. Record actual board/power/variant/antenna conditions separately.
2. Remove and prepare/retap five times. Check one read per press; unarmed
   rediscovery must not append read result or issue transceive.
3. Remove phone during transfer: TagLost/I/O may depend on controller timing;
   capture actual failure category, ensure no stale current success, reprepare
   and verify recovery. Merely removing after success does not test interruption.
4. Input -1, 128 and nonnumeric values; no arm/no TX; input error visible.
5. Prepare, background, return: cancelled/no pending read; prepare again works.
6. Copy/Clear logs and previous-success/history indication after failure.

Command/parser/input and one-shot/pause gate unit tests do not prove physical
I/O cancellation, RF read correctness or error recovery. Previous NFC-V discovery
PASS remains valid; it is not substituted for these new memory-read checks.
Canonical result/update evidence: docs/handoff.md. Source/range contract in
nfc-protocol.md; no MCU/PPK2Automation/PeboBridge changes.

Final offline update (2026-10-15): serialized cancellation/normal connection close;20 tests PASS,build PASS in29s,signature PASS. Final APK SHA256 BBEB950B61B57F83B02A1509330D4AB2AAC4A2352776948A6F3015FAE575E83E. Intermediate build installed before user disconnected phone; final build NOT installed. All five memory-read physical checks pending. See canonical handoff.


2026-10-15 final-build physical Block0 PASS: final APK update install Success; user-operated read, session1792039552199; full RX00 00 00 00 00, four data bytes00 00 00 00. readBusy=false,armedBlock=-1 afterwards. Evidence in original TEMP project/single-block-physical-result.log. Five repeats/interruption/invalid UI/background and Copy-Clear UI remain unverified;see handoff.

## 2026-10-15: Protocol v0.1 Android/Sample validation

2026-10-25 Status fix update: source formatter repaired and4 regression tests added;
62 JVM tests0 failures/errors/skips, debug build PASS on PC-02. No wire/model change.
Fixed APK not installed because Sony/local debug signers differ; phone UI crash
remains in installed originalv3. Instrumentation skipped(no runner; emulator lacks
hypervisor). No new RF/HIL PASS. See handoff/evidence/pc02-20261025-status-fix and
canonical handoff. Minimal v0.2 documents add proposals only, not tested firmware.

2026-10-25 PC-02 physical update: Sony XQ-DQ72 Android15/v3 main launch,
Sample labels/DeviceInfo PASS; Read Status FAIL, reproducible formatter crash at
ProtocolActivity.kt188. Other full Sample flows remain pending. Engineering
NfcV block0 user-retap PASS, full RX00 00 00 00 00, data00 00 00 00;
does not prove real MCU protocol. Evidence under Android handoff/evidence/
pc02-20261025-ui-nfc; canonical details in docs/handoff.md. Extended five-read,
interruption/input/lifecycle/Copy-Clear checks not completed in this session.

58 JUnit PASS: 38 protocol plus 20 existing engineering regressions; no failures,
errors or skips. Covers the 12 requested groups, independent CRC fixture,
LE/timestamps, validation, records, History40/40/5, all eight Sample commands,
sequence checks, explicit unbound-real failure and 2000 malformed buffers.
Protocol has no Android/ST imports. NFC permission only; no dependencies added.
Debug build19s/40tasks and signature PASS; previous warnings non-blocking.
No phone at install attempt: version3 NOT installed; no UI/RF validation.
Previous discovery and Block0 PASS must not be counted as protocol HIL.

Next: reconnect/authorize, update same App, open Protocol/Sample button; verify
Sample banners, Info, Status3events/1completed, Config edit/readback, History
40/40/5, Sync Time and Debug/invalid HEX. Switch real mode: old values cleared,
explicit unavailable error, no packet traffic. Return engineering screen for
separate discovery/Block0 regression. True MCU protocol writes require agreed
firmware and atomic transport binding. See PROTOCOL-v0.1.md and handoff.
