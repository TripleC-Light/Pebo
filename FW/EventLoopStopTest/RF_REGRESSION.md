# RF wake historical regression, 2026-10-25 (Taiwan)

## Scope and preserved state

PC-02 TripleC, main fdacaf6, dirty tree preserved; no commit/push/checkout/reset.
Before investigation: tracked binary diff, status, and complete EventLoopStopTest
copy saved at `%TEMP%/Pebo-RFRegression-20261025`. This is a local safety copy,
not a cloud/Git publication claim. Previous stage2/3 work untouched. No PPK2 or
measurement instruments opened; user supplies board power throughout.

## Historical source and evidence

- Source commit `11e2f6e` (2026-10-09 04:19:25 +08), sketch blob
  `0efdbd2be75b99bfcf87d0d1c2cf6bf2d6da1cd5`, identical to current sketch.
- Included PeboST25DV cpp/h blobs `1ae3b4870c9feba6fca72f6281a0c9d7663eeb11`
  and `cd6f64f0f867f9a1cd2afe51dc5d482591cb3040`, also identical.
- `c8dd924` (04:22:52 +08), docs/nfc-protocol.md records actual phone GPO
  STOP wake, idleLow/activeHigh and source clear. `1ad612e` architecture records
  independent successful paths. `8bd77d1` (04:22:49 +08), docs/power-budget.md
  records historical ~4.43uA at2.8V, not current integrated current.
- Exact historical ELF/binary, UART capture and toolchain manifest were not
  found in repository/history searched. The detailed numeric UART clues given
  by user are historical testimony, not independently recovered old raw logs.
  Rebuild preserves source/dependencies, cannot prove byte-identical old image.

Current original rebuild: STM32duino3.0.0, GCC14.2.1, existing GenericL031G6Ux
LTO/nano/noUSB FQBN. Arduino Flash24512B, actual24732B, staticRAM2000B.
Image SHA256 dc875a43b064cbbed1f8d6059903ec8c51763085e345b54282a16061c297aa75.
Original sketch unchanged. Before flash integrated probe: GPO1=11, RF00,
EH08/FIELD0, PA3Low/pull0. Thus original's conditional static EEPROM write
was not entered; actual boot has no GPO1_WRITE log. No security session opened.

## Differences (source and actual evidence, not presumed causes)

| Condition | Original independent | Integrated a5cd6df4 baseline | Interpretation |
| --- | --- | --- | --- |
| Supply/VDCG | No software switch; mapped ST25_VDD | Same hardware, no switch | Actual VDCG voltage unmeasured; pinmap is wiring basis, not voltage proof |
| LPD startup | OUTPUT thenLow,1ms; staysLow through setup | OutputHigh boot; scopedLow1ms for each access | Different active windows; both High in STOP |
| LPD wake/service | Low across all reads; High next STOP | Low within scoped cause read, immediately High on return | Check field/cause retention separately; no silent policy change |
| GPO1 | Reads system57/0000; conditional OR11 write only if missing | Reads FIELD_CHANGE bit; refuses absent bit, never writes static | This board reads11 in both |
| GPO2 | Not accessed | Not accessed by original probe | KC57/0001 controls pulse timing/I2C causes; value not yet captured here |
| Dynamic gate | Writes53/2000=1; some reads unchecked | Writes1 and verifies; off writes0 before reinit | KC always writable without password; reported1 in both |
| RF/EH |53/2003,2002 read after boot/wake | Probe same; EH during source service | RF00; EH08 absent/0C present, not IRQ proof |
| IT_STATUS |53/2005 once init; once after wake | Once init; only foreground event/level/retry service | Read-to-clear, no diagnostic polling of IT |
| PA3 | INPUT/NOPULL, idleLow choosesRISING; reattach eachSTOP | INPUT/NOPULL fixed documentedRISING, attach perrun | No blind pull/edge change; CMOS requires powered VDCG |
| EXTI/NVIC | IRQ6 priority0; all other peripheral IRQs disabled/cleared | IRQ5/6 priority3 from installedcore; RTC IRQ2 priority0; preserve others | Do not copy original blanket disable into integration |
| Pending | ClearsEXTI3/NVIC6 eachsleep and afterread | Owned pending checked before WFI, no external blanketclear | Integrated race protection retained |
| RTC/LIS | Neither initialized; unusedPB0/PC pins analog,RTCclockdisabled | PB0EXTI0 + PC14/15LSE,WUT1Hz | Distinct EXTI0/3, routing readback passed; shared I2C100kHz |
| STOP | Unmasked HALWFI; GPIOclocksdisabled; noRTCdeadline | PRIMASK atomic pendingcheck/WFI, clocksretained, RTCdeadline | Test each layer, no bypass for apparent success |
| Restore | SystemClock_Config; pin/I2C/UART rebuild | Same clock restore plus tick/UART handling; ISR records first | Original disablesSysTick permanently, not a future app template |
| Field atstart | Actualboot EH08,ITclear00 | Earlier failedRF field at exactstart undocumented | Physical instructions/finite capture need explicit timing |

