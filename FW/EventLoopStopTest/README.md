# EventLoopStopTest

## Latest RF regression: 2026-10-25

RF wake now reproduced on **unchanged** integrated binary a5cd6df4... after
original ST25GpoWakeTest passed three windows. Awake GPO ISR2/confirmed2;
STOP RF60s ISR/wake/confirmed2; baseline10/60/off/re-run1 PASS; motion30s PASS;
mixed60s motion14 andGPO2 ISR/wake/confirmed, STOP75, errors0/no quarantine.
Earlier failure records below remain historical, not current non-support.
No firmware root cause reproduced or speculative patch applied. Only test
orchestration/docs changed; Flash31480B/staticRAM2860B unchanged.
Full source provenance, difference table, raw-evidence links and open causes:
[RF_REGRESSION.md](RF_REGRESSION.md). No current measurement or removal-wake claim.

For repeatable manual RF tests, use bench_verify.py `--stimulus rf
--window-seconds 60 --require-field-absent --existing` with the explicit port,
STLINK, image and result path shown by `--help`; awake test uses
`--stimulus awake-rf --window-seconds 45`. Keep phone away before arming, wait
for RUN_READY/operator prompt, approach5s then remove. Mixed uses
`--stimulus mixed --window-seconds 60 --require-field-absent`. No debugger
accesses during run. Field/status last-sample values are not fresh IRQ evidence.

Standalone stage4 event -> foreground service -> safe STOP skeleton. Default
debug/interactive UART, never sleeps automatically at boot. No drinking
recognition, schedules, blue/red effects, NFC application data or simulated time.
Existing bring-up sketches are unchanged. All parameters are engineering values.

## Confirmed hardware and resources

- STM32L031G6U6TR, 32KB Flash /8KB SRAM; same STM32duino3.0.0 Generic
  L031G6Ux/LTO FQBN as earlier stages, SWD4MHz, PA9TX/PA10RX1152008N1.
- I2C1 PB6/PB7, external10kohm pull-ups,100kHz. LIS2DW12 at0x19,
  R16-powered LIS2_VDD; WHO_AM_I0x44 required. PB0/EXTI0 rising/activeHigh,
  push-pull latched INT1, no pull. CTRL2=0C, CTRL3=10, CTRL4=20, CTRL5=0,
  CTRL6=0, WAKE_UP_THS=2,DUR=0,CTRL7=20,CTRL1=20 (12.5Hz LP12bit).
  Based on passed LIS2WakeupStopTest; no unnecessary sample/axis reads or reset.
  WAKE_UP_SRC0x38/ALL_INT_SRC0x3B read in foreground to clear latch. Not a drink.
- User physically confirmed ST25DV04KC-JF6D3, GPO->100ohm->PA3, no external
  pull-up/down. KC-specific profile only, not a K-compatible universal driver.
  PA3/EXTI3 rising, Input/GPIO_NOPULL follows CMOS positive GPO with VDCG
  powered from ST25_VDD (R17). EXTI0 andEXTI3/IRQ0_1 andIRQ2_3 do not conflict.
  See updated `docs/hardware_pinmap.md` for the supply-state table.
- Directly checked ST DS13519 Rev8 sections2.4.1/2.4.2, table29 and KC
  dynamic-register tables: https://www.st.com/resource/en/datasheet/st25dv04kc.pdf .
  CMOS driver requires VDCG; NOPULL does not define an unpowered/floating VDCG
  condition. Current board has no software supply switch. No new hardware needed
  for the mapped common-powered condition; do not claim operation with R17 removed.
- PA4/LPD stays Output High in STOP. Each foreground ST access temporarily
  lowers it, waits the existing bounded1ms startup interval, then RAII restores
  High on every return. No supply, RF management or EEPROM programming command.
  Static GPO1 FIELD_CHANGE bit4 must already be enabled; otherwise disable/fault
  this source, never open a security session or rewrite EEPROM to make it pass.
  Dynamic GPO_CTRL0x2000 b0 gates output; IT_STS0x2005 read-clears b3/b4 causes;
  EH_CTRL0x2002 b2 is FIELD_ON (not RF_MNGT). User/system addresses0x53/0x57.
