# NativeDrinkDetectionTest - stage6 engineering prototype

Current supplemental acceptance (separate from original empty-bottle release):
[SUPPLEMENTAL_VALIDATION.md](SUPPLEMENTAL_VALIDATION.md). Firmware/parameters
remain frozen; host capture now requires CAPTURE_BEGIN before the visual cue.
This session completed ten independent empty single gestures and ten pickup
negatives, plus cancellation checks. Real-water testing deferred by the user;
no real-drinking acceptance or production accuracy claim. See report for
deadline operator-confirmation and trace-completeness limitations.

Independent native STM32L031G6U6TR, 32KB Flash / 8KB SRAM. No Arduino core,
RTOS, heap allocation, application NFC protocol, storage, task scoring, sound
or automatic progress increment. SHAKE displays the manually configured level;
DRINK_CONFIRMED means motion compatible with drinking, NOT verified intake.
It cannot measure volume, prove water reached the mouth, reliably distinguish
pouring, or guarantee detection of upright straw use.

## Recovery and reproducible build

Stage5 commit128d941f0c81c40e1cb0ad388f8817f40f65c13d, image
2ae93c94b3038495a94f9af0101a858ac326cdcc8427ec7c3f88d162f5828ed2.
Annotated checkpoint/native-shake-progress-v1 points to that commit, not stage6.
Verified baseline/native-hal-eventloop-v1 resolves820ecb3f2b9fb199ddf5cf95246698d4648b73b1.
Both tags explicitly pushed to origin2026-10-25, no force/update. At validation
time stage6 was uncommitted; user subsequently authorized publication2026-10-26.
Stage6 commit 3d6d70a9a60e5c0ea0ed8dd95b6a00ca1f72c1cf is now published on origin/main.

```powershell
$env:ARM_GCC_BIN = '<installed arm-none-eabi GCC14.2.1-1.1 bin directory>'
python FW/NativeDrinkDetectionTest/build.py --toolchain-bin $env:ARM_GCC_BIN --build-dir "$env:TEMP/Pebo-NativeDrinkDetectionTest"
```

Python stdlib; GCC14.2.1/newlib-nano4.4; Os/LTO/sectionGC/soft-float/integer
snprintf/nosys; _sbrk rejects allocation. Build checks111 pinned vendor files,
actual C++ pure policies and strong IRQ vectors; emits ELF/bin/hex/map/JSON.
HAL1.10.7 BSD-3-Clause, CMSIS Device1.9.4/Core6.1(package6.3) Apache-2.0 from
sibling NativeEventLoopStopTest/vendor, plus unchanged local HAL TIM sources
with license/manifest. Keep the whole repository; no unrecorded Arduino source
path dependency. Existing projects remain unchanged except stage5 status docs.

## Confirmed hardware / ownership

Same stage5 Board/startup/linker: HSI16 PLLx4/2=32MHz; LSE32768 RTC prediv127/255,
WUT1Hz priority0. No backup-domain reset, RTC date initialization, option-byte
change or new backup marker. BKP2/BKP3 retain stage3 meaning, others untouched.
USART2 PA9TX/PA10RX AF4,1152008N1; current verified PC-02 Prolific COM5.
I2C1 PB6/7 AF1,100kHz/external10k. LIS2@0x19 WHO44; PB0 INT1 rising/noPull,
latched High priority3. ST25DV04KC GPO100ohm to PA3, rising/noPull priority3;
GPO1=11/dynamic gate1, scoped IT read-clear in foreground. PA4LPD Output High
in STOP, Low only scoped ST I2C then restored. VDCG electrical limits remain
as documented in pinmap; RF field state is not NFC application transfer.
Blue PA5/6/7/8 High-on; low1000Hz25%, highGPIOHigh. TIM2_CH1 shared PA5/8,
TIM22_CH1 PA6 /CH2 PA7, at most one low. Same3s/zero120ms double flash.
Exclusive timer ownership in this test; future piezo TIM2_CH1 needs arbitration.
Off stops owned PWM/clocks and restores GPIO low. No red/buzzer integration.