Directly read official ST DS13519 Rev8, pp7-8/43-46:
https://www.st.com/resource/en/datasheet/st25dv04kc.pdf . KC UFDFPN12 CMOS uses
powered VDCG, positive pulses, no external pull-up requirement. LPDHigh does
not guarantee FIELD_FALLING on GPO. GPO1/2 system0000/0001; gate2000; EH2002
b2FIELD_ON; RF2003; IT2005 b4RISING/b3FALLING read-to-clear. Gate b0 overrides
static gate; static write password-protected. No inaccessible-PDF inference.

## Current original regression evidence

`tests/results/rf-history-20261025-01..03/session.json` retain raw PC UART.
First round compiled/flashed/verified, rounds2/3 UART-only without reset or SWD.
All three35s windows observed WAKE:ST25GPO +IT10 +EH0C +gate01 +afterclear0,
then ENTER STOP. Counts1,2..4,5 respectively. GPO=0 in the same print means
the original OR-condition used its ISR flag (not active pin level); original
has a boolean ISR mailbox, not a separate numeric ISR counter. Several RF
pulses during one phone interaction are not several physical phone approaches.
No IT08/FIELD_FALLING captured; removal wake not claimed. Observed return to
STOP is sketch output plus subsequent real wake, not current measurement.

## Integrated investigation

Original firmware was restored to unchanged integrated image a5cd6df4...;
rebuild PASS31260B reported/31480B actual, data324+bss2536=2860B.
`rf-integrated-awake-20261025-01` is a tool connection failure, NOT RF failure:
original STOP disables debug keep-alive, HOTPLUG returnedDEV_TARGET_CMD_ERR.
Switched only flash-mode initial identification to existing under-reset method;
`--existing` still uses HOTPLUG/readback without reset. No firmware change.

`rf-integrated-awake-20261025-02` PASS: run1 ends with sources enabled/debug,
then45s awake observation, GPO ISR2/confirmed2/handled3/errors0/wake0, fieldOn3.
No new diagnostic firmware or interrupt injection. Extra unknown1 is a service
check, not a physical event. Script ends off and closes UART.

`rf-integrated-stop-20261025-01` PASS: explicit pre-arm probe FIELD0/EH08,
60s run, RTC elapsed60, STOP62, GPO ISR2/wake2/confirmed2/handled2/errors0,
lastSt10; RTC ISR61/wake60, no unknown wake/quarantine; returns debug thenoff.
User confirms two approaches/removals in this window. Counts are actual RF
source observations, not an assumption based on that confirmation. Wake is
the defined post-WFI source-observation window, not sole electrical-cause proof.

### Root cause status

No reproducible firmware root cause identified. The exact pre-investigation
final integrated binary succeeds without any source/offset/pull/priority/edge/
LPD/PRIMASK/RTC/LIS changes. Therefore no speculative firmware fix is justified.
Earlier RF failure was an un-reproduced integrated observation, not hardware
non-support. Previously failed RF used older b46... build, mixed used eb8...
64-service policy; final a5cd... had not had successful RF acceptance before
this investigation. Do not pretend all older failed builds are identical.