- Important limit: with LPD High, field-falling FIELD_CHANGE cannot be assumed
  to produce GPO. Field appearance is the reliable intended wake case. A later
  GPO-triggered sample may report field absent; RTC wake is NOT used to poll RF
  or invent a field-removal interrupt. Persistent field is not a new event.
  IT_STS may have no retained field cause after the LPD/boot transition; count
  unknown cause separately, never infer cause merely from FIELD_ON.
- RTC/LSE PC14/PC15 uses prior127/255 prescalers, WUT ck_spre1Hz reload0,
  existing SystemClock_Config HSI16->PLL32MHz restore. No backup reset/date write.
  This test allocates **no backup register**, reads stage3 BKP2/BKP3 only for
  UTC-validity display (and shares pure date utilities, not sim/time UI).
  Deadline uses validated seconds-of-day modulo86400, even if UTC is uncalibrated;
  uncalibrated UTC remains invalid. Reject foreign clock/config/alarm ownership.
- PA4/PB0/PA3, SWD and PC14/15 are excluded from unused Analog/NoPull shutdown;
  unused LED/piezo pins analog, I2C inputs after Wire.end, UART analog only while
  in run. No TIM2/TIM22 owner instantiated, no effect/power instrumentation.

## API, synchronization and counts

`initializeEventSources()`, `processEvents()`, `standbyIfIdle()` and
`stopEventSources()` separate source setup, service and standby.
ISRs only use the HAL/EXTI hardware handler and record a bit plus saturating
uint32 observed count. No I2C/UART/application work. PRIMASK save/disable/DMB/
restore protects shared RMW **in ISR too**, because RTC may preempt EXTI.
Foreground takes all pending bits and clears only that mailbox under the same
critical region. Events arriving during processing remain in the new mailbox.
No single-source else-if dispatch, no large queue or dynamic event allocation.

Counters intentionally differ:
- isr: recorded callbacks, saturation UINT32_MAX; not every physical motion.
- wake: source observed in the post-WFI/clock-restore window, coalesced per STOP
  return. Not proof of the sole electrical wake cause or precise simultaneity.
- confirmed: foreground batch with source evidence: RTC callback+valid clock,
  LIS2 WU_IA /ALL_INT WU flag, KC FIELD_RISING/FALLING status bits.
- handled: foreground service attempts, including level/retry recovery.
- unknown: service without a confirmed reason, including failed status reads.
- fieldOn/fieldOff: successful field samples, not change/IRQ counters. `field`
  in status is only the last sample (-1 unknown), not a continuously fresh value.

Source bits and physical flags can merge transitions. Never equate ISR, field
samples or source reads with a complete count of physical events or NFC writes.
`counters clear` resets diagnostics atomically but retains pending work, fault
state and service/retry budgets. Explicit off cancels work after detaching owned
producers; subsequent run clears stale source state before re-enabling producers.

## Safe STOP and bounded faults

Before WFI: software replies drained, UART flushed/ended, I2C transaction done
and Wire ended. Under PRIMASK, disable SysTick/clear only its own pending tick,
temporarily clear debug STOP keep-alive, then check mailbox, enabled pending
NVIC, owned EXTI pending, RTC WUTF and active external pins together. No blanket
pending clear. A new enabled IRQ between final check and masked WFI prevents
indefinite sleep/wakes WFI, then is serviced on PRIMASK restore. Only owned RTC
WUT flags are cleared during arm/disarm, never external pending on each sleep.
After return restore clock/SysTick, service all recorded sources before standby.
Exact hardware race placement remains untested; logic model tests are not a
measurement of that narrow physical timing window.