## Calibration and one sampling owner

Install as product, place bottle upright and still, issue calibration start.
2200ms acquisition, discard first2 fresh conversions, require150 samples,
norm820..1230 counts and each axis range<=60 counts. Accept mean vector only
on success; failed calibration preserves the previous reference. RAM only,
reset invalidates it. Upright reference measures gravity, not spout heading.
Physical axes not inferred from schematic. This session empty direct-drink
bottle, stage5 mounting retained; measured reference15,-1017,-32 counts.

One Observation owner disables LIS2 wake routing, selects CTRL1=50 (100Hz LP1,
12bit +/-2g), retains BDU/autoincrement CTRL2=0C/CTRL6=00. STATUS27 DRDY and
one6-byte28..2D read, int16LE/16,0.976mg/count. Poll2ms, no fresh100ms,
3 I2C failures abort; independent awake HAL-tick bound limit+250ms. Restore
verified12.5Hz LP1 idle config up to3 tries, quarantine failure. Never stale
polls as new samples. ISR only records flags/counters; UART/RTC/GPO continue
in foreground during sample/display. No STOP while sample/display/TX active.
Official [DS11811 Rev9](https://www.st.com/resource/en/datasheet/lis2dw12.pdf)
and [AN5038 Rev6](https://www.st.com/resource/en/application_note/dm00401877-lis2dw12-alwayson-3d-accelerometer-stmicroelectronics.pdf)
directly checked; two discarded samples are conservative project policy.

## Recognition (PROVISIONAL engineering parameters)

Motion IRQ is an observation entrance, not a drink. Automatic observation15s;
single candidate max12s. Filter gravity with integer1/8 IIR, accept norm820..1230
and consecutive step<=180 counts, exclude unreliable dynamics. Missing>100ms
cannot bridge a hold; unreliable>=150ms breaks tilt continuity/return stability.
Tilt enter cos-squared<=2500/10000 (~60deg), release>=~45deg, return
cos-squared>=8200/10000 (~25deg). Use signed dot products and64bit squared
comparisons, no runtime trigonometry/floats. Require1200ms tilted and500ms
upright stable (raw/filter residual<=120); incomplete/timeout/error rejected.
These values are not product requirements or population-validated thresholds.

Shared pure Drink::coordinate runs shake and drink logic on identical samples.
Confirmed shake blocks the current drink segment until500ms upright stability;
then a later drink can start, including while LED remains active. Shake retains
three reversals/600ms/20ms direction dwell; stage6 additionally requires20 fresh
samples outside norm750..1300 within its1500ms window. Training data showed
pickup in a single empty-bottle sip wrongly met old shake logic, whereas
intentional shake had sustained dynamic norm departures. This extra condition
is provisional; weak shakes may be missed.2s cooldown from confirmation and
quiet rearm prevent continuous shaking from extending3s display.

Ordinary tilted returns can resemble drinking. Slow tilt/pouring discrimination
remains a limitation, not a promise. Empty-bottle gestures are NOT real intake.

## Deferred merge / STOP

Each completed candidate increments candidates only, no LED/progress/audio.
Pending group keeps first tilt time, last completed candidate time and count.
Before processing a new candidate, elapsed>=10000ms flushes old group once;
exactly10s starts a new group, <10s extends it from the latest completion.
DRINK_CONFIRMED includes first/last/count/output elapsed-ms timestamps and
STOP count, so motion occurrence and delayed output are distinct. These are
wrapping boot-relative monotonic times, not UTC/history storage. clock reports
separate UTC validity. Calibration and classifier state are RAM only.

Automatic sampling can end after completed candidate, restoring idle so merge
waiting may STOP. RTC WUT1Hz reevaluates due time (up to about1s quantization),
not HAL tick. The fixed maximum run remains60s. Run deadline takes precedence
over deferred confirmation: cancels pending group. Off/debug/explicit setutc
also cancel it; reset loses pending RAM group without inventing an event.
Same coherent RTC/elapsed midnight/backward/wrap/rebase fix as stage5; no
clock adjustment is counted as elapsed time. PRIMASK snapshot/pending-checked
WFI, retries/fault budgets and STOP clock recovery retained. UART reopens when
deferred output is queued after wake; normal RX is not a STOP wake source.

## UART and finite evidence

Existing help/status/counters/counters clear/mode debug/run1..60/off/progress0..8/
show/clock/setutc/ledregs/lisregs/probe/observe/trace retained. Status adds DRINK.
calibration start / calibration status; drink status / drink counters;
capture5..30 (seconds, debug only, requires calibrated enabled sensor).
Fixed32-byte parser rejects NUL/overlong/invalid input without state mutation.
Observe is a finite diagnostic, NOT physical wake proof. Trace stores at most320
rows at >=50ms spacing (~16s); later rows truncate, never overwrite memory.
Explicit capture streams all fresh raw timestamp/XYZ at100Hz for bounded time;
host stores them, dropped counter must be checked. It is not normal background
logging and no second big sensor buffer exists. Trace is not full-rate replay.

```powershell
python FW/NativeDrinkDetectionTest/tests/bench.py --test flash --image "$env:TEMP/Pebo-NativeDrinkDetectionTest/NativeDrinkDetectionTest.bin" --results FW/NativeDrinkDetectionTest/tests/results/NEW-FLASH --split engineering
python FW/NativeDrinkDetectionTest/tests/bench.py --test calibration --image "$env:TEMP/Pebo-NativeDrinkDetectionTest/NativeDrinkDetectionTest.bin" --results FW/NativeDrinkDetectionTest/tests/results/NEW-CAL
python FW/NativeDrinkDetectionTest/tests/bench.py --test capture --seconds 30 --label empty_three_sips --split training --arm --image "$env:TEMP/Pebo-NativeDrinkDetectionTest/NativeDrinkDetectionTest.bin" --results FW/NativeDrinkDetectionTest/tests/results/NEW-CAPTURE
```

Use `--seconds 30` (space between option/value). --arm waits at most45s for a
controller newline AFTER setup, allowing a clear operator start prompt. Helper
validates STLINK56FF6C066688555210351387/MCU425/selected Prolific UART, image readback,
and finally off/debug/closes UART. No power instruments. Pyserial already local.
On PC-02 after2026-10-26 reconfiguration pass --uart-port COM13 explicitly;
COM5 is historical/default, not assumed current. Nordic COM6/7 never opened.
Debug awake windows permit up to150s for operator pacing, without changing
firmware run1..60 or sampling bounds. Start the controller window FIRST, wait
for WINDOW_READY, THEN prompt the operator; a pre-window gesture is not scored.
Acquisition PASS is not recognition acceptance. Keep training vs validation
labels distinct; user must confirm actual completed motions. Use tests/replay.py
on raw.json with actual reference/expected counts and GCC compiler; it evaluates
the exact shared C++ modules. features.py is descriptive analysis, not classifier.

Three-sip operator recipe: pick up once, naturally tilt then HOLD tilted2s;
naturally return then HOLD upright1s; repeat, put down after third.2s/1s are
posture holds, not rotation durations. No forced larger angle to pass.
--cue sends existing manual progress0 double flash at debug capture start;
watch that cue rather than delayed chat text. It is not classifier output.

## Resource / acceptance

32KB/8KB linker, stack minimum1024/heap0, peak stack NOT_MEASURED. No live stack
painting. Main trace2560 bytes/appTX1024 dominate RAM. See build-report.json and
VALIDATION.md for actual versioned numbers, results and limitations. Power,
long-term RTC accuracy, fault/race stress remain unmeasured. No six-month claim.