Now excluded as categorical explanations: absent configured FIELD_CHANGE,
inherent inability of PA3 callback routing/current priority, permanent hardware
inability to generate GPO, inherent incompatibility of current protected STOP.
Not excluded intermittent supply/contact/antenna/phone RF behavior. Original
first wake was23.25s after capture began, so20s windows can be too short for
human interaction; this is a credible test limitation, NOT proven old root
cause. Old logs lack RF transition timestamps, so they cannot determine whether
field existed at arming or changed within the exact active window.

Minimal changes are test-side: finite historical capture, explicit awake RF
phase, configurable longer windows and optional pre-arm FIELD0 check (no IT
read), plus under-reset flash identification. No speculative production fix,
hardware change, pull/edge change, Flash slimming or fifth-stage work.

### Final regression matrix

All integrated cases below use the same a5cd6df4... binary, verified by full
31480B readback before each session, no debugger accesses during timed runs.

| Evidence directory under tests/results | Result | Actual observations |
| --- | --- | --- |
| rf-regression-baseline-20261025-01 | PASS | run10/60: RTC elapsed10/60,STOP10/60, GPO/motionISR0; off/re-run1, UART parser/roundtrip/recovery PASS |
| rf-regression-motion-20261025-01 | PASS |30s,STOP55,motion ISR/wake/confirmed27,handled54,errors0; GPOISR0 |
| rf-regression-mixed-20261025-01 | PASS |60s,STOP75,motion ISR/wake/confirmed14,handled28; GPO ISR/wake/confirmed2,handled2,lastSt10; RTC61 callbacks/59wake |
| rf-regression-host-20261025.json | PASS | constexpr policies/interleaving model and Python AST |

Motion unknown27/14 are extra service checks without a new latched cause, not
errors or counted physical motions. Every final case reached RTC deadline,
returned debug with healthy sources/no error/no unknownWake, thenoff with all
sources disabled and UART closed. RF was absent in explicit pre-arm probe for
baseline/motion/mixed. Off then re-arm succeeds across these sessions, including
another real RF wake in mixed after RF-only session. Counters are bounded
observations; merged IRQs mean sums need not equal individual physical events.

Status `field` and RUN_END `lastSt` are last-sample diagnostics, can remain1/10
across off/countersclear even in a stimulus-free run. They are NOT fresh RF
evidence; fresh probe FIELD0 and current counters0 distinguish that case.

### Limits and final state

No power measurement, VDCG rail voltage/waveform, field-removal wake, precise
simultaneous/race injection, long-run noise/storm qualification or hardware fault
injection. No firmware root cause claimed and no speculative firmware patch.
Historical independent sketch disables RTC clock while sleeping; calendar may
pause during that regression. No date/backup reset or calibration performed;
integrated timeValid remains0 and accurately indicates invalid wall-clock time.
No proper UTC/time-retention acceptance from this RF investigation.

Integrated Flash actual31480/32768 (spare1288), staticRAM2860/8192, minimum
heap512/stack1024; peaks unmeasured. Both sketches/modules unchanged relative
to preserved snapshot (all four integrated source SHA256 checked). Only test
scripts/docs/evidence added/changed. Supply user-controlled2.8V unchanged;
STLINK observed2.70..2.71V, no supply instrument opened. Final off/debug,
UART closed, no helper left running. No commit/push. Do not infer long-term
reliability, battery life or production qualification from these finite passes.

## Electrical escalation only if needed

No hardware modifications or instrument commands authorized. If software
comparison remains inconclusive, user may measure common-ground VDCG at the
ST25 package VDCG pin or its verified accessible net (confirm board orientation
before probing), VCC/ST25_VDD, PA4/LPD, and both sides of GPO100ohm resistor.
Expected powered CMOS rail near board supply; LPDHigh beforeSTOP; GPO positive
pulse on new RF field with field absent first. Scope single-shot rising trigger
on ST25-side GPO and MCU-side PA3 distinguishes no output from route loss;
normal pulse may be only tens/hundreds of microseconds, DMM cannot prove it.
Use schematic-labelled accessible pads, not guessed package probe coordinates.
If VDCG absent, wiring/power cause still requires operator confirmation; do not
infer damage or add pulls. Do not connect probes until user chooses this step.