Active latch prevents sleep. Failed reads retry at most3 consecutive times with
100ms awake spacing. A successfully read but still-active latch is not called an
I2C failure: recheck after100ms (longer than the12.5Hz/80ms sample interval),
bounded by128 external service attempts per run. Thus continuous motion can hold
the CPU awake temporarily, not trigger a20ms false-fault; endless motion is
quarantined after a bounded service budget. Faulted source is detached, best-effort routed off/powered
down (LIS2 ODR0) or dynamic GPO disabled, and reported. Cleanup failure reported;
no blind infinite retry. Other available sources and RTC deadline continue.
Wire's installed twi.c has100ms I2C_TIMEOUT_TICK; transactions are blocking but
bounded and only foreground. This test adds no independent global bus reset;
installed Wire.begin itself performs bounded recovery clocks if SDA is held Low.
It is a standalone bus owner, not suitable for concurrently shared driver owners.
Wire3.0.0 also allocates minimum RX/TX buffers via realloc and frees on end:
application events are fixed-size, but the complete core is NOT zero-heap.
Three unexplained post-WFI returns abort to debug instead of a wake storm.
Hardware LSE loss while already asleep has no independent watchdog rescue here;
hardware fault injection/clock-loss robustness is deferred, not claimed solved.

## UART and lifecycle

LF/CRLF, fixed32byte parser (31 data bytes), embedded NUL invalid, oversized line
discarded through LF, partial command retained. No String, per-loop RX budget,
fixed640byte TX ring with admission backpressure. Read replies; flooding beyond
the hardware RX ring is not supported. Debug leaves UART usable.

| Command | Behavior |
| --- | --- |
| help | Commands |
| status | mode, enabled/fault pairs, pins PB0/PA3, LPD, UTC validity, last field/error |
| probe | Explicit one-shot debug-only KC register/PA3 diagnostic, never IRQ proof |
| counters | Three source lines then END counters |
| counters clear | Diagnostics reset, pending/budgets preserved |
| mode debug | Cancel probe/automatic standby when command can be processed |
| run N | N=1..60; initialize/reinitialize sources, preflight, bounded RTC run |
| off | Stop owned sources, return debug; never reset calendar/backup |

Each run requires ready LSE/RTC, valid time-of-day read, a **new** WUT callback
and observed clock advance within1500ms while awake. Only then emits RUN_READY
and starts the N-second deadline. Failure stays/debug returns with error.
Total command window includes bounded setup and up to1.5s preflight. Periodic
1Hz RTC wakes guarantee foreground deadline checks while hardware is healthy;
sensor wake does not restart the deadline. No STOP duration uses millis.
RUN_BEGIN warns UART RX cannot reliably wake STOP; commands sent asleep may be
lost. Send mode debug/off only while awake/UART accessible. run ends with
RUN_END reason/requested/elapsed/stop metrics and batched source counters.
External sources remain serviceable in debug; off detaches them, future run
reinitializes. Dynamic GPO disable is not permanent across ST25 power/LPD boots;
detached MCU EXTI is the off guarantee, RF itself is not disabled.

## Build and validation

Existing FQBN:
`STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX,xserial=generic,usb=none,opt=oslto,dbg=none,rtlib=nano,upload_method=swdMethod`

```powershell
& 'G:/Google Drive/Application/Arduino-cli/arduino-cli.exe' compile --fqbn 'STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX,xserial=generic,usb=none,opt=oslto,dbg=none,rtlib=nano,upload_method=swdMethod' --build-path "$env:TEMP/Pebo-EventLoopStopTest-20261025" FW/EventLoopStopTest
& "$env:LOCALAPPDATA/Arduino15/packages/STMicroelectronics/tools/xpack-arm-none-eabi-gcc/14.2.1-1.1/bin/arm-none-eabi-g++.exe" -std=gnu++17 -Wall -Wextra -Werror -fsyntax-only FW/EventLoopStopTest/tests/LogicChecks.cpp
python FW/EventLoopStopTest/tests/bench_verify.py --help
```

Compiler-executed host constexpr checks: merged/all-source dispatch, arrival
after snapshot surviving old service, saturation, repeated starts/stops, midnight
deadline, retry wrap and exhaustion, parser boundary/recovery. Mailbox model
does not prove actual NVIC/preemption latency; PRIMASK implementation is reviewed
separately and exact race/precise simultaneous physical trigger remain pending.
Bench script has no instrument dependency/control. Explicit target and image
identity check before flash; --existing verifies byte-for-byte without reset.
No debugger accesses during run. Auto tests10/60/off/re-run1 and parser; human
motion/RF/mixed tests finite20/20/30seconds. Preserve failed evidence, never
replace actual PC RX with debugger buffers. User controls power throughout.
Physical field-removal wake and every motion count are not pass criteria.

Manual: hold still/phone away for run10 and run60; then after RUN_READY gently
move2seconds and put down; separately hold unlocked NFC phone near for5seconds
and remove. Combined run: move first, phone second, within30seconds. Read
summary/counters, verify debug response, off then run again. STOP success is
not a measured current or battery-life claim. Results and map budget below are
updated after actual execution; canonical status is `docs/handoff.md`.

## 2026-10-25 actual results and remaining blocker

Stage4 is **partially accepted**, not all-source PASS. Existing toolchain build,
flash/verify and final byte-for-byte readback PASS. Final image SHA256:
`a5cd6df49ec35dc828d850e359cafa633734b47ac5a61496e80ef83516e1accc`.
Evidence in tests/results: host-20261025 PASS; bench-20261025-02 PASS10/60s
RTC deadlines, UART recovery, off/re-run1 and parser bounds. Final motion03
PASS:20s,35STOP, motion ISR17/wake17/confirmed17, handled34/errors0/no fault.
Extra17 unknown service checks are not extra physical movements. Earlier20ms
guard/64-service-cap failures retained; final100ms/128 policy avoids false
isolation in this limited motion test, not unlimited-motion robustness proof.

RF01 and mixed01 FAILED acceptance: no GPO ISR/wake. Phone tag detection and
debug EH=0C/FIELD1 samples prove field presence only (one debug I2C failure
retained). staticGPO1=11/gate1/RF00; routes-20261025 confirms PA3 input/NOPULL,
EXTI3 PA rising/masked-in, IRQ6 enabled, independent PB0/EXTI0/IRQ5 and PA4High.
No physical GPO waveform or VDCG voltage measured. Do not infer a hardware fix
or fake wake with polling. Need user-approved electrical diagnosis, then RF
and combined acceptance again. LPDHigh field-removal wake is not guaranteed.
No manual stimulus needed until that diagnosis is arranged.

Final timeValid0 correctly marks uncalibrated RTC; WUT deadlines work without
calendar validity. No date or backup reset, no new backup slots. Final off/debug,
UART closed, user supply unchanged, no instruments opened or power measured.

## Final linker budget

| Item | Bytes |
| --- | ---: |
| Arduino reported Flash |31260|
| Actual binary including omitted sections |31480|
| Flash capacity / spare |32768 /1288|
| .data / .bss |324 /2536|
| Static RAM / remaining RAM |2860 /5332|
| Minimum heap / stack reservation |512 /1024|
| Reservation alignment / unreserved gap |4 /3792|

Map: `%TEMP%/Pebo-EventLoopStopTest-20261025/EventLoopStopTest.ino.map`.
RAM origin20000000 length2000; Flash08000000 length8000. Large static owners:
TX640, UART344, unused core SerialLP1 global344, GPIO IRQ table320, stdio312
plus impure76, Wire228, core serial-debug188, source states84. Sizes are not
independent extra allocations. Core Wire RX/TX dynamically allocate at least
32B each, free on end; application does not add dynamic queues or String.
Minimum linker reservations are NOT measured stack/heap peaks; no reliable
high-water measurement performed, remaining RAM is not all safe stack.
Only1288B Flash spare is a significant future-integration constraint: reduce
diagnostic/core overhead before adding previous stages, do not sum sketches.
Precise race/simultaneous physical injection, clock-loss rescue, fault injection,
long-term RTC accuracy and electrical/current acceptance remain unverified.
