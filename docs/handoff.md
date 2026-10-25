# Pebo handoff log

## 2026-10-25: Native B local annotated recovery baseline

User explicitly authorizes one scoped baseline commit and a new annotated tag
baseline/native-hal-eventloop-v1; no push. Include native B/vendored licenses/
hash manifest/evidence, full stage4 A reference/test tooling, pure TimeLogic.h
dependency and relevant native/stage4 handoff/HIL/hardware notes only. Stage2
FeedbackTest, remaining stage3 files and their unrelated dated document entries
stay uncommitted. Baseline instructions: FW/NativeEventLoopStopTest/BASELINE.md.
Vendor text conversion disabled to preserve SHA256 across OS checkouts. Validate
build from staged-only export before saving; tag annotation lists PASS/limits.
Resolve commit from the immutable tag instead of embedding a circular self SHA.
Current board remains native e7e4e403... off/debug, UART closed, instruments and
power untouched; this Git-only capture does not add new HIL claims.

Next user-authorized scope after capture: independent NativeShakeProgressTest,
first valid RTC/reset retention, then native blue8-step UART display and bounded
real-sample shake observation; no drinking recognition/sound/NFC app protocol.
No stage5 commit/push authorized. Do not start feature edits until tag saved.

## 2026-10-25: Native HAL feasibility B implemented; functional comparison PASS

PC-02 TripleC, main fdacaf6/cached origin ahead3, no fetch/commit/push. All prior
uncommitted stages preserved; no changes to successful Arduino sketches/modules
or shared pure EventLogic/TimeLogic/bench scripts. New FW/NativeEventLoopStopTest
contains native main/Board/RTC/source wrappers, HAL config, linker, Python GCC
builder, vendored manufacturer files/licenses/SHA256 manifest, README, state
inspector and actual result JSON. Relevant HIL/machine/handoff updated only.

Read AGENTS/README/handoff/machine/HIL/pinmap, stage4/RF_REGRESSION/time modules,
installed clock/pinmaps/HAL/CMSIS and linker/build flags. Actual L031G6U6TR32KB/
8KB; USART2 PA9TX/PA10RX AF4/115200 (notUSART1), I2C1PB6/7@100kHz, PB0EXTI0
and PA3EXTI3 rising/noPull, fitted KC-JF6D3 GPO via100ohm/noexternalpull unchanged.
PA4 OutputHigh STOP/temporaryLow I2C startup policy retained. HSI16PLLx4/2=32MHz,
LSE32768, RTC127/255/WUT1Hz. No date/backup reset/newBKP; readsBKP2/3 only.
ST startup/vector/data/bss/MSP/SysTick/IRQ explicit. Same PRIMASK pending check/
WFI, snapshot dispatch/saturation/retry/fault budget, run1..60 preflight/deadline.
HAL UART IT fixed64RX/64TX plus640appTX; no application allocation/usableheap.

No CMake/Ninja onPATH: existing Python+GCC command-line builder needs no new
software/download/GUI. GCC14.2.1-1.1/newlib-nano4.4.0, Os/LTO/sections/GC,
integer snprintf/no forced float formatter. HAL1.10.7 BSD3, Device1.9.4 and
Core(M)6.1/distribution6.3 Apache2 sources locally vendored unchanged (109
manifest entries), no Arduino core/library wrapper link or personal SDK paths.
Full commands/versions/sections/largest symbols in tests/results/build-report-
20261025.json; build twice produced identical native SHA256 below. CLI accepts
PATH/ARM_GCC_BIN/explicit--toolchain-bin. All pure policy checks/vector checks PASS.

A Arduino re-created a5cd6df4..., Flash31480/staticRAM2860 unchanged. B native
image e7e4e403763de5e1175f6b891f278817262aa5712ac69deb7c38a43426a367c0:
text16656/rodata1208/data100/bss1252/vector192/unwind8, Flash18164 (-13316,
42.30%), staticRAM1352 (-1508,52.73%). Flashspare14604; RAMafterstatic6840;
minimumstack1024/heap0 leaves5816 unreserved, NOT measured safe-stack budget.
Largest Bmain4220 afterLTO, consolebegin1076, USART2IRQ840; appTX640/uart136/
sources84/I2C84/RX64/TX64 dominate RAM. No slim diagnostic C: retained same
commands/status/counterstrings/buffers; fixedI2C timing removes runtime search,
explicit callbacks/selected HAL remove generic maps/globals/wrappers. Savings
are this implementation/configuration, not all uniquely attributable to Arduino.
Generic bus recovery differs (not ported), HAL_Delay startup1..2ms vs1ms, UART
flush now bounded100ms. No exact power/electrical equivalence claimed.

Fresh STLINK56FF6C066688555210351387/ID425/32KB/Cube2.11.0/SWD4MHz, Prolific
COM5/115200. Existing normal under-reset per-image flash/verify PASS; fullreadback
before each later session PASS; no optionbyte/RDP/mass erase. Tests/results:
native-baseline01/02 PASS10/60/off/re-run1/parser, no external ISR/storm even
after stimuli; motion01 PASS30s, ISR/wake/confirmed26/STOP53/errors0; awakeRF01
PASS45s ISR/confirmed1,wake0; stopRF01 PASS60s ISR/wake/confirmed2,IT10/STOP62;
mixed01 PASS60s motion31/GPO1 ISR/wake/confirmed,STOP89/errors0. Every timedrun
RTCdeadline/debug/UART recovery, nounknownWake/quarantine. External RF tests
pre-armFIELD0, no ITdiagnostic polling/softwareIRQ injection/debugduringSTOP.
Original scripts reused without adaptation. Actual stimuli are operator-assisted.

Before/after/final RTC state inspectors PASS: fiveBKPwords0 remain0, PRERsame,
LSEsource/ready, PA4High,WUTstopped. CurrentUTCinvalid/timeValid0; nonzero valid
schema retention not physically tested this round. No clock/calibration writes.
Stack peaks unmeasured; GCC .su main304 is only a local frame, not wholechain/
libc/ISR bound. Native _sbrk rejects requests; printf still links allocator
branches, no usable dynamic heap. No clock/I2C-fault/precise race/simultaneous/
long-run/electrical waveform/power tests. Historical RF non-reproduction cause
still OPEN; native success is not a root-cause finding.

Recommendation: meaningful resource headroom makes native HAL a staged migration
candidate; NOT an approved wholesale migration. Keep Arduino references; decide
after stress/peakstack/fault/power and maintenance review. No fifth-stage work.
CURRENT BOARD: native B e7e4e403..., off/debug/all sourcesdisabled, UARTclosed,
no helpers left running. User2.8V unchanged (STLINK2.70..2.71V), no PPK2/other
instrument handles or COM6/7 opened. Handoff saved locally; next step is review
comparison and choose migration scope, not automatic feature development.

## 2026-10-25: RF historical regression and integrated revalidation PASS; cause OPEN

PC-02 TripleC, main fdacaf6, cached origin ahead3; no fetch/commit/push. Preserved
all prior uncommitted work. Snapshot EventLoopStopTest +tracked diff/status at
%TEMP%/Pebo-RFRegression-20261025. Read AGENTS/README/hardware/pinmap/HIL/handoff,
both tests/modules, installed core APIs and official KC DS13519Rev8 directly.
No power/instrument operation; user supply only, freshSTLINK56FF6C066688555210351387
ID425/32KB/Cube2.11.0/SWD4MHz, COM5Prolific067B:2303/115200.

Historical source11e2f6e/ST25GpoWakeTest blob0efdbd2... and included driver blobs
match current files. c8dd924 records phone GPO success;8bd77d1 records historical
4.43uA, NOT current power. Exact old ELF/UART/tool manifest unavailable; cannot
prove same old machine image. Current unchanged original build PASS24512B
reported/24732B actual/staticRAM2000; image dc875a43... . Three35s windows
rf-history01..03 observed WAKE+IT10/EH0C/gate1/clearPin0 thenSTOP, counts1..5;
rounds2/3 no reset/SWD. Same-print GPO0 implies old ISR boolean true; not a
separate numeric ISR counter. No FIELD_FALLING08 claim. Original disables RTC
clock in STOP, may pause calendar; no date/backup reset/calibration performed.

Restored/rebuilt original integrated a5cd6df49ec35dc828d850e359cafa633734b47ac5a61496e80ef83516e1accc,
all four sketch/module hashes unchanged. Flash/verify/readback PASS31480B,
staticRAM2860B, Flash spare1288B; no slimming/new stage/no pull/edge/LPD/priority
or concurrency changes. First restore HOTPLUG failed while original asleep,
retained as awake01 tool failure; under-reset flash identification fixed that
tool path only. Existing-image checks still readback without reset.

New evidence tests/results: rf-integrated-awake02 PASS45s ISR2/confirmed2/wake0;
rf-integrated-stop01 PASS60s STOP62,GPO ISR/wake/confirmed2,IT10/errors0;
rf-regression-baseline01 PASS10/60/off/re-run1/parser/UART; motion01 PASS30s,
STOP55,motion27 ISR/wake/confirmed/errors0; mixed01 PASS60s,STOP75,motion14,
GPO2 ISR/wake/confirmed,RTCdeadline/debug/UART recovery, no errors/quarantine/
unknownWake. Pre-arm FIELD0 verified; noIT diagnostic polling or software IRQ
injection. Host constexpr/policy/AST PASS. These are finite functional results,
not precise simultaneous/race or reliability/power qualification.

No reproduced firmware root cause: prior failedRF used b46.../mixed eb8...,
current final a5cd... works unchanged. Short20s human windows/RF-at-start/phone
polling/contact/supply are hypotheses, NOT established causes. No inference of
hardware damage/non-support. VDCG voltage/waveform and RF removal remain pending;
no hardware changes indicated by current tests. Full comparison/limits in
FW/EventLoopStopTest/RF_REGRESSION.md. Only bench script, new historical capture,
README/report/JSON and relevant docs changed. Previous failed evidence preserved.

Final off/debug, sources0/0, UART closed, no helper left running; user supply
unchanged (STLINK2.70..2.71V); COM6/7/PPK2/instruments not opened. timeValid0
explicit, no UTC accuracy claim. Handoff saved locally. Next: retain these as
regression baseline; if intermittent RF failure recurs capture exact operation
times/config/power, compare before changing firmware. Flash headroom remains a
future-integration risk, deliberately no large optimization this round.

## 2026-10-25: EventLoopStopTest stage 4; RTC/motion PASS, RF wake unresolved

PC-02 TripleC, main fdacaf6; local only, no commit/push/fetch. Previous
FeedbackTest/TimeManagementTest and dated observations preserved. Added
FW/EventLoopStopTest sketch, EventLogic/DeadlineRtc/WakeSources modules, README,
constexpr checks, UART bench/probe/route scripts and actual JSON evidence.
Updated pinmap, HIL, machine profile and NFC regression note.

User confirms ST25DV04KC-JF6D3, GPO via100ohm toPA3, no external pulls.
Directly checked ST DS13519 Rev8: this package CMOS activeHigh/idleLow needs
VDCG powered; PA3 NOPULL is conditional on that supply, not an all-power-state
guarantee. VDCG-off signal undefined; LPDHigh field-falling wake not guaranteed.
No hardware change justified yet; actual VDCG voltage/GPO waveform unmeasured.
PA4 remains outputHigh in STOP; temporary foreground I2C access restores it.

Minimal ISR records under PRIMASK (including nested ISR RMW), foreground atomic
snapshot dispatches all sources. Masked final pending/pin recheck plus WFI
closes check/sleep race without clearing unrelated pending IRQs. RTC 1Hz LSE
deadline is preflight checked; no RTC date/backup reset or new BKP allocation.
BKP2/3 validity only read, core BKP1/HID4 untouched. Failed sources bounded by
3 real consecutive failures or128 service attempts; valid held motion retried
after100ms, not falsely called I2C failure. Debug boot; off/re-run supported.

Final image SHA256 a5cd6df49ec35dc828d850e359cafa633734b47ac5a61496e80ef83516e1accc.
STM32duino3.0.0/GCC14.2.1/LTO build PASS: Arduino31260B, actual31480B/32768
(1288B spare); data324+bss2536=2860B. Heap512/stack1024 minimum reservation
plus4B alignment leaves3792B unreserved, NOT measured safe stack. Wire allocates
at least64B heap; stack/heap peaks unmeasured. Flash headroom is a major
integration risk; do not sum independent test memory totals.

STLINK56FF6C066688555210351387, ID425/32KB, Cube2.11.0/SWD4MHz; flash/verify
and final byte-for-byte31480B readback PASS. ProlificCOM5/115200 actual RX:
bench-20261025-02 PASS run10/run60/off/re-run1/parser boundaries; RTC elapsed
10/60, STOP10/60, returns debug. Host constexpr/policies PASS, not NVIC emulator.
Final motion-20261025-03 PASS: elapsed20, STOP35, motion ISR/wake/confirmed17,
handled34, errors0, no quarantine; ends off/debug. Unknown17 are extra service
checks without a fresh latched cause, not17 additional physical events.
routes-20261025 PASS_MCU_ROUTES_ONLY: PA3 input/NOPULL, PB0 input, EXTI3 PA,
EXTI0 PB, rising/mask9, IRQ5/6 enabled, PA4High. No debugger during timed STOP.

RF/mixed HIL NOT PASSED: rf01 and mixed01 contain zero GPO ISR/wake despite
user-confirmed phone detection. Debug present probe observed EH=0C/FIELD1 twice
(one failed I2C sample), absent/removal EH08/FIELD0; static11/gate01/RF00.
These prove sampled RF presence/config, NOT physical GPO or STOP wake. Mixed01
also hit older64-service cap; final128 policy passed motion retest, combined RF
acceptance still pending. Earlier no-power and old20ms guard failures retained.
Do not overwrite historic GPO success with an invented current PASS.

Current timeValid=0 after supply interruption; RTC deadline works without valid
calendar. No calibration/date write performed. Long-term accuracy, exact race,
precise simultaneous stimulus, fault injection, stack peaks and power numbers
unverified. Supply remained user-controlled2.8V (STLINK2.71V); PPK2/instruments
and COM6/7 never opened. Final off, UART closed, no helper remains; supply not
switched. Handoff saved. Next: resolve powered VDCG/physical GPO route versus
LPD strategy with user-approved electrical checks, then repeat RF/mixed HIL;
reduce Flash before integrating other stages. No battery-life claim.

## 2026-10-25: Full workspace Git publication authorized

Push blocked by automated approval review: user must explicitly approve the
destination https://github.com/TripleC-Light/Pebo for the selected source,
documents, APK/ZIP and test evidence. No push executed. Next: obtain destination
confirmation, normal push origin main, then verify remote main equals HEAD.

Publication preparation completed: local content commit `b3adc7b` contains
141 changed files; merge commit `d6d9f2b` integrates all five remote commits
through `c8dd924`. README, NFC and power-budget conflicts were resolved by
preserving both histories; older power measurements remain labeled historical.
Source/document whitespace checks and conflict-marker scan passed. Archived
Gradle logs/HTML retain original whitespace warnings as evidence, unchanged.
Normal push is the remaining publication step; verify remote main equals HEAD
before treating this handoff as synchronized. Git reported an existing
packed-refs.lock during maintenance, but both commits completed successfully;
no lock was deleted. No device, UART or power operation was performed.

User explicitly requests committing and pushing all previously uncommitted
project content, including Android/protocol drafts and bring-up/LED/UART work.
Fetched origin successfully: main remote advanced5 commits from11e2f6e to c8dd924,
covering verified wake/power documentation. Preserve both local/remote history;
save local work first, merge remote docs, then normal push (no force push).
Nonignored source/docs/evidence selected; ignored cache/venv/ELF/bin/map artifacts
remain local under existing .gitignore, including raw register/measurement bins.
APK/ZIP/images are intentional handoff evidence, not private signing keys.
File-name and private-key/token-pattern checks found no candidate credentials
in selected source/doc paths. No firmware/device/supply operations in Git step;
manual supply remains user-controlled. Final commit/push result recorded below.

## 2026-10-25: UART-only manual-power retest PASS; previous PC RX blocker resolved

User corrected port to COM5; pyserial verified Prolific067B:2303 before opening.
COM6/COM7 Nordic ports not opened. Supply entirely user-controlled: no PPK API,
instrument commands, power toggling, ST-LINK/reset/flash or firmware edits.
First help received ERR too_long (recorded FAIL, input boundary not clean).
Then sent LF solely to resynchronize, received ERR invalid, drained it and
performed seven actual PC UART round-trips at115200/8N1: help,8,off,show(CRLF),
invalid9,40-byte overlong input,off. All replies exactly matched,7/7 PASS.
off preserved level8; show reported HHHH; final off reported OOOO.
Evidence: FW/BlueLedProgressTest/tests/results/uart-manual-power-20261025-01/
session.json(first mismatch) and -02/session.json(resync plus7 passes).
Board/adapter return path is now verified working; original cause of earlier
empty RX was not proven, do not attribute it solely to a wiring or supply fix.
Historical run05 partial remains unchanged; this is a separate UART-only PASS,
not a new full bench power/flash run. Prior LED GPIO/PWM/visual acceptance retained.
Serial closed, LEDs off/current8 retained; supply left unchanged under user
control and not independently queried. No helper left running. TripleC/main
11e2f6e8954aa45e373c615d853d323aac053979, existing work preserved; handoff,
registry/HIL/README and generated evidence saved locally, no commit/push/fetch.
Next: stage1 LED implementation and UART acceptance are available for next work;
future commands still require fresh port/power checks, keep original tests intact.

## 2026-10-25: UART-only retest requested, manual power; port identity pending

User requests another UART test with supply controlled by user; do NOT open PPK2
control/API or toggle supply in this retest. TripleC/main11e2f6e, prior changes
preserved. Read current handoff/machine profile and command replies.
Fresh pyserial/PnP inventory twice: COM5 Prolific VID067B/PID2303;
COM6 Nordic VID1915/PIDC00A MI03 and COM7 same VID/PID MI01,
both serialDC01B057091B. User says UART is COM6. This conflicts with prior
Prolific port identification; exact COM6 bridge purpose/wiring remains unverified.
Asked user which physical UART connects PA9/PA10 and whether power is ON.
No serial port opened, command sent, PPK API/control, ST-LINK action, reset or
flash in this retest. UART retest NOT EXECUTED; do not report PASS/FAIL anew.
Next: resolve COM6 purpose using user's confirmation/current USB mapping,
confirm manual power, then test help/off/show on verified UART only. Never send
ASCII test commands into an unverified instrument interface. Only handoff/registry
updated locally; no commit/push/fetch or cloud synchronization claim. No helper
started, supply state remains user-controlled and not independently queried.

## 2026-10-25: BlueLedProgressTest implemented/flashed; LED acceptance PASS, PC UART RX unresolved

PC-02 TripleC/main11e2f6e8954aa45e373c615d853d323aac053979; previous work preserved,
no commit/push/fetch/cloud synchronization claim. Read AGENTS/README, hardware
pinmap/firmware architecture/HIL/power notes and existing HardwareTest/LED sketches.
Only new FW/BlueLedProgressTest plus directly relevant pinmap/HIL/handoff/registry
notes changed. Old LedBrightnessTest, piezo/other bring-up and official FW untouched.

Added BlueLedProgressTest.ino, ProgressLogic.h, README, .gitignore,
tests/LogicChecks.cpp and tests/bench_verify.py, generated results/images/logs.
Startup all-off/current0; LF/CRLF0..8/show/off/help; 3s hold; zero LED1 low120ms,
off120ms, low120ms then off. Nonblocking millis state, fixed16B line buffer and
256B TX queue; errors/overlong line don't modify level/display or timer. Help
cancels effect and preserves level. No delay/String/blocking RX/periodic logs.

Pins PA5/6/7/8 active High from verified pinmap/HardwareTest. User first authorized
that provisional fill order, then after repeated playback confirmed zero flashes,
low/high distinction and physical water-drop fill order all correct. Updated pinmap.
Installed core's PWM AF5: PA5/PA8 TIM2_CH1, PA6 TIM22_CH1, PA7 TIM22_CH2.
Restricted pattern has at most one low LED: hardware PWM25%/1000Hz only there,
high100% GPIO High; avoids shared-channel conflict without software PWM ISR.
High<100 is compile-time forbidden. Explicit pin AF on every PWM start avoids
PA5/PA8 route reuse; off pauses both timers/channels, restores Output Low.
PA4 stays Output High; no STOP or power-consumption claim. Timer clocks/UART
remain enabled, future TIM2 piezo coexistence/clock gating need review.

Build actual PASS STM32duino3.0.0 / ARM GCC14.2.1-1.1 / Generic L031G6Ux/LTO:
17160/32768B Flash report,2352/8192B static RAM. Bin17380B SHA256
7B48732A6CA10BBF135DAE0B7F0635DE288D964077AB7A9BD53C3250AA913AC9.
ASCII output C:/Users/Edward/AppData/Local/Temp/Pebo-BlueLedProgressTest-20261025;
portable bin/build-summary under new tests/results/build-20261025.
ARM GCC -std=gnu++17 -Wall -Wextra -Werror -fsyntax-only LogicChecks.cpp PASS:
constexpr evaluates nine patterns, initial off,2999/3000ms, wrap,show reset,
off retention,zero phases/cancel,invalid level,CRLF/fragments/NUL/overflow recovery.
Python syntax/help PASS. These logic checks execute in compiler, not target unit-test code.

Latest user instruction explicitly overrode initial no-PPK-operation restriction:
user asked PPK2 supply and confirmed cell removed/sole supply/common GND.
Enumerated PPK2 COM7/COM6, control metadata HW1518 onCOM7, serialDC01B057091B;
UART Prolific COM5,115200; ST-LINK56FF6C066688555210351387/V2J39S7.
Target identified0x425 RevX/32KB; CubeProgrammer2.11.0 SWD4000KHz.
PPK2 source2.8V, ST-LINK indicated2.72..2.73V (not precision rail measurement).
No current acquisition, scope or other instrument control. UART wiring confirmed
TXadapter->PA10/RXadapter<-PA9 plus GND; adapter VCC not required.

Flash+verification PASS under reset; subsequent run04/05 readback matched bin
without rewriting. Runs01..04 fail at PC receiving help; all sent PPK OFF in finally.
User reconnected both directions but PC RX still empty. CPU running verified;
MCU parser received help, TX ring has correct text and drained indices. Thus no
evidence of a frozen MCU or missing incoming command; physical return-path cause
not proven. Do not call UART round-trip PASS or modify pins based on guesses.
Diagnostic run05 verifies MCU TX via matching ELF/SWD and checks real GPIO/AF,
four low-channel duty ratios exactly0.25, timeout outputs low and TIM2/TIM22 CEN0,
off/show retention,invalid/overlong/fragmented input and42 MCU replies. Result
PARTIAL_PC_UART_RX_UNVERIFIED, not full PASS. Original run05 exit0 is historical;
script now returns2 for this diagnostic partial mode and rejects CLI Error text.
Zero blink count/immediate sub120ms cancellation have compiler checks; human
confirmed two flashes, not an automated optical measurement. Slow SWD checks
do not prove every sub120ms preemption timing. No fake full UART/UI acceptance.

Evidence: FW/BlueLedProgressTest/tests/results/bench-20261025-01..05/session.json,
register binaries, uart-diagnosis-20261025, visual-repeat-20261025/session.json.
Explicit host playback0..8, not firmware auto-demo; second playback4s/step accepted
by user: all correct. All power sessions ended with OFF sent/ports closed;
physical VDD OFF not measured. No UART/supply helper left running at final check.
Next: use known-good UART RX/adapter or isolated adapter loopback to resolve PC
return path, then rerun normal bench mode; no need reflash if verify-existing matches.
Do not enable PPK automatically in future sessions without reviewing setup.
Handoff/registry saved locally, uncommitted/unpushed. No NFC/RTC/motion/scoring/
red LED/buzzer implementation or Android changes in this task.

## 2026-10-25: Status crash source fix / Minimal v0.2 draft / local build PASS

Current PC-02 TripleC, main11e2f6e8954aa45e373c615d853d323aac053979.
Prior dirty/untracked work preserved; no fetch/commit/push/cloud sync verification.
Read README, handoff/registry, firmware/hardware/power/NFC documents and Android
source before work. No firmware, wire/model, Gradle, manifest or dependency edits.

Crash root: full interpolated Status string containing literal percent-newline
was passed to String.format(flags), throwing UnknownFormatConversionException.
ProtocolActivity now delegates only Status text to new pure StatusText.kt;
only constant "%04X" is formatted, literal percent/dynamic time text never reparsed.
New StatusTextTest.kt has4 regressions: exact full output/3-vs-1 counts,
0/65/100 percent, percent-bearing dynamic time, real Sample-client Status rendering.
Audited remaining format calls: hex/numeric constants, fixed enum/UID/time/error
labels have no other current literal-percent defect; no unrelated rewrites made.

Actual testDebugUnitTest assembleDebug --no-daemon --console=plain PASS2m51s,
40 tasks executed; XML62 tests=38 protocol+14 block+6 gate+4 Status, all0
failures/errors/skips. Original23-file manifest: only ProtocolActivity changed;
2 new source/test files, all25 match local build copy. Wire/models/config unchanged.
Evidence/APK/log/XML/HTML/current source-manifest/build-summary:
Software/PeboAndroid/handoff/evidence/pc02-20261025-status-fix/.
APK SHA256 CDD03A63757B32B8BF859432F11968B95F175DF67B77650817E790D8949220F0.
Original ZIP and handoff/source-manifest.csv retained as historical snapshots;
Android README now explains current manifest and intentional old hash mismatch.

Build download blocker resolved for this local setup by copying JBR cacerts to
C:/Users/Edward/AppData/Local/Pebo/Android-PC02-20261016/build-trust/cacerts,
adding ONLY existing Windows-trusted AVG inspection PUBLIC root. JAVA_TOOL_OPTIONS
points to this copy in subprocesses; system/JBR original trust stores unchanged,
TLS validation enabled. Studio JBR25.0.3 / existing Wrapper9.3.1 and SDK unchanged.
Rebuild from local PeboAndroid with process-only JAVA_HOME=Studio jbr,
GRADLE_USER_HOME=parent/gradle-cache, JAVA_TOOL_OPTIONS=
-Djavax.net.ssl.trustStore=C:/Users/Edward/AppData/Local/Pebo/Android-PC02-20261016/build-trust/cacerts.
Do not copy trust/SDK/cache/private key as another machine's readiness.

Exactly one authorized Sony XQ-DQ72 observed. Installed App public certificate:
e559eb3dd75ea241b28726488f7069c3abc3f93169eedba7c219f32007da0bb5;
new local APK:ff119e3ef72af127c3ca71da7c2cbf7023f0c75a44647b03e40a598d878ede4a.
Mismatch: no install/uninstall/data clear/private-key transfer attempted. Phone
still runs original crashing v3; source fix is NOT a physical UI PASS. Installed
base.apk pulled only locally to compare public signer. Instrumentation not run:
no configured runner/dependencies; existing Medium_Phone_API_37.0 AVD acceleration
check reports missing hypervisor driver. No driver installed/emulator launched.
No new NFC commands, MCU/power/instrument work. Final process observation:
adb PID24208 available, no java/studio64; single-use build daemon exited.
Final checks: current25-file manifest0 mismatches,20 numbered Minimal sections,
Full Draft/decision hashes unchanged, git diff --check reports no whitespace errors.

Added docs/pebo-nfc-protocol-v0.2-minimal-draft.md and minimal-decisions.md:
20 required sections, separate from untouched Full Draft/v0.1. New wire rules
PROPOSED only; no v0.2 implementation. One-entry cache12B has CRC collision limits;
recommend18B plus existing-config equality; no cross-reset exactly-once promise.
History8B plus epoch metadata/live cursor; fixed224B config/352B total EEPROM;
core976..1984B code and304B static RAM with exact comparison, total support
2192..4288B code/448B static plus128..256 stack, all estimates not MCU builds.
History raw64/128/256=512/1024/2048B; conservative physical1536/2688/4992B.
Conditional Flash log recommendation, not hardware acceptance. RM0377 PDF not
directly verified due size/download restrictions; precise driver/power conditions
remain OPEN. Datasheet/official HAL directly checked for limits/page/word granularity.

Changed this session: Activity, new StatusText/test, Android README, new Minimal
draft/decisions, generated evidence/current manifest, handoff/registry/HIL notes.
All saved locally, uncommitted/unpushed. Next: agree signing-compatible test path
without erasing phone data, UI recheck; review Minimal P0 especially counter/epoch,
retry/reset guarantees, Flash packing/link/power, target/LED and production write
protection. Keep Full Draft history; never mix two different layouts as profile02.

## 2026-10-25: v0.2 document progress audit

Compared attached original request with docs/pebo-nfc-protocol-v0.2-draft.md
and pebo-nfc-protocol-v0.2-decisions.md. Both exist (filesystem timestamps
2026-10-16, not proof of authorship/commit). Draft has requested19 sections plus
resource estimate, v0.1 comparison, validation plan and evidence boundaries.
Decision table includes P0/P1/P2 and review record; all review decisions remain
OPEN/unassigned. Status: draft authored, awaiting review; not approved/implemented.
Highest implementation gates: NVM estimate1152B exceeds1KB (RAM-only alternative
requires explicit product acceptance); atomic/RTC recovery and retry guarantees;
authorization/key lifecycle plus real FTM wake/power validation. Flash/RAM are
estimates, not verified product builds. No new PDF/source correctness validation
or v0.2 test execution in this progress-only audit.
Earlier physical Status formatter crash is subsequent evidence not reflected in
draft section21; consult preceding UI/NFC handoff. No code/spec design changed.
Next: record accept/reject/alternative and owner for P0 review groups, choose
history persistence profile, finalize resource/wire/auth decisions before coding.
TripleC/main11e2f6e; prior dirty work preserved. Only handoff/registry updated,
saved locally, no commit/push/fetch. Phone/process/instrument state not requeried;
prior ADB observation retained as history, no device/instrument actions this audit.

## 2026-10-25: PC-02 physical UI/NFC test - PARTIAL, Status crash found

TripleC/main11e2f6e; authorized Sony XQ-DQ72/Android15, installed v3 unchanged.
MainActivity cold launch PASS; Sample labels and GET_DEVICE_INFO display PASS.
Read Status reproduced fatal UnknownFormatConversionException twice, at
ProtocolActivity.kt:188/showBody. Interpolated battery percentage leaves a literal
percent before newline in a string later passed to format(flags), causing the
formatter exception. First unexpected return to main was explained by crash log.
No source fix/build/install performed in this test-only session; Gradle PKIX and
cross-machine debug-signature compatibility remain prerequisites for device fix.

User retapped Pebo after Prepare block0: physical NfcV read PASS at01:04:48+0800.
UID Android raw order:30 13 40 7F 66 52 02 E0; TX22 20 30 13 40 7F 66 52 02 E0 00;
full RX00 00 00 00 00, validated4 data bytes00 00 00 00. Screenshot subsequently
showed discovery count2 but only one success log; no second prepared read claimed.
Board variant/power/antenna geometry not independently verified. This is engineering
block-read evidence, not fresh MCU status or real application protocol compatibility.
Evidence: Software/PeboAndroid/handoff/evidence/pc02-20261025-ui-nfc/
nfc-block0.xml/png, status-crash.log, status-crash-dialog.xml/status-crash.png.
NFC screenshot visually inspected; UI/XML and crash snapshots taken separately.

Full Config/History/SyncTime/invalidHEX/real-mode smoke, five repeated prepared
reads, transfer interruption, invalid block input, lifecycle and Copy/Clear remain
pending. No real NFC writes, firmware/power/instrument control or App data clear.
Next: repair Status formatter with regression, resolve trusted Gradle download,
build/test/signature-compatible update, repeat UI smoke and remaining RF checks.
Handoff/machine/HIL notes and evidence saved locally, uncommitted/unpushed;
existing work preserved, no fetch/cloud verification. ADB left available.

## 2026-10-25: PC-02 phone development connection verified

TripleC/main11e2f6e8954aa45e373c615d853d323aac053979; existing work preserved.
Read current handoff/machine profile; adb1.0.41/package37.0.1 and JBR25.0.3
rechecked. Exactly one authorized phone: Sony XQ-DQ72, Android15/API35;
adb device state=device and adb_enabled=1. Read-only shell/package queries PASS.
Pebo com.pebo.nfcprobe20261015 installed for user0, versionCode3,
versionName0.1-protocol-sample, DEBUGGABLE, minSdk23/targetSdk37;
MainActivity resolves and NFC permission granted. NFC hardware present/state on.
Screen OFF_LOCKED at observation; foreground UI/RF tests not performed.
USB/ADB development connection is ready; current Gradle PKIX build blocker
remains unresolved (not rerun in this phone-only check). Installed signer versus
future local APK compatibility remains unverified. No install/update/uninstall,
clear-data, App launch, NFC transaction, firmware/power/instrument command.
adb PID24208 left available; no java/studio64 observed. Next: unlock phone for
Sample UI/RF testing; resolve trusted Gradle HTTPS and validate local build,
then compare signing certificates before any update. Handoff and registry saved
locally, uncommitted/unpushed; no fetch/cloud synchronization verification.

## Current PC-02 Android readiness: 2026-10-25

- Hostname TripleC, profile C:/Users/Edward; Git2.46.2.windows.1,
  main11e2f6e8954aa45e373c615d853d323aac053979. Existing modified/untracked
  work preserved; no fetch/commit/push or cloud synchronization verification.
- Read project/Android README, handoff/registry, protocol and v0.2 decision documents.
  All23 source-manifest SHA256/byte lengths match; transfer ZIP hash matches
  7698C1CBF57231A3709E781B478339EA84617207E50DE80DB29FA200A3AC6C4B.
  Existing local ASCII copy also matches manifest; application source unchanged.
- Studio JBR25.0.3, SDK android-37.0, BuildTools36.0.0, adb1.0.41/package37.0.1
  verified. PATH Java resolves to Oracle Java8; use explicit Studio JBR for builds.
- Local development/build copy:
  C:/Users/Edward/AppData/Local/Pebo/Android-PC02-20261016/PeboAndroid;
  sibling gradle-cache and machine-local local.properties retained. Shared
  Software/PeboAndroid remains saved source; reconcile edits before builds/handoff.
- Retried testDebugUnitTest assembleDebug --no-daemon --console=plain using
  process-only JAVA_HOME/GRADLE_USER_HOME. BLOCKED at Gradle9.3.1 HTTPS download:
  SSLHandshakeException / PKIX path building failed. Tests did not execute and no
  new APK produced. Log: local-copy parent/build-20261025.log. No TLS bypass,
  global Java/environment/certificate/license changes. Prior PC-01 PASS is historical.
- v0.2 draft/decision table explicitly unadopted, unimplemented and awaiting review;
  current App remains version3/v0.1 Sample with real packet transport unbound.
- adb devices -l: no devices; server started and left available(PID24208 observed).
  No java/studio64 observed afterward. No emulator/install/MCU/power/instrument
  operations. UI/RF/HIL skipped. Default shell sandbox still fails initialization;
  approved outside-sandbox inspection/build commands worked.
- Next: resolve trusted JBR/Gradle HTTPS access, rerun58 tests/debug build, then
  connect/authorize phone and check signing compatibility before update/UI/RF checks.
  Never uninstall/clear data to fix signing mismatch. Review v0.2 before implementation.
- Only handoff/registry updated plus local build failure log/cache activity;
  handoff saved locally, uncommitted/unpushed. Previous dated observations retained.

## Current Android source handoff — 2026-10-15

- Android source is now saved in `Software/PeboAndroid` in this workspace.
  The old TEMP path is historical, not required for the next computer.
- Start with `Software/PeboAndroid/README.md` and `PROTOCOL-v0.1.md`.
  Latest source transfer/verification details are in the final dated section.
- Complete portable archive: `Software/PeboAndroid-handoff-20261015.zip`;
  verify its adjacent `.sha256` after cloud/file transfer.
- No commit/push or remote/cloud replication is implied by these saved files.
  Preserve all existing local changes and verify arrival on the next computer.


Last updated: 2026-10-25 (Asia/Taipei).

This file is the cross-computer handoff note for Pebo development. Update it
whenever a development machine, hardware setup, measurement result, or confirmed
decision changes.

## Current repo location

- Active workspace verified in this session: `G:\Google Drive\Application\Project\Pebo`
- Some other computers or Google Drive language settings may show the same
  synced folder as `G:\我的雲端硬碟\Application\Project\Pebo`. Verify the actual
  path on each computer before running build, flash, or instrument commands.

## Required first reads

Before changing firmware, hardware assumptions, or tests, read:

1. `AGENTS.md`
2. `README.md`
3. `docs/hardware.md`
4. `docs/hardware_pinmap.md`
5. `docs/firmware-architecture.md`
6. `docs/nfc-protocol.md`
7. `docs/motion-detection.md`
8. `docs/power-budget.md`
9. `docs/hil-test-plan.md`

## Confirmed hardware and bring-up status

Source of truth: `docs/hardware_pinmap.md`, Pebo v0.1.

Confirmed so far:

- MCU is STM32L031G6U6TR, UFQFPN28.
- Firmware environment is Arduino IDE / STM32duino.
- ST-LINK / SWD programming works at 4 MHz.
- ST-LINK firmware was updated and the earlier USB reconnect issue was resolved.
- LED GPIO bring-up passed.
- UART debug on PA9 / PA10 works at 115200 baud with `Firmware/HardwareTest`.
- I2C on PB6 / PB7 works at 100 kHz.
- LIS2DW12 ACK is at `0x19`, and `WHO_AM_I` reads `0x44`.
- LIS2DW12 raw X/Y/Z output is non-zero after ODR initialization.
- ST25DV user/system I2C ACK works at `0x53` / `0x57`.
- ST25DV user memory read from address `0x0000` succeeded.
- Best measured whole-board deep-sleep baseline is 6.84 uA average at 2.8 V
  with ST-LINK disconnected and ST25_LPD actively driven High.

## Critical low-power rule

On Pebo v0.1, PA4 / ST25_LPD must remain configured as GPIO Output High during
deep sleep while ST25DV is powered.

Do not include PA4 in generic Analog/No-Pull GPIO shutdown code. Any low-power
helper that reconfigures GPIOA must explicitly exclude PA4 or restore PA4 High
before entering STOP.

Measured effect at 2.8 V:

- ST25_LPD not actively held High: about 56.68 uA average, about 130 uA peak
  every 40 ms.
- ST25_LPD actively driven High: about 6.84 uA average, about 16 uA peak every
  40 ms.

## Firmware areas

- `Firmware/HardwareTest`: current UART-controlled PCBA bring-up sketch.
- `FW/`: official product firmware area and focused experiment sketches.
- `FW/libraries/PeboLIS2DW12`: minimal LIS2DW12 I2C driver.
- `FW/libraries/PeboST25DV`: minimal ST25DV I2C driver.
- `FW/StopBaseline_LPDHigh`: current lowest-current STOP baseline preserving
  PA4 / ST25_LPD High.

Do not treat `Firmware/HardwareTest` as product firmware. It is bring-up and
diagnostics firmware.

## Software tools

- `Software/hardware_test_gui.py`: Tkinter GUI for HardwareTest UART commands.
- `Software/hardware_test_probe.py`: command-line UART probe.
- `Software/PeboBridge`: read-only MCP bridge source for project inspection.

Known issue:

- `Software/PeboBridge/README.md` records that installing the Python MCP SDK
  previously failed because of local TLS/transfer issues. Verify on the current
  computer before assuming the bridge can be launched.

## Build and upload notes

The HardwareTest README records this Arduino CLI path from a development PC:

```powershell
G:\Google Drive\Application\Arduino-cli\arduino-cli.exe
```

On this computer, verify the actual Arduino CLI path before building or
flashing. Use the `Firmware/HardwareTest/README.md` commands as the starting
point, and keep LTO enabled because the STM32L031G6Ux has only 32 KB flash.

Do not flash or run hardware tests until the connected board, ST-LINK, UART,
supply voltage, and current-measurement setup are verified.

## Next work queue

Hardware and measurement:

- Identify unexpected I2C address `0x2D` seen during HardwareTest scan.
- Verify LIS2DW12 INT1 / INT2 behavior.
- Identify the source of the remaining about 40 ms / 16 uA sleep peak.
- Test LSE / RTC.
- Test piezo drive and supply droop.
- Verify STOP-mode wake-up behavior.
- Run PPK2 subsystem-isolation profiling with R15 / R16 / R17.
- Measure I2C rise time and decide whether 10 kOhm pull-ups should change to
  22 kOhm.

Firmware and protocol:

- Define product firmware structure under `FW/`.
- Keep NFC data format versioned before implementation: byte order, offsets,
  bounds, validity, checksum/CRC if used, commit behavior, and stale-command
  handling.
- Avoid claiming phone reads are fresh MCU data until the ST25DV update path is
  measured.
- Keep UART logs parseable in development builds and off or minimized in
  battery-life builds.

Validation:

- Run software-only tests when changing Python tooling.
- Run relevant Arduino build checks when changing firmware.
- Mark HIL checks skipped unless actual connected instruments were used.

## Handoff checklist for the next computer

1. Confirm the workspace path and update this file if it changed.
2. Run `rg --files` to confirm the expected project layout is present.
3. Check git state before editing, and do not overwrite local uncommitted work.
4. Re-read `AGENTS.md` plus the relevant document under `docs/`.
5. Verify Arduino CLI path, STM32duino core, ST-LINK, UART port, and instrument
   connections before any flash or measurement command.
6. If making firmware changes, preserve the PA4 / ST25_LPD High deep-sleep
   rule.
7. After confirmed measurements or decisions, update the relevant document and
   this handoff note.

## 2026-10-13: Battery voltage accuracy test / PC automation

Repository remote checked: https://github.com/TripleC-Light/Pebo.git.
Added standalone `FW/BatteryVoltageTest` and `Software/PPK2Automation`.
The user identifies LowestCurrentStopBaseline, LIS2WakeupStopTest,
RTCStopWakeTest and ST25GpoWakeTest as passed bring-up firmware.
All four directories were verified unchanged; none was flashed.
The older next-work list above must be read alongside this newer status.

Confirmed implementation/source details (not physical measurement):
- STM32duino 3.0.0 on this PC; its concrete serial class is `Uart`,
  as already used by HardwareTest.
- ADC channel `ADC_CHANNEL_VREFINT` (17), 12-bit factory calibration
  `*VREFINT_CAL_ADDR`, `VREFINT_CAL_VREF=3000` mV, and
  `LL_ADC_DELAY_VREFINT_STAB_US=10` verified in installed ST headers.
- One discarded conversion plus 32 averaged conversions at boot; no battery
  percentage. ADC/internal buffer/clock and UART shut down before STOP.
  PA4/ST25_LPD stays Output High.
- Python 3.9.13; pyserial 3.5; ppk2-api 0.9.2. Package API inspected directly.
  Temporary inspection dependencies are in ignored
  `Software/PPK2Automation/.deps`; install requirements in .venv on a new PC.
- Arduino CLI nightly-20260521 (commit dd407d4), located at
  `G:\我的雲端硬碟\Application\Arduino-cli\arduino-cli.exe`.
- 12 hardware-free automation tests passed; Python syntax, real imports and
  CLI help passed. Tests cover full voltage list, timeout continuation,
  fragmented UART/noise, +/-50 mV boundary, CSV preservation, serial errors,
  Ctrl+C, OFF retry/failure and initialization cleanup.

Build status: PASS (Debug ON and OFF, after explicit user approval to resume).
STM32duino 3.0.0, STM32L031G6Ux, LTO, ARM GCC 14.2.1-1.1:
- Debug ON: 13856 / 32768 bytes flash; 1780 / 8192 bytes RAM.
- Debug OFF: 13692 / 32768 bytes flash; 1780 / 8192 bytes RAM.
- ELF strings: BATTERY_MV present in both; VREF_RAW, VREF_CAL and
  SAMPLE_COUNT present only in Debug ON.
- ON ELF SHA256: BE53D80792C206B6F6928D23A97CA69598AD9EADE0FC23A5F5DFBDFB8C552D3A
- OFF ELF SHA256: 2CE5E7D2C10E1F5A1923F4D50667B1A6775936E06EE40066C14ED758213EDB94

Build fixes confirmed on this computer:
- The linker could not write its ELF under the Chinese Google Drive path.
  Use an ASCII-only build output path; successful outputs are in
  C:\Users\echou\AppData\Local\Temp\Pebo-BatteryVoltageTest-debug-on
  and the corresponding -debug-off directory. Temporary artifacts are not
  portable handoff files; rebuild using the README command on each PC.
- Removed duplicate sketch ADC MSP hooks and reused STM32duino's existing
  hooks. The core's default selected ADC pin is NC; pinmap_pinout(NC)
  returns without GPIO changes. This sketch never calls analogRead.
At the build-only stage, no flash/upload or instrument control was attempted.
The later flash result is recorded below.

HIL status at the build-only stage: SKIPPED. No PPK2 supply or DUT was exercised; ports, wiring,
2.6..3.0 V boot/accuracy, factory calibration readings, actual rail voltage,
ADC shutdown and STOP current remain unverified for this new sketch.
Next: verify PPK2/UART wiring and discharge/back-power behavior,
flash only BatteryVoltageTest, run the sweep, retain CSV and independently
measure VDDA plus post-measurement current. Record firmware build/commit,
board revision, instrument versions and actual COM ports.
OFF is attempted in finally and retried; lack of PPK2 output acknowledgement
means unplugged/failed hardware cannot be physically guaranteed OFF.

## 2026-10-13: BatteryVoltageTest flashed / initial boot capture

User confirmed CR2032 removed, PPK2 is the sole supply, UART connected only
adapter RX to PA9 plus GND, and Power Profiler/serial terminals closed.
User explicitly authorized flashing BatteryVoltageTest.

Hardware discovery:
- PPK2 control: COM10, metadata HW=1518; COM12 is the other USB interface
  and returned no valid PPK2 metadata.
- UART: Prolific USB-to-Serial COM14, 115200 baud.
- ST-LINK SN: 56FF6C066688555210351387; firmware V2J39S7.
- STM32CubeProgrammer v2.11.0 detected device ID 0x425, Rev X,
  STM32L03x/L04x/L010, 32 KB flash.
- Debug ON ELF SHA256 matches the build record above.

Flash result: PASS. CubeProgrammer connected SWD at 4000 KHz under hardware
reset, downloaded BatteryVoltageTest.ino.bin to 0x08000000, verified the
download successfully and started the program (exit code 0).
Only the flashed image changed; the four protected bring-up source directories
were not modified. No option bytes or separate mass-erase command were used.

Initial UART capture: BATTERY_MV=2768 with PPK2 setpoint 2800 mV.
Setpoint difference is -32 mV (-1.14%). ST-LINK was still attached during
this capture; its reported target voltage was 2.71 V and is not an independent
precision VDDA measurement. This is evidence of one successful flash/boot/
measurement/UART path, not acceptance of the five-point accuracy sweep.
PPK2 OFF was sent from finally after flashing/capture and the port closed;
physical OFF was not independently measured.

Current HIL status: PARTIAL. Full 3.0..2.6 V sweep, independent rail accuracy,
power-off discharge/back-power check, PA4 physical level and STOP current
remain pending. User has been notified to disconnect ST-LINK board-side
connections and retain PPK2 + UART before the sweep.

## 2026-10-13: Five-point sweep and post-measurement current capture

ST-LINK board-side connections were disconnected by the user before these
runs. PPK2 control COM10 / HW=1518, UART COM14 / 115200, pyserial 3.5,
ppk2-api 0.9.2, Debug ON image SHA256 as recorded above. Cell removed;
adapter connected only RX-to-PA9 and GND; other desktop port users closed.

First run with default OFF wait 1 s: 3000 -> 2991 mV PASS; the following
2900/2800/2700/2600 points timed out. RESULT FAIL, 1 PASS / 4 FAIL.
Artifact: Software/PPK2Automation/results/battery_voltage_20261013_160717_365815.csv.
Timeout continuation and FAIL reporting were exercised on real hardware.

Diagnostic run, OFF wait 15 s: 2800 -> 2799 mV PASS.
Artifact: Software/PPK2Automation/results/battery_voltage_20261013_160837_264928.csv.

Full rerun with OFF wait 15 s, boot wait 1.5 s, UART timeout 5 s,
inclusive +/-50 mV threshold: RESULT PASS, 5 PASS / 0 FAIL.

| Source setpoint (mV) | MCU (mV) | Error (mV) | Error (%) | Result |
| ---: | ---: | ---: | ---: | --- |
| 3000 | 2991 | -9 | -0.300000 | PASS |
| 2900 | 2904 | +4 | +0.137931 | PASS |
| 2800 | 2799 | -1 | -0.035714 | PASS |
| 2700 | 2713 | +13 | +0.481481 | PASS |
| 2600 | 2626 | +26 | +1.000000 | PASS |

Artifact: Software/PPK2Automation/results/battery_voltage_20261013_160939_341251.csv.
Use --off-wait 15 --timeout 5 for this observed bench profile.
The CLI default remains 1 s as originally requested; it did not produce
reliable repeated boots here. Insufficient discharge is a hypothesis,
supported by longer waits restoring repeated boot; VDD decay timing and
the exact reset threshold were not directly measured.

Additional 2800 mV boot: BOOT BatteryVoltageTest, VREF_RAW=1790,
VREF_CAL=1661, BATTERY_MV=2784, SAMPLE_COUNT=32.
The raw/calibration values agree with the implemented factory-cal formula.
The extra boot reading differs from the sweep by -15 mV; both meet the
setpoint comparison limit. No battery percentage estimate was produced.

PPK2 post-measurement capture began after a 2 s boot/UART window and
lasted approximately 5 s: 496128 decoded samples; mean 3.161851 uA,
median 3.155366 uA, minimum 2.451175 uA, maximum 5.576022 uA.
PPK2 metadata Calibrated=0; the meaning/validity of that flag and absolute
current calibration have not been independently verified. These readings
are evidence of low consumption after the ADC/UART window, not a certified
current measurement or a like-for-like improvement over the old 6.84 uA baseline.
JSON includes the full PPK2 metadata and UART lines; raw serial bytes are
stored alongside it for decoding with the inspected 0.9.2 API:
Software/PPK2Automation/results/battery_post_measurement_current_20261013_161228_072850.json
Software/PPK2Automation/results/battery_post_measurement_current_20261013_161228_072850.bin

All sessions sent PPK2 OFF in finally and closed the ports. The user reports
having a meter/scope; an OFF-state VDD reading has been requested and is
pending. Independent rail voltage, PA4 physical voltage, VDD decay waveform,
current calibration, Debug OFF physical execution and forced interruption
while powered remain unverified. Exception/Ctrl+C OFF paths have software
regression coverage, not a deliberately interrupted powered HIL test.
Results are compared with the source setpoint, not an independently measured
VDDA; do not claim absolute +/-50 mV accuracy or six-month battery life.

## 2026-10-13: Manual rail checks / PPK2 metadata cleanup

User-reported DMM reading with output OFF: VDD=0.27 V.
This is an OFF-state observation after a long wait, not a timed 1 s / 15 s
discharge waveform. It does not prove VDD reaches zero or identify the
earlier timeout cause.

A 90 s bounded 2800 mV manual measurement window booted and printed
BATTERY_MV=2787; finally sent OFF at its deadline.
User reported VDD=2.8 V and PA4/ST25_LPD=0.48 V.
VDD agrees with the requested setpoint at the reported meter resolution.
LPD=0.48 V conflicts with the required Output High level if measured while
VDD was 2.8 V. Timing relative to the automatic OFF deadline is uncertain,
so physical PA4 acceptance is NOT confirmed. A second 180 s bounded 2.8 V
window is being used to remeasure PA4 first, then VDD with the same ground;
its startup printed BATTERY_MV=2788. Record the user's follow-up below.

After current capture, subsequent get_modifiers() calls encountered
UnicodeDecodeError from residual binary sample data. Neither failed
metadata attempt enabled supply; finally sent OFF. Stopping measurement,
draining the serial data for 2 s, then resetting the input buffer restored
metadata access (HW=1518). The sweep now performs this cleanup before
metadata initialization. Added a regression for a previous active stream/
buffered current packets: 13 tests and Python syntax validation PASS.
The two later manual windows used this confirmed initialization sequence.

Pending: independent current calibration, timed VDD discharge, physical
PA4 confirmation, and Debug OFF physical execution. Do not mark the
low-power pin requirement passed based only on current or source code.

### Manual follow-up: controlled powered readings

During the second bounded 2.8 V window, the user measured PA4/ST25_LPD first
and then VDD, both against the same board GND: both read 2.805 V on the DMM.
The startup UART reading for this window was 2788 mV, -17 mV (-0.61%)
relative to the user's 2805 mV rail reading. Meter calibration and exact
temporal alignment to the boot sample are not independently characterized.

PA4 physical High is confirmed for this post-measurement powered state.
The earlier 0.48 V LPD reading was not reproduced in the controlled ON
window; measurement timing relative to the previous automatic OFF remains
the suspected explanation, not a proven cause. No further firmware change
was needed for this recheck.

Current qualification: five-point source-setpoint sweep PASS with 15 s OFF,
factory-calibration UART data observed, initial low-current evidence captured,
one independently observed powered rail point within 50 mV, and PA4 High
confirmed by the user. Five-point independent absolute rail accuracy,
timed discharge waveform, current calibration, Debug OFF physical execution
and deliberately forced powered Ctrl+C remain pending. No battery-life claim.

### Final regression and instrument state

The updated sweep script (including previous-stream cleanup) was exercised
on real COM10/COM14 at 2800 mV with --off-wait 15 --timeout 5:
UART 2798 mV, error -2 mV (-0.07%), RESULT PASS, 1 PASS / 0 FAIL.
CSV: Software/PPK2Automation/results/battery_voltage_20261013_162832_784414.csv.
Both bounded manual windows ended normally and sent OFF. The final script
also sent OFF from finally and closed its ports. No instrument session is
left running. User-reported physical OFF VDD was 0.27 V in the earlier
long-OFF reading; it was not remeasured after this final run.

Final software checks: 13 automation regressions PASS; Python syntax PASS.
Firmware unchanged since the verified Debug ON flash; all four protected
bring-up source directories remain unchanged. Successful full sweep and
initial failed sweep artifacts are both retained. No Git commit/push was
performed; keep the working-tree changes when handing off to the next PC.

## 2026-10-14: New development PC / CubeProgrammer retry

- Workspace: G:/我的雲端硬碟/Application/Project/Pebo; Windows user tripl.
- STM32CubeProgrammer version query PASS: v2.11.0, executable at
  C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe.
  No target connection, flash, reset, power or instrument command was issued.
- Default sandbox command launch still fails with
  helper_unknown_error: setup refresh had errors. Outside-sandbox commands work.
  Missing CubeProgrammer was not the cause of the process-launch failure.
- Arduino CLI exists both in Arduino IDE resources and at
  G:/我的雲端硬碟/Application/Arduino-cli/arduino-cli.exe.
  Current default Arduino package directory contains AVR, SAM and ESP32;
  STM32duino is not installed for this Windows user.
- Attempted STM32duino 3.0.0 installation. Official index downloaded, but
  GCC 14.2.1-1.1 (308.39 MiB) downloaded extremely slowly; installation was
  interrupted before completion. No download or Git process was left running.
- Git fetch of codex/led-brightness-test did not complete and was interrupted.
  Local branch is main. Existing modified README.md, docs/hil-test-plan.md,
  docs/power-budget.md and untracked BatteryVoltageTest/PPK2Automation/handoff
  were preserved. BatteryVoltageTest exists locally but is absent from remote main.
- LED implementation is in remote branch codex/led-brightness-test, draft PR
  https://github.com/TripleC-Light/Pebo/pull/1; not yet synchronized locally.
  Implementation commit b40ae97e792f6d3f4492751225796445414f5a7d passed core 3.0.0 CI:
  Debug ON Flash 18700 bytes / static RAM 1920 bytes;
  Debug OFF Flash 12264 bytes / static RAM 1044 bytes.
  Local LED build and physical LED/STOP tests remain pending.
- Next: finish installing STM32duino 3.0.0, fetch LED branch while preserving
  local changes, build both debug modes with an ASCII build-output directory,
  then rediscover and verify current board/supply/ST-LINK/UART before flashing.

## 2026-10-14: Three-computer development registry

- Added docs/development-machines.md as the per-computer environment registry.
- PC-01 is verified as TRIPLEC_AERO, user tripl, Windows 10.0.26200.0.
  User names PC-02 TripleC and PC-03 Edward-PC, and confirms historical
  user echou belongs to Edward-PC. Purpose labels are omitted as requested.
  Edward-PC's 2026-10-13 build/HIL observations are linked as historical
  evidence; TripleC's setup has not been checked.
- Recorded current CLI/core/CubeProgrammer state, local HEAD, preserved
  uncommitted work, incomplete STM32 setup and pending LED synchronization.
- Updated AGENTS.md to require startup checks and an end-of-session handoff
  plus current machine profile update. README links to both records.
- Canonical local task log is docs/handoff.md. Reconcile the LED remote
  branch's docs/development-handoff.md when integrating; retain its CI history.
- Documentation-only session: no firmware, build or instrument changes.
  Verification: reread saved files and inspect Git diff; no code tests needed.
- Changes saved locally; not committed/pushed. Other computers need these
  documents transferred before the rules are available in their checkout.
- Next: verify TripleC/Edward-PC's current tools/Git/instruments on arrival,
  and complete TRIPLEC_AERO's STM32duino 3.0.0 setup.

## 2026-10-14: TRIPLEC_AERO software-only readiness PASS

User installed STM32duino 3.0.0 and reports no connected hardware.
Core list verifies 3.0.0; GCC 14.2.1-1.1 and CubeProgrammer 2.11.0 execute.
Shared Arduino CLI nightly-20260521 and IDE bundled CLI 1.4.1 execute.
Git 2.53.0.windows.3 local commands work outside the Codex sandbox.
Remote synchronization remains separate and unverified.

Actual local builds (Generic L031G6Ux, core 3.0.0, LTO):
- BatteryVoltageTest Debug ON: PASS, Flash 13856 bytes, static RAM 1780 bytes.
- BatteryVoltageTest Debug OFF: PASS, Flash 13692 bytes, static RAM 1780 bytes.
- HardwareTest: PASS, Flash 32148 bytes, static RAM 2180 bytes.
Build output uses ASCII directories; exact paths/FQBN in development-machines.md.
Firmware source and protected bring-up sketches were not edited.

Created machine-local Python environments instead of sharing portable venvs:
- Python 3.9.10: installed pyserial 3.5 / ppk2-api 0.9.2, pip check PASS,
  PPK2Automation 13 mock tests PASS and CLI --help PASS.
- Python 3.10.11: installed mcp 1.30.0, pip check PASS; bridge 6 tests PASS,
  real project status/README read PASS, MCP stdio handshake/list_tools/tool
  call PASS. All child processes closed after verification.
- Five Python tool files parse successfully; Tcl 8.6.12 loads.

Fixed Software/PeboBridge/requirements.txt to mcp>=1,<2: unbounded mcp
installed 2.3.0 and failed to import FastMCP. SDK 1.30.0 works with existing
server code. README now requires Python 3.10+ and bounded requirements.
Store Python redirects its venv into its package LocalCache; use the exact
verified executable path in development-machines.md.

Hardware/HIL, GUI application operation and physical USB/COM drivers SKIPPED.
CubeProgrammer PASS means version/startup only, not programming capability.
No hardware connection, flash, target reset or supply command was issued.
No install/build/server session is left running. Documents and dependency
fix saved locally; not committed/pushed. Previous local work preserved.
Next: synchronize/build LedBrightnessTest, then verify instruments when
connected; keep historical hardware measurements separate from this PC's tests.

## 2026-10-14: MCP Bridge excluded from routine work

User requests ignoring MCP Bridge going forward. Do not include its setup,
installation, launch or tests in routine development/readiness checks unless
explicitly requested. Existing source and historical records remain intact.
Preference saved in AGENTS.md and development-machines.md as well as this log.
Documentation only; no firmware, dependency or instrument changes.
Saved locally, not committed/pushed.

## 2026-10-14: TripleC handoff document check

Current computer identified as TRIPLEC, user context triplec/codexsandboxoffline.
Workspace verified as G:/Google Drive/Application/Project/Pebo. Local HEAD is
11e2f6e8954aa45e373c615d853d323aac053979 on main tracking origin/main.

Git working tree is not clean and must be preserved before any pull, branch
switch, merge or cleanup. Modified files: AGENTS.md, README.md,
Software/PeboBridge/README.md, Software/PeboBridge/requirements.txt,
docs/hil-test-plan.md and docs/power-budget.md. Untracked paths:
FW/BatteryVoltageTest/, Software/PPK2Automation/, docs/development-machines.md
and docs/handoff.md.

Tool checks on TripleC: Git 2.46.2.windows.1, shared Arduino CLI
nightly-20260521/dd407d4, STM32CubeProgrammer 2.11.0 version query PASS, and
STM32duino core STMicroelectronics:stm32 3.0.0 installed. Arduino core list
required outside-sandbox access because the default sandbox could not read
C:/Users/Edward/AppData/Local/Arduino15. Python on PATH resolves to
C:/Users/Edward/AppData/Local/Programs/Python/Python312/python.exe.

No firmware build, flash, target reset, power supply command or instrument
operation was issued during this handoff check. Hardware/HIL readiness on
TripleC remains unverified; rediscover ST-LINK, UART, PPK2, scope, board
wiring and power state before any physical operation. MCP Bridge remains
excluded from routine checks unless explicitly requested.

## 2026-10-14: LedBrightnessTest implementation / build

Added standalone FW/LedBrightnessTest/LedBrightnessTest.ino. The protected
bring-up firmware directories LowestCurrentStopBaseline, LIS2WakeupStopTest,
RTCStopWakeTest, ST25GpoWakeTest and BatteryVoltageTest were not modified.

PWM capability was checked against the installed STM32duino 3.0.0
Generic L031G6Ux variant. PA5 maps to TIM2_CH1, PA6 to TIM22_CH1, PA7 to
TIM22_CH2 and PA8 to TIM2_CH1. Because PA5 and PA8 share TIM2_CH1 and the
four LEDs cannot all use a single consistent hardware PWM arrangement, the
test uses software PWM with one HardwareTimer time base on TIM21.

LED test settings: PWM frequency 1000 Hz, 20-step shared time base,
DIM=25 percent duty and BRIGHT=100 percent duty. Debug ON initializes UART
PA9/PA10 at 115200 and logs each test state; Debug OFF builds without UART
initialization. Test sequence covers individual DIM/BRIGHT, ALL DIM/BRIGHT/OFF,
mixed levels and the progress display demo. At test end, all LEDs are OFF,
PA4/ST25_LPD is driven Output High and the MCU enters STOP.

Builds on TripleC with STM32duino 3.0.0, Generic L031G6Ux, LTO, ARM GCC
14.2.1-1.1:
- Debug ON PASS: 15056 / 32768 bytes flash; 1900 / 8192 bytes static RAM.
- Debug OFF PASS: 12200 / 32768 bytes flash; 1212 / 8192 bytes static RAM.

No flash/upload, target reset, power command, UART capture or visual LED test
was performed in this session. Physical acceptance remains pending: verify
flicker, DIM/BRIGHT distinction, mixed display, PA4 level and post-test STOP
behavior on the actual board after rediscovering hardware connections.

## 2026-10-14: LedBrightnessTest upload attempt blocked by target voltage

User reported hardware connected and requested upload test. Debug ON build
artifact existed at C:/Users/Edward/AppData/Local/Temp/Pebo-LedBrightnessTest-debug-on/LedBrightnessTest.ino.bin
(15276 bytes). COM port discovery found PPK2 CDC interfaces COM7/COM6 and
USB-UART COM8; PPK2 metadata check identified COM7 as the control port.

First Arduino CLI upload attempt via ST-LINK/SWD found ST-LINK
56FF6C066688555210351387 / V2J39S7, but target voltage was 0.00 V and no
STM32 target was found. PPK2 COM7 was set to source mode and commanded ON
at 2.8 V; a second upload attempt saw only 1.54 V and failed with
DEV_CONNECT_ERR. PPK2 was then commanded to 3.0 V; a CubeProgrammer
connection check after the control port closed again saw 0.00 V.

A short-lived background helper was started to hold PPK2 COM7 open and source
3.0 V during connection. While it was running, CubeProgrammer still reported
only 1.35 V and no STM32 target. No firmware was uploaded and no flash erase
or programming occurred. The helper exited, and a final PPK2 OFF command was
sent on COM7.

Current blocker: board power / ST-LINK VTref path is not healthy for flashing.
Before retrying, physically verify PPK2 VOUT to Pebo VCC_3V0, PPK2 GND to
Pebo GND, ST-LINK VTref/VCC_3V0 sense, common ground, polarity, any battery
or parallel supply state, and actual VDD with a meter. Do not retry flashing
until ST-LINK reports a normal target voltage around the intended rail.

Follow-up retry after user reported DMM and CubeProgrammer GUI both showed
2.77 V and closed nRF Connect / Power Profiler: command-line CubeProgrammer
still reported 1.54 V with DEV_CONNECT_ERR using SWD 4 MHz under hardware
reset. COM7, COM6 and COM8 could all be opened, so the serial ports were not
busy. Additional no-write connection checks still failed: HOTPLUG 4 MHz
reported 0.00 V, HOTPLUG 100 kHz reported 1.35 V, and UR 100 kHz reported
1.35 V. No flash/write was attempted. The discrepancy between GUI/user
measurement and CLI remains unresolved; verify the exact GUI probe/connection
settings and ST-LINK VTref pin before upload.

User then requested opening PPK2 supply for measurement. A background helper
using ppk2-api 0.9.2 called use_source_meter(), set_source_voltage(2800) and
toggle_DUT_power("ON") on COM7 for a 180 s window. User measured only 1.35 V.
The local API source confirms set_source_voltage() expects millivolts, so
2800 should mean 2.8 V; this is not a simple V-vs-mV argument error. User
reports that setting 2.8 V directly in the PPK2 / Power Profiler software
produces 2.77 V. Treat Python PPK2 source control on this computer as suspect
until revalidated against a meter. A PPK2 OFF command was sent after this
finding. For the next upload attempt, prefer user-controlled Power Profiler
source output and do not let Codex control PPK2 supply.

User then powered the board manually through Power Profiler / PPK2 GUI.
CubeProgrammer CLI saw target voltage 2.72 V, connected over SWD at 4 MHz
under hardware reset and identified STM32L03x/L04x/L010, device ID 0x425,
32 KB flash. Arduino CLI uploaded the Debug ON LedBrightnessTest build
successfully. During upload CubeProgrammer reported target voltage 2.71 V,
erased flash sectors [0 119], downloaded LedBrightnessTest.ino.bin
(14.92 KB) to 0x08000000 and started the application successfully.

Post-upload UART capture attempts on COM8 at 115200 captured no bytes: one
18 s capture after upload and one 20 s capture after ST-LINK reset. The reset
command itself succeeded and target voltage remained 2.71 V. Flash result is
PASS; UART/log verification and visual LED acceptance remain pending. Check
USB-UART RX-to-PA9, GND, selected COM port, and whether the sketch has already
entered STOP before relying on UART logs.

User visually confirmed the LED brightness test output is acceptable. Treat
DIM/BRIGHT visual distinction and the observed display behavior as accepted
for this bring-up test. UART log capture remains unverified; do not treat the
COM8 debug path as passed from this run.

UART recheck: COM8 is the Prolific USB-to-Serial adapter and can be opened at
115200. Holding COM8 open while triggering ST-LINK reset captured the complete
LedBrightnessTest log: BOOT LedBrightnessTest; PWM=SOFTWARE TIM21 1000Hz
DIM=25% BRIGHT=100%; LED1/2/3/4 DIM and BRIGHT; ALL DIM/BRIGHT/OFF; mixed
LED1=DIM LED2=DIM LED3=BRIGHT LED4=OFF; progress STEP 0..5; and
TEST DONE: ALL OFF, PA4 HIGH, ENTER STOP. ST-LINK reset reported 2.71 V and
identified STM32L03x/L04x/L010. COM8 UART debug path is now verified PASS for
this firmware.

PPK2 API source-control recheck after isolating the earlier 1.35 V symptom:
with DUT disconnected and only a meter on PPK2 VOUT/GND, ppk2-api 0.9.2
set_source_voltage(2800) produced 2.8 V. With the board reconnected, the same
API command produced 2.78 V at the board. With board plus ST-LINK connected,
the same command again produced 2.78 V. This clears the initial suspicion that
2800 was a V-vs-mV argument error or that the Python API always mis-programs
the source voltage on this setup.

An integrated retry kept the PPK2 COM7 API connection open, set source mode
to 2800 mV, then ran CubeProgrammer check and Arduino upload while powered by
the API. CubeProgrammer saw 2.73 V and connected to STM32L03x/L04x/L010.
Arduino upload then succeeded; a second byte-safe repeat also succeeded,
erasing sectors [0 119], downloading LedBrightnessTest.ino.bin (14.92 KB)
to 0x08000000 and starting the application. PPK2 OFF was sent at the end.
Conclusion: PPK2 Python API source control is working in the reproduced test.
The earlier 1.35 V / no-target state was transient or setup-state dependent,
not reproduced after rechecking connections and COM ownership.

## 2026-10-14: Piezo + LIS2 interaction / PPK2 current automation

Current computer: TRIPLEC, user context triplec/codexsandboxoffline.
Workspace remains `G:\Google Drive\Application\Project\Pebo`, local HEAD
11e2f6e8954aa45e373c615d853d323aac053979 on main tracking origin/main.
Existing local modified/untracked work was preserved. No commit or push was
performed.

Added:
- `FW/PiezoLIS2InteractionTest/PiezoLIS2InteractionTest.ino`
- `Software/PPK2Automation/piezo_current_test.py`
- `Software/PPK2Automation/tests/test_piezo_current_test.py`

Updated:
- `Software/PPK2Automation/README.md`
- `docs/hil-test-plan.md`
- `docs/power-budget.md`
- `docs/handoff.md`
- `docs/development-machines.md`

Protected passed firmware directories were not edited:
`FW/LowestCurrentStopBaseline`, `FW/LIS2WakeupStopTest`,
`FW/RTCStopWakeTest`, `FW/ST25GpoWakeTest`, `FW/BatteryVoltageTest` and
`FW/LedBrightnessTest`.

Firmware details:
- Piezo drive is single-ended only. PA0 drives Piezo A, PA1 stays LOW.
- Waveform generation uses STM32duino `HardwareTimer` on TIM2 channel 1,
  matching the installed STM32L031G6Ux variant mapping for PA0.
- Frequencies are compile-time constants: 2000, 3000 and 4000 Hz.
  Each runs 3 repeats, 500 ms beep, 1500 ms gap.
- LIS2DW12 setup follows the passed LIS2 wake test: 12.5 Hz low-power,
  wake-up detect, latched active-high INT1 routed to PB0.
- Before each beep, old LIS2 sources are cleared. During the beep and the
  300 ms post-beep observation window, firmware checks PB0 ISR flag, INT1
  level, WAKE_UP_SRC and ALL_INT_SRC. Any new trigger is recorded as a false
  trigger; thresholds are not auto-raised.
- UART emits fixed markers: PIEZO_EVENT_BEGIN, PIEZO_EVENT_END and
  PIEZO_TEST_SUMMARY. At completion, piezo is off, LIS2 source is cleared,
  UART/I2C are ended, PA4/ST25_LPD is Output High and firmware enters STOP.

Automation details:
- PPK2 flow: OFF, wait `--off-wait` default 15 s, set voltage default 2.8 V,
  clear old stream/UART data, `start_measuring()`, DUT ON, collect all 9
  markers, stop acquisition, DUT OFF in finally.
- ppk2-api 0.9.2 API used: `start_measuring()`, `get_data()`,
  `get_samples()`, `stop_measuring()`, source-meter mode and DUT power
  commands through the existing CheckedPPK2 pattern.
- The inspected API has no per-sample timestamp or simultaneous DUT voltage
  waveform in this path. The script timestamps decoded current samples using
  PC monotonic receive time and `--sample-rate-hz` default 100000; UART
  markers use the same monotonic clock.
- Baseline window: 300 ms before each begin marker.
- Event window: begin to end marker with 100 ms pre/post margins by default;
  edge alignment is attempted around the expected 500 ms active window and
  falls back to marker timing with `timing_method=marker`.
- Charge uses baseline-subtracted trapezoidal integration of
  `max(I_uA - baseline_uA, 0) * dt_seconds`; charge_uAh is charge_uC / 3600.
- Peak records raw single-sample peak and p99; current sanity uses p99.
- Voltage sag is not fabricated. CSV fields remain blank and console prints
  `VOLTAGE_SAG=NOT_AVAILABLE_FROM_API`; use a scope for instantaneous VDD sag.

Validation:
- `python -m py_compile Software\PPK2Automation\piezo_current_test.py` PASS.
- `python -m unittest Software\PPK2Automation\tests\test_piezo_current_test.py -v`
  PASS, 12 tests.
- `python -m pytest Software\PPK2Automation\tests\test_piezo_current_test.py`
  was attempted on PATH Python but pytest is not installed. Existing repo
  test style is unittest, so no pytest dependency was added.
- Firmware build PASS on TRIPLEC with STM32duino 3.0.0, Generic L031G6Ux,
  LTO: 25500 / 32768 bytes flash, 2160 / 8192 bytes static RAM.

HIL status: SKIPPED. No flash/upload, target reset, PPK2 acquisition, UART
capture, sound check, LIS2 false-trigger check, current measurement or VDD sag
measurement was performed for this new test. Physical next steps: rediscover
current PPK2/UART/ST-LINK ports and wiring, flash only
PiezoLIS2InteractionTest, run the script at 2.8 V with `--off-wait 15`,
retain the CSV, listen for 2/3/4 kHz subjective loudness, and verify VDD sag
with scope if needed. Instrument state: no Codex-started supply/acquisition or
background process was left running in this session.

### Piezo HIL start / PWM re-arm fix

User requested starting physical test. Current ports on TRIPLEC:
PPK2 control COM7 (HW=1518, Calibrated=0), PPK2 secondary CDC COM6,
USB-UART COM8 (Prolific). Python PATH initially lacked pyserial/ppk2-api;
installed `pyserial==3.5` and `ppk2-api==0.9.2` from
`Software/PPK2Automation/requirements.txt`.

Initial PiezoLIS2InteractionTest flash used PPK2 COM7 source mode at 2.8 V
and STM32CubeProgrammer over SWD under reset. CubeProgrammer exit code was 0
but stdout was not retained because of a local cp950 decode issue; subsequent
test markers confirmed the piezo firmware was running. PPK2 OFF was sent from
the wrapper finally.

First automation run at 2.8 V, `--off-wait 15 --timeout 45`:
CSV `Software/PPK2Automation/results/piezo_current_20261014_163817.csv`.
Result FAIL. UART functional PASS, 9/9 events complete. LIS2 FAIL:
1 false trigger on seq=1 / 2000 Hz during beep. Current sanity FAIL on
seq=4 and seq=9 because active average was not above baseline. PPK2 OFF was
sent by the script.

Second automation run, same conditions:
CSV `Software/PPK2Automation/results/piezo_current_20261014_163929.csv`.
Result FAIL. UART functional PASS, 9/9 events complete. LIS2 FAIL:
1 false trigger on seq=1 / 2000 Hz during beep. Current sanity FAIL on
seq=5, seq=7 and seq=8. User reported only hearing one beep. The current
CSV also showed only seq=1 had a large charge step; later beeps mostly had
small deltas, matching the audible observation.

Root cause found in firmware: `stopPiezo()` returned PA0 to GPIO LOW, but the
next `HardwareTimer::setPWM()` call saw the channel was already in PWM1 mode
and did not call `setMode()` again, so PA0 was not restored to timer alternate
function after the first beep. Fixed `startPiezo()` to force
`setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PA0)`, set overflow/capture compare,
reset count, resume channel and resume timer for every beep.

Fixed firmware build PASS on TRIPLEC, STM32duino 3.0.0 / Generic L031G6Ux /
LTO: 25252 / 32768 bytes flash, 2160 / 8192 bytes static RAM. Flash retry
with CubeProgrammer text suppressed returned exit code 0; PPK2 OFF was sent.

Post-fix automation run at 2.8 V, `--off-wait 15 --timeout 45`:
CSV `Software/PPK2Automation/results/piezo_current_20261014_164204.csv`.
Result FAIL. UART functional PASS, 9/9 events complete. LIS2 FAIL with
6 false-trigger events: seq=1 (2000 Hz during), seq=4/5/6 (3000 Hz during
and after), seq=8/9 (4000 Hz during and after). Current sanity FAIL on
seq=5 and seq=6 due active average below baseline in addition to LIS2 FAIL.
Voltage sag remains unavailable from ppk2-api and was not scoped.

Mean active / p99 / charge from the post-fix run:
- 2000 Hz: mean active 6140.033 uA, mean p99 7992.162 uA,
  mean charge 787.334 uC, false triggers 1/3.
- 3000 Hz: mean active 6895.840 uA, mean p99 7432.605 uA,
  mean charge 36.836 uC, false triggers 3/3.
- 4000 Hz: mean active 6856.046 uA, mean p99 7629.217 uA,
  mean charge 59.418 uC, false triggers 2/3.

User confirmed hearing 9 beeps after the fix, matching the 9/9 UART marker
sequence. This confirms the PWM re-arm fix restored repeated audible playback.

Important qualification: the post-fix run proves the marker sequence,
repeated audible playback and automation flow, but does not pass acceptance.
The unusually large seq=1 charge and high active baseline after boot need
review before using these numbers as product energy estimates. Do not raise
LIS2 thresholds automatically from this test; record the false triggers and
decide the next controlled experiment separately. No instrument session is
intentionally left running; the script/wrappers sent PPK2 OFF.

## 2026-10-15: Android environment / independent Kotlin smoke build

Scope: environment checks and minimum Android build only. No MCU firmware,
PPK2Automation or PeboBridge files modified; no commit/push, global environment,
registry, driver, tool upgrade or SDK license acceptance performed.

Observed Windows computer name: LAPTOP-1JNDJFQA (Windows 10.0.19045).
This does not match the three registered names; relationship to PC-01/02/03
remains unverified. Do not relabel an existing profile based on the echou path.
Workspace: G:/我的雲端硬碟/Application/Project/Pebo.
Git 2.53.0.windows.3, branch main, HEAD
11e2f6e8954aa45e373c615d853d323aac053979. Status main...origin/main uses
cached refs only; no fetch or synchronization claim. Existing modified
AGENTS.md, README.md, Software/PeboBridge/README.md and requirements.txt,
docs/hil-test-plan.md, docs/power-budget.md and untracked FW/BatteryVoltageTest,
FW/LedBrightnessTest, FW/PiezoLIS2InteractionTest, Software/PPK2Automation and
handoff/machine docs preserved. Session repository edits: only handoff.md and
development-machines.md. All changes saved locally, uncommitted/unpushed.

- Android Studio: C:/Program Files/Android/Android Studio;
  AndroidStudio2026.2.1, AI-262.9437.185.2621.16467767 (product-info.json).
  Bundled JBR 25.0.3, java -version executed successfully.
- CLI Java: Oracle 26.0.1; JAVA_HOME unset. java resolves through
  C:/Program Files/Common Files/Oracle/Java/javapath/java.exe.
  Build used Studio JBR only in its child PowerShell process. Gradle 9.3.1
  supports JDK 25; JDK 26 requires Gradle 9.4+ in the official matrix.
  No existing Android project/IDE Gradle JVM setting was verified.
- SDK: C:/Users/echou/AppData/Local/Android/Sdk. Platform android-37.0
  revision 2 / Android 17, API 37.0 extension 22; Build Tools 36.0.0;
  Platform Tools 37.0.1-15733141 / adb 1.0.41. Necessary SDK files verified.
  adb not on PATH, full path worked. sdkmanager absent from SDK/Android
  installation search; Studio SDK Manager UI not exercised. No emulator needed.
- No Android Gradle project or wrapper found in repository, including hidden
  file search. Independent project created outside repository at
  C:/Users/echou/AppData/Local/Temp/Pebo-Android-EnvCheck-20261015.
- Stable AGP 9.1.1, Gradle Wrapper 9.3.1, built-in Kotlin compiler 2.2.10,
  JBR 25.0.3, existing SDK 37.0 / Build Tools 36.0.0. App ID
  com.pebo.envcheck20261015; min API 23 / target API 37. Kotlin Activity
  displays only "Pebo Android 環境測試成功". No NFC permission or NFC logic.
- Wrapper --version PASS; assembleDebug BUILD SUCCESSFUL in 1m 8s,
  34 executed tasks. APK signature verification PASS (v1/v2); aapt confirmed
  package, SDK bounds and launcher Activity. APK size 848032 bytes.
  APK: app/build/outputs/apk/debug/app-debug.apk under temporary project.
  SHA256 868DB23EF02BB29FC7126973AA4645D98E6CE97EF1D150CA19AA2586DF0E8A5B.
  README.md, wrapper-version.log and assembleDebug.log retained there.
  Gradle/Maven downloaded to project-local .gradle-user-home; official Gradle
  distribution SHA256 pinned for future downloads (download ZIP already removed
  by Wrapper, so no independent post-download ZIP hash comparison was possible).
- Non-blocking warnings: SDK XML v4 encountered by parser supporting v3;
  apksigner native-access and META-INF metadata-entry warnings. Build/signature
  checks succeeded. SDK automatic download disabled in test project.
- adb devices -l repeatedly returned no devices. Phone model/Android/API,
  install/launch, NFC feature declaration, NFC switch and NFC actual sensing
  all remain UNVERIFIED. No app installed or data cleared.
- Process state: single-use Gradle daemon exited, no java process remained;
  adb server PID 53788 remained running. No MCU/instrument commands issued;
  current PPK2/UART/ST-LINK connections and electrical state not checked.
- Executor: default sandbox process refresh failed; approved local execution
  was used. No system settings were changed to work around this.

Next: connect an unlocked phone with USB debugging enabled using a data cable,
accept its RSA authorization, rerun SDK platform-tools/adb.exe devices -l.
If absent, try another cable/USB port and inspect Windows Device Manager/phone
USB mode; driver installation is a separate user action if necessary.
If exactly one authorized physical phone exists, install this independent APK
and launch com.pebo.envcheck20261015/.MainActivity, then verify foreground UI.
Read only model/release/API and pm list features for android.hardware.nfc;
NFC sensing is outside this task. Temp artifacts are local and not portable
or protected from Windows cleanup. Handoff saved in this file.

Compatibility references checked:
https://developer.android.com/build/releases/agp-9-1-0-release-notes
https://docs.gradle.org/current/userguide/compatibility.html
### 2026-10-15 follow-up: user connected phone

After the user reported connecting the phone, adb devices -l was executed
twice more and remained empty (no device or unauthorized entry). Windows
Get-PnpDevice -PresentOnly does see WPD device XQ-DQ72 with status OK.
This confirms Windows portable-device enumeration, not an ADB connection.
No matching Android/ADB named device was found by the filtered PnP query;
this alone does not establish a missing driver. Android release/API/model
from adb, NFC feature/switch, APK installation and foreground launch remain
unverified. No installation or data operation attempted.
Next minimum user action: unlock phone, enable Developer options > USB debugging,
reconnect and accept the RSA authorization prompt. If still absent, try USB
file-transfer mode, a data cable/another USB port and inspect Device Manager
for an Android ADB interface; no driver or security setting changed by Codex.
Build remains PASS; only adb server intentionally remains running.
### 2026-10-15 follow-up: USB file-transfer mode enabled by user

User confirmed switching phone USB to file transfer. adb devices -l still
returns an empty list; no unauthorized entry. Windows WPD XQ-DQ72 remains
OK (CM_PROB_NONE); filtered present-device query shows no named Android/ADB
interface. USB debugging/RSA authorization must be checked next; this query
does not prove a missing driver. No install/launch/NFC query possible yet.
No tools, settings, drivers or application data changed. Git remains main at
11e2f6e8954aa45e373c615d853d323aac053979; existing work preserved, no commit
or push. This follow-up only appends the two handoff/machine documents.
Build result remains PASS; adb server remains available; no instrument action.
Next: enable USB debugging in Developer options while unlocked, reconnect,
accept the computer authorization, then rerun adb. Handoff saved locally.
### 2026-10-15 follow-up: USB debugging authorized / physical install and launch

After user enabled USB debugging, adb devices -l showed exactly one authorized
physical device. Read-only getprop: manufacturer Sony, model XQ-DQ72,
Android 15, API 35; ro.kernel.qemu empty. Serial intentionally omitted from
handoff. pm list features declares android.hardware.nfc, nfc.any, nfc.ese,
nfc.hce, nfc.hcef and nfc.uicc. Filtered dumpsys nfc reports mState=on;
initial mScreenState=OFF_LOCKED. This confirms advertised NFC support and
service enabled state, not actual RF/tag/Pebo NFC communication.

Verified test application ID was absent before installing (no replacement).
Installed the previously verified independent com.pebo.envcheck20261015 APK:
adb install returned Success. am start -W returned Status: ok, COLD launch,
MainActivity, TotalTime 438 ms / WaitTime 439 ms. pidof confirmed running
process 9005; activity dump shows ResumedActivity and focused app is
MainActivity. Window focus still NotificationShade/lock overlay and filtered
uiautomator test did not find the success text; unobscured visual screen text
remains unverified until the user unlocks/dismisses overlay. No security settings
changed or lock bypass attempted. No other app removed or data cleared.

Results now: toolchain/build PASS, physical installation PASS, Activity launch
and running process PASS, NFC advertised support/enabled state PASS, actual
NFC sensing NOT TESTED. APK/project path and SHA256 unchanged from above.
Only the two handoff/machine documents appended; no code change/commit/push.
Test app remains installed/running and adb server available. MCU/instruments
untouched. Next: unlock phone and verify success screen; NFC interaction is
outside this environment-only task. Handoff saved locally.
### 2026-10-15: User-confirmed physical success screen PASS

User explicitly reported PASS after unlocking the phone/dismissing the overlay
and checking the displayed text "Pebo Android 環境測試成功". This completes
physical success-screen verification based on user visual confirmation;
the prior automated UI text query was obscured and is not reclassified.
Final results: minimum Kotlin APK build PASS; physical install PASS;
Activity launch PASS; user visual screen confirmation PASS; NFC feature
support and enabled service confirmed by read-only queries; actual NFC
sensing/communication NOT TESTED and not implemented in this task.
Test project/APK location and versions unchanged. This follow-up only appends
docs/handoff.md and docs/development-machines.md; no commit/push or code/tool/
system changes. No new device/instrument commands issued; test App and adb
were last observed running, current process state not rechecked in this
acknowledgment. No remaining blocker for this minimum environment validation.
Handoff saved locally. Next: retain temporary artifacts if needed; production
Android/NFC work requires a separately scoped task.
## 2026-10-15: Pebo NFC-V physical discovery PASS

User requested actual NFC sensing and selected the Pebo board/ST25DV target.
Current computer still LAPTOP-1JNDJFQA (mapping to original PC inventory
unverified); Git 2.53.0.windows.3, main at
11e2f6e8954aa45e373c615d853d323aac053979. Existing uncommitted work preserved;
no commit/push/fetch or global setting changes. Relevant README, AGENTS,
NFC/hardware pinmap/HIL and handoff/machine records read before work.

Created separate temporary project (original environment app unchanged):
C:/Users/echou/AppData/Local/Temp/Pebo-NfcProbe-20261015.
App ID com.pebo.nfcprobe20261015; label Pebo NFC 唯讀測試.
Same stable AGP 9.1.1 / Wrapper 9.3.1 / Kotlin 2.2.10 / Studio JBR 25.0.3,
existing API 37.0 / Build Tools 36.0.0. Reused original test project Gradle
cache, no SDK download/upgrade/license acceptance. assembleDebug PASS in
20 seconds, 34 tasks; signature verification PASS; independent new install
Success and cold Activity launch Status: ok (367 ms). Existing Apps preserved.
APK: app/build/outputs/apk/debug/app-debug.apk below temporary project;
SHA256 621E1743BAAB736D356B710B5017B58079E500B74765BB9F0FD9A1E3FA30CD56.
README.md, assembleDebug.log and physical-detection.log retained there.

Probe uses ReaderMode NFC_V | SKIP_NDEF_CHECK. No application transceive,
NDEF probing, data-memory read/write, formatting, password, lock or custom
command. UID display uses Android Tag.id byte order. Host evidence omits UID.
Private status/metadata saved for current Activity session; no other app data
collected. Polling runs while Activity resumed; keep-screen-on is Activity-only.
No MCU/PPK2Automation/PeboBridge source or instrument/power operation.

Physical evidence on Sony XQ-DQ72, Android 15/API 35:
- Session 1792033259057; first 45-second window READY/count=0.
- Later callback at phone-log local 10-15 11:02:30.815:
  NFC_DETECTED session=1792033259057 count=1 nfcV=true uidLength=8 tech=NfcV.
- State NFC_V_DETECTED; NFC service on; screen ON_UNLOCKED; probe Activity
  top-resumed. User independently confirmed "NFC-V 實際感應成功" after using
  the selected Pebo target. Together this is physical discovery PASS.
- Final saved counter/state in physical-detection.log may include later taps;
  first-event evidence above is preserved and is not a reliability measurement.
- Board supply state was asked but not provided; battery/PPK2/voltage, board
  revision, fitted K vs KC, ferrite/case and geometry remain unverified in this
  test. No claim of powered/unpowered operation, read range or battery impact.
- Discovery proves an NFC-V RF response; it does not uniquely prove chip model,
  Pebo application identity, user-memory readability, payload freshness,
  MCU wake/handshake, time sync, battery or drinking data. Those were not tested.

ST manufacturer PDF DS13519 Rev 8 (July 2024), pages 1 and 3 directly verified
via web PDF: ISO/IEC 15693/NFC Forum Type 5. This PDF was accessible; no
restricted-PDF disclaimer needed. Android ReaderMode/NfcV docs verified.
Sources:
https://www.st.com/resource/en/datasheet/st25dv04kc.pdf
https://developer.android.com/reference/android/nfc/NfcAdapter
https://developer.android.com/reference/android/nfc/tech/NfcV

Session repository changes: appended docs/nfc-protocol.md (scope/limits),
docs/hil-test-plan.md (procedure), docs/handoff.md and current machine
observations in docs/development-machines.md. All saved locally only.
Build daemon exited; last process query saw adb and no java. NFC probe remains
installed and foreground/reading while resumed; exit the App to stop polling.
No instrument connections/power states checked or changed. Handoff saved.
Next optional scope: document exact board/power conditions and repeat tap
reliability, or define a bounded read-only ST25DV memory test separately.
Do not implement product payload/writes until versioned format is confirmed.
## 2026-10-15: Addressed ST25DV single-block read implemented / final APK offline

User explicitly requested read-only engineering memory test in existing project,
then authorized continuing offline while disconnecting the phone.
LAPTOP-1JNDJFQA; identity mapping to original 3 PCs still unverified. Git
2.53.0.windows.3, main at 11e2f6e8954aa45e373c615d853d323aac053979;
main...origin/main is cached only; no fetch/commit/push. Existing local work
preserved. Applicable AGENTS/README/NFC/HIL/handoff/machine docs read.

Project continued IN PLACE:
C:/Users/echou/AppData/Local/Temp/Pebo-NfcProbe-20261015.
No duplicate or new production Android project. App ID unchanged:
com.pebo.nfcprobe20261015; versionCode=2/versionName=1.1. Changed files:
- app/src/main/java/com/pebo/nfcprobe20261015/MainActivity.kt
- app/src/main/java/com/pebo/nfcprobe20261015/SingleBlock.kt (new)
- app/src/test/java/com/pebo/nfcprobe20261015/SingleBlockTest.kt (new)
- app/src/test/java/com/pebo/nfcprobe20261015/ReadGateTest.kt (new)
- app/build.gradle (version and JUnit test dependency only); README.md.
Original MainActivity saved as MainActivity.discovery-backup.kt outside source
sets. Existing unarmed NFC-V discovery remains; no automatic memory read.
No MCU firmware, PPK2Automation or PeboBridge modifications or instruments.

Features: decimal block 0..127/default 0; Prepare consumes exactly one next
NFC callback; arm/busy guards, no retry/parallel read; worker-thread connect/
transceive/finally close; pause cancels pending/running requests, disables
ReaderMode and closes active connection on cancellation executor. Serialized
close-once prevents delayed cancel cleanup interfering with next read. Busy
persists until worker finishes; cancelled/stale completion cannot show success.
UI shows NFC support/on state, raw-order NFC UID (not Pebo Device ID), actual
block, built TX/sent state, entire Android RX, validated data, specific failure
and separate explicitly historical last success. Tag errors/code, TagLost,
other I/O, malformed/invalid-tag/runtime/cancel errors distinguished. Zero/FF
valid, no application interpretation. Timestamp/block/TX/RX/result/error logs
bounded latest100, user Copy/Clear, in-memory plus bounded Activity saved state.
Manifest unchanged: NFC permission only, no network/storage permissions.
Existing internal status.json retained for diagnostics, no read-log disk/network
persistence. No external clipboard operation except user Copy button.

Protocol/source checks:
- KC DS13519 Rev8 p3 and pp80-83/88-89 directly checked.
  https://www.st.com/resource/en/datasheet/st25dv04kc.pdf
- K family DS10925 Rev11 p3 and pp75-78/83-84 directly checked via official
  https://www.st.com/resource/en/datasheet/st25dv64k.pdf (includes 04K/16K/64K).
  st25dv04k.pdf URL failed; no claim derived from that failed URL. Alternative
  official family PDF was directly accessible and used as evidence.
- Both 04K variants:128 user EEPROM blocks x4 bytes; restriction0..127 matches.
- TX 22 20 [8 raw Android Tag.id bytes, no reverse] [1-byte block]; flags
  high-rate/addressed, Option/Select/Inventory/extension clear; no App CRC.
- Android RX success flags00/exact5 bytes; error01/exact2 bytes with code;
  other flags/empty/wrong lengths malformed. RX shown as Android returns it,
  without adding RF framing or app-side CRC. Android reference directly read:
  https://developer.android.com/reference/android/nfc/tech/NfcV
No writes/locks/password/system/config commands or Pebo payload format added.
Actual fitted chip variant and user EEPROM access protections not inferred.

Validation FINAL:
.\gradlew.bat testDebugUnitTest assembleDebug --no-daemon --console=plain
using same process-only JBR25.0.3, AGP9.1.1/Kotlin2.2.10/Gradle9.3.1,
SDK37.0/BuildTools36.0.0 and existing original environment-test Gradle cache.
BUILD SUCCESSFUL in29s;40 tasks(8 executed/32 up-to-date).
20 JUnit PASS:14 command/parser/input tests,6 one-shot/lifecycle gate tests;
0 failures/errors/skips. Covers exact command/UID/noCRC/bounds, success,
00/FF,known/unknown tag errors,empty/wrong-length/reserved flags,unarmed
callback,duplicate prevention,background cancellation/rearm/stale completion.
Final APK signature verify PASS; existing SDK XML/native-access/META-INF
warnings non-blocking. No tool upgrades/global environment/license changes.
APK: app/build/outputs/apk/debug/app-debug.apk under existing project;
884385 bytes; SHA256
BBEB950B61B57F83B02A1509330D4AB2AAC4A2352776948A6F3015FAE575E83E.
Logs:single-block-build-test.log; XML app/build/test-results/testDebugUnitTest;
HTML app/build/reports/tests/testDebugUnitTest/index.html.

Phone/install distinction:
One authorized Sony phone at install stage; intermediate block-read build
C5B9AF09605659E4454B96A79E7F185EBF228D1F9AF7EC8D794770462FC908AD
updated via install -r Success, launch Status:ok (847ms). Initial app status
PAUSED; no block read evidenced. User then disconnected phone; subsequent adb
queries returned no devices. Final synchronized-close APK above was built
AFTER disconnect and is NOT installed. Update same app when user reconnects;
no uninstall/data-clear. Prior NFC-V physical discovery/user confirmation PASS
remains historical; memory-read PASS is not claimed.

All five physical checks PENDING: block0=4 bytes; prepare/retap5x with no
unarmed repeat; remove DURING I/O then recover; invalid UI inputs -1/128/text
rejected; background cancellation/return/reprepare. Parser/gate unit passes
are not substituted for actual UI/RF/lifecycle checks. Copy/Clear UI pending.
When ready: reconnect/unlock USB-debug phone, update final APK, open app,
move Pebo away, input0/Prepare, bring antenna close/hold for result. Record TX,
RX,4 bytes,error if any and actual board/power/variant/antenna conditions.

Session repository edits only appended docs/nfc-protocol.md,
docs/hil-test-plan.md, docs/handoff.md and docs/development-machines.md.
All saved locally,uncommitted/unpushed; temporary project not cloud/Git synced.
Latest process query:adb running,no java; single-use build daemon exited.
Phone disconnected,remote App state unverified; polling only while foreground.
No power/supply/MCU/instrument command. Handoff saved locally.
### 2026-10-15: Phone reconnected, adb authorization pending

User reconnected phone. adb devices -l shows one device in unauthorized state;
no install or phone-data query attempted. Final APK still matches SHA256
BBEB950B61B57F83B02A1509330D4AB2AAC4A2352776948A6F3015FAE575E83E.
Final build remains pending install; physical memory-read tests remain pending.
Next: unlock phone and accept USB debugging RSA prompt, then update same App
with install -r and launch. LAPTOP-1JNDJFQA/main at
11e2f6e8954aa45e373c615d853d323aac053979; existing changes preserved,
no commit/push. Only these two documents appended, saved locally. adb server
available; no MCU/instrument/power actions or global setting changes.
### 2026-10-15: Final APK installed / Block 0 physical read PASS

After user accepted USB debugging, exactly one authorized Sony XQ-DQ72 was
present. Final APK SHA256 verified against recorded BBEB950B61B57F83B02A1509330D4AB2AAC4A2352776948A6F3015FAE575E83E.
install -r returned Success; Activity cold launch Status:ok,261ms. NFC on,
screen ON_UNLOCKED,session1792039552199 initially READY/count0/no arm/no log.
User operated Prepare and presented target; observed at phone log12:46:08.241:
READ_RESULT session=1792039552199 block=0 result=SUCCESS.
Private status:SUCCESS,count1,readBusy=false,armedBlock=-1,logCount1.
App UI independently captured: flags/length valid; full Android RX
00 00 00 00 00; validated four data bytes00 00 00 00. This is Block0
physical memory-read PASS, not just discovery/build. Zero data accepted as
raw bytes, no battery/drinking/payload interpretation. Full TX and RX retained
locally in existing project/single-block-physical-result.log (UID not copied
into shared handoff). UI capture taken only from test App.

Final build is now installed (supersedes previous pending-install note).
Remaining checks UNVERIFIED:five rearmed retaps/no unarmed repeat; interruption
DURING I/O/error/recovery;invalid inputs via actual UI;background/return/rearm;
Copy/Clear UI. Command/parser/gate20-unit-test PASS unchanged. Board supply,
exact fitted K/KC variant and RF geometry still not independently verified.
App remains installed and last observed foreground idle after success; reader
continues discovery only until another user Prepare. adb server available;
no instrument/power/MCU actions,code edits,commit/push or global setting changes.
Only handoff/machine/HIL/protocol result notes appended,all saved locally.
Next:operator proceeds through remaining physical checklist;do not call them
PASS from this single successful read. Canonical handoff saved.
## 2026-10-15: Pebo Protocol v0.1 Android layer and explicitly Sample UI complete

Machine: LAPTOP-1JNDJFQA (mapping to the original three PCs still unverified).
Git 2.53.0.windows.3; main at 11e2f6e8954aa45e373c615d853d323aac053979.
Cached main...origin/main only; no fetch, commit or push. Existing modified and
untracked work preserved. Applicable instructions, source and handoff read.
User supplied the full v0.1 request and explicitly accepted provisional CRC and
History formats with a clearly labelled Sample mode. MCU protocol is not ready.

Continued the SAME local project:
C:/Users/echou/AppData/Local/Temp/Pebo-NfcProbe-20261015.
Same App ID com.pebo.nfcprobe20261015; versionCode 3 / 0.1-protocol-sample.
New files under app/src/main/java/com/pebo/nfcprobe20261015:
- protocol/Wire.kt, Models.kt, PayloadCodec.kt (pure Kotlin, no Android/ST imports).
- transport/PeboTransport.kt, SamplePeboTransport.kt, St25NfcVBlockTransport.kt.
- ProtocolActivity.kt.
New test: app/src/test/java/com/pebo/nfcprobe20261015/protocol/ProtocolTest.kt.
Changed: MainActivity.kt (new button and ST adapter), app/build.gradle (version),
AndroidManifest.xml (internal Activity and label; NFC permission unchanged),
README.md. Original single-block Activity saved outside source sets as
MainActivity.single-block-backup.kt. Added PROTOCOL-v0.1.md and original user
request copy protocol-user-request-v0.1.txt. No duplicate project created.

Architecture: packet/LE/CRC and typed payloads are independent of ST registers.
PeboClient checks command, response direction, sequence, CRC and typed payload.
All eight commands work against the in-memory Sample through actual codecs.
The real packet transport returns an explicit missing-binding error and sends
no protocol commands. Existing actual discovery and one-shot single-block read
remain; that adapter is still the only actual transceive call.
UI includes distinct actual UID and Pebo Device ID, Info, Status, Config editor,
History pages, Sync Time and Debug TX/RX/status/CRC plus offline HEX parsing.
Sample banners remain visible; switching source clears decoded values.

Provisional choices are documented in PROTOCOL-v0.1.md and nfc-protocol.md:
CCITT-FALSE (header+payload, CRC LE), status-prefix/status-only ACKs, explicit
History prefix with 40 records/page, half-open periods ending at up to 1440,
unique IDs, zero-period/target allowances. Payload limit is 248, not 255.
Drink events and completed periods are separate (Sample 3 vs 1).
Before MCU integration: ratify CRC/ACK/History, define EEPROM/mailbox allocation,
atomic commit/wake/ACK/timeout/retry/idempotency/authentication and freshness;
resolve History snapshot/index rollover, local day/DST, uint8 count overflow,
LED 8 vs period 16 vs target, and undefined status/flag semantics.
No firmware agreement or real application-protocol success is claimed.

Validation: compileDebugKotlin PASS; testDebugUnitTest assembleDebug PASS in
19 s, 40 tasks (17 executed, 23 up-to-date). 58 JUnit tests PASS: 38 new protocol
plus 20 existing engineering regressions; 0 failures/errors/skips. Covers all
12 requested groups, Python-binascii independent CRC fixture, signed/unsigned
LE, config/records, 40/40/5 pagination, all Sample commands, sequence checks,
unbound real transport and 2000 random malformed buffers through parsers.
Signature PASS; previous SDK XML / native-access / META-INF warnings non-blocking.
No new dependencies/permissions. Same process-only JBR 25.0.3, AGP 9.1.1,
Gradle 9.3.1, Kotlin 2.2.10, SDK 37.0 and Build Tools 36.0.0.

APK: app/build/outputs/apk/debug/app-debug.apk under the existing project;
916853 bytes; SHA256 E79CB668514AA061C4628E5909F5F87587FD6AE4BEBB5F06DF679D90726F35C7.
Log: protocol-build-test.log. Reports: app/build/reports/tests/testDebugUnitTest
and app/build/test-results/testDebugUnitTest.
Phone was initially authorized but adb listed no devices at installation time.
Version 3 NOT installed; UI and hardware not verified for this build. Prior
physical discovery / Block 0 PASS remains historical, not protocol evidence.

Repository edits: only appended nfc-protocol.md, hil-test-plan.md, handoff.md
and development-machines.md. No MCU, PPK2Automation or PeboBridge modifications,
flash, supply, NFC write or instrument operations. Latest process query saw adb
and no java; the single-use build daemon exited. Global settings/tools unchanged.
All work saved locally, uncommitted/unpushed; App source is local TEMP and is
not Git/cloud synchronized. Canonical handoff saved.
Next: reconnect/authorize phone, update APK, open Protocol/Sample button and
smoke test all sections and real-mode unavailable error; regress physical block
read separately. Resolve listed wire/transport questions before MCU integration.
## 2026-10-15: Android handover on PC-01 / source transfer required

User requested taking over current Android App development. Current computer
verified as TripleC_AERO (PC-01), Windows user tripl. Workspace:
G:/我的雲端硬碟/Application/Project/Pebo. Read AGENTS.md, README.md,
canonical handoff, machine registry, NFC protocol and relevant HIL checklist.
Git 2.53.0.windows.3; main HEAD 11e2f6e8954aa45e373c615d853d323aac053979;
only cached main/origin/main refs present. No fetch/commit/push performed.
Existing modified/untracked work preserved; session edits only this handoff
and development-machines.md.

Latest Android state is HISTORICAL evidence from LAPTOP-1JNDJFQA above:
versionCode3 Sample/protocol build, 58 passing tests, version3 not installed;
physical discovery/Block0 PASS belong to the previous engineering version.
Real MCU packet binding remains undefined; do not claim protocol HIL success.

Current blocker: recorded original project
C:/Users/echou/AppData/Local/Temp/Pebo-NfcProbe-20261015 is absent here.
No Kotlin/APK/Gradle wrapper/PROTOCOL-v0.1.md found in this workspace or the
shared Application/Project tree. No Pebo Android project in local user TEMP;
C:/Users/tripl/AndroidStudioProjects absent. No replacement app generated.
Asked user to provide synced path or copy the complete original project to
Software/PeboAndroid (suggested destination; not created or verified).
Preserve wrapper, source, tests, PROTOCOL-v0.1.md and user request; transfer
logs/evidence if needed. Exclude local.properties and build caches from Git;
retain the original project until transferred files are verified.

Current machine tool checks PASS (version queries only):
- Android Studio installation: C:/Program Files/Android/Android Studio.
- Bundled jbr/bin/java.exe: OpenJDK 25.0.3, build25.0.3+-15898627-b508.16.
- SDK C:/Users/tripl/AppData/Local/Android/Sdk; platforms/android-37.0 and
  build-tools/36.0.0 directories present (build not run).
- SDK platform-tools/adb.exe:1.0.41, package37.0.1-15733141.
- java/adb/gradle not discovered on PATH; JAVA_HOME/ANDROID_HOME/
  ANDROID_SDK_ROOT not observed. User .gradle directory absent.
- Sandbox cannot access this Drive mount; explicit outside-sandbox read/write
  commands used. No global setup, downloads or license changes made.

Validation: no Android build/tests run because source is unavailable.
Phone authorization/connections and physical tests UNVERIFIED this session.
No adb server/device query, install, NFC command, MCU/flash/power/instrument
operation issued. Final process query found no adb/java/studio64 processes;
no session-owned background process started. Handoff saved locally only,
uncommitted/unpushed; cloud replication not verified.

Next: obtain original source, inspect nested instructions/build configuration,
verify App ID com.pebo.nfcprobe20261015 and version3, configure machine-local
SDK path, then rerun testDebugUnitTest assembleDebug and compare results to
58 historical tests. Continue Sample UI checklist; real protocol integration
requires the documented wire/atomic transport decisions. Rediscover phone
before any authorized install or physical regression.

## 2026-10-15: PC-01 Android environment verification (requested separately)

User requested checking App development environment while locating original
source. Current PC TripleC_AERO, main HEAD11e2f6e8954aa45e373c615d853d323aac053979;
Git2.53.0.windows.3; existing changes preserved. No fetch/commit/push.
Android source/wrapper still absent in repository at this check.

Verified actual installed tools:
- Android Studio product-info.json: AI-262.9437.185.2621.16467767,
  data directory AndroidStudio2026.2.1, minimum runtime Java25.
- Studio bundled java AND javac25.0.3 execute successfully.
- SDK platform android-37.0 revision2 (API37.0), Build Tools36.0.0.
- aapt2 version2.20-13193326 PASS; apksigner version0.9 PASS;
  D8 version8.10.9-dev PASS. apksigner reports nonfatal native-access warnings.
- Independent Java source compiled/executed PASS; compiling with explicit
  --release17 followed by D8 --min-api26 using platform android.jar generated
  classes.dex (1368 bytes), PASS. This is a tool smoke test, not Pebo APK.
- Initial javac default25 output failed D8:unsupported class major69.
  Explicit Java17 target resolved the smoke test. Inspect original project's
  Java/Kotlin bytecode target before build; do not infer its target from this
  diagnostic. No SDK/JDK upgrade performed.
- adb1.0.41/package37.0.1 executed; devices -l starts server and found exactly
  one unauthorized device. No install, device-data query or authorization
  bypass attempted. User unlock/RSA acceptance required before phone testing.
- Emulator executable and Medium_Phone_API_37.0 AVD .ini present; emulator
  not launched, acceleration/runtime not tested. This does not validate NFC.
- SDK licenses/android-sdk-license file present; no license acceptance changed.
- cmdline-tools directory not found, sdkmanager not available on PATH.
- java/javac/adb/gradle not on PATH; no global JAVA_HOME/ANDROID_HOME/
  ANDROID_SDK_ROOT/GRADLE_USER_HOME observed; default user .gradle absent.
  Studio's bundled runtime/absolute SDK tools work; process-only JAVA_HOME
  used for signing/D8 queries. No persistent environment edits.

Remaining: original Gradle wrapper, distribution/dependencies and exact AGP/
Kotlin settings unavailable, so Gradle sync, 58 App tests, complete resource
packaging/signing and APK installation NOT verified on this PC. No standalone
Gradle installed. Restore original source then run wrapper build/tests with
machine-local SDK path; use original versions before changing dependencies.

Artifacts: C:/Users/tripl/.codex/visualizations/2026/10/15/
01a13f1b-1b35-7142-a43d-2b329cb7d4b5/android-env-check contains smoke source,
class and classes.dex. No production source/firmware change or instrument/power
operation. adb server left available after device query; no build daemon or
emulator launched. Handoff and machine registry appended and saved locally,
uncommitted/unpushed; cloud synchronization unverified.

## 2026-10-15: Android source transfer packaged and restore verified

Current machine verified again: LAPTOP-1JNDJFQA; original registry mapping remains
unverified. Git 2.53.0.windows.3; main at
11e2f6e8954aa45e373c615d853d323aac053979. Existing modified/untracked work
preserved; no fetch, commit or push. No MCU, PPK2Automation or PeboBridge edits.
The earlier PC-01 source-unavailable observation is retained as history; source
is NOW saved in this workspace. Destination arrival/readiness is not yet checked.

Saved the existing TEMP project to Software/PeboAndroid (no reimplementation).
23 original source/config/Wrapper/spec files match SHA256; see its
handoff/source-manifest.csv. Original TEMP project retained. Source of record
for transfer is now Software/PeboAndroid; build on the next computer in an
ASCII local checkout/path per its README. No local.properties, Gradle/IDE cache,
SDK, private keystore or credentials transferred. Added project .gitignore.
Preserved source backups, original request/spec, logs, HTML/XML test reports,
physical discovery/Block0 evidence, and exact version3 signed APK. README covers
machine-specific SDK setup, process-only JBR, Wrapper build/test, device discovery,
Sample UI checks and cross-machine debug-signature mismatch without data loss.
Do not uninstall/clear data if a newly signed APK cannot update the old App.

Portable verification: created ZIP, extracted to a separate ASCII TEMP directory,
then ran testDebugUnitTest assembleDebug --offline --no-daemon --console=plain.
First attempt found missing output in old Gradle transform cache. Preserved failure
log; no source/cache deletion or workaround in code. Separate clean cache seeded
from existing Wrapper distribution and Maven modules regenerated transforms.
Final restore PASS: 58 tests, 0 failures/errors/skips; debug build in 1m1s,
40/40 tasks executed. Fresh restore log/reports included in project handoff/.
These are current source-machine results, not a new PASS for PC-01 or other PCs.

Version3 App ID com.pebo.nfcprobe20261015, protocol Sample. Exact provided APK:
handoff/evidence/pebo-v3-protocol-sample-debug.apk (916853 bytes), SHA256
E79CB668514AA061C4628E5909F5F87587FD6AE4BEBB5F06DF679D90726F35C7.
Version3 NOT installed/UI not physically checked. Earlier real NFC-V discovery
and Block0 4-byte read PASS belong to the earlier build. Remaining physical
block tests, Sample UI smoke and real application protocol are pending.
Protocol CRC/History/ACK remain explicitly provisional; true EEPROM/mailbox
ownership/atomic commit/wake/ACK/timeout/authentication still need definition.

Portable archive: Software/PeboAndroid-handoff-20261015.zip, top folder
PeboAndroid; adjacent .zip.sha256 is the expected transfer checksum. Archive
contains a dated canonical handoff/machine snapshot, not a competing live log.
Verify archive SHA256 after transfer and recheck 23 source hashes before use.
Saved to the shared workspace only: cloud replication NOT verified and no Git
commit/push. Wait for Drive transfer to finish; do not use the matching path
alone as proof. Do not run two machines against the same shared .git concurrently.

Session changes: new Software/PeboAndroid/, portable ZIP and checksum sidecar;
README.md adds Android entry; docs/handoff.md adds current-source entry and this
record; docs/development-machines.md appends current-machine result. Previous
PC-01 and other dated observations retained. Original application code unchanged.
No firmware/flash/power/instrument/phone/ADB device query was performed in this
handoff session. Build was single-use; final process observation is recorded below.
No global environment/registry/driver/tool changes. Handoff saved locally.

Next computer: read AGENTS.md and this file; identify machine/Git state; verify
arrival/checksums; read Software/PeboAndroid/README.md and PROTOCOL-v0.1.md;
restore to ASCII local path, generate local SDK setting, run 58 tests/debug build;
then reconnect/authorize one phone, update supplied APK and smoke Sample UI.
Return to engineering read regression separately. Do not implement real protocol
writes before the documented wire/transport decisions are resolved.
Final process observation: No java/adb/studio64 processes observed at final handoff check.

Final archive SHA256: 7698C1CBF57231A3709E781B478339EA84617207E50DE80DB29FA200A3AC6C4B
Archive bytes: 1096782. The adjacent sidecar is included alongside the ZIP; snapshots inside the ZIP precede this checksum line to avoid a self-referential hash.

## 2026-10-15: PC-01 received and verified Android transfer

User requested rereading latest handoff. Current machine TripleC_AERO; main
11e2f6e8954aa45e373c615d853d323aac053979; existing changes preserved.
Latest source-machine transfer note and Software/PeboAndroid README/spec read.
Source-unavailable blocker in earlier PC-01 entries is now RESOLVED here:
Software/PeboAndroid and portable archive/sidecar are present and readable.
Verified ZIP SHA2567698C1CBF57231A3709E781B478339EA84617207E50DE80DB29FA200A3AC6C4B
and supplied APK SHA256E79CB668514AA061C4628E5909F5F87587FD6AE4BEBB5F06DF679D90726F35C7
match latest canonical source-machine handoff. All23 source manifest entries
match both SHA256 and byte length on THIS PC. Wrapper9.3.1/checksum and
app ID/versionCode3 confirmed; Java/Kotlin targets explicitly17.
This verifies arrival/integrity; source-machine58-test/restore PASS remains
historical, not a PC-01 App build/test result.

Next: use ASCII local build copy, configure this PC's SDK/JBR without global
changes, resolve pinned Wrapper/AGP/Kotlin dependencies and run
 testDebugUnitTest assembleDebug --no-daemon --console=plain.
Then separately verify authorized phone and Sample UI; supplied signed APK
retains old signature, local debug signing may differ. No uninstall/data clear.
Real MCU packet binding/provisional decisions remain open.

Only canonical handoff/current-machine registry appended this check. No App
source changes, downloads/build/install/device query, MCU/power/instrument or
Git commit/push/fetch. adb process from earlier environment check observed;
no java/studio64 observed. Receipt saved locally; no remote Git synchronization
claimed. Android source/ZIP currently untracked, preserved as received.

## 2026-10-15: PC-01 full Android build and 58 tests PASS

User requested local tests/APK build. TripleC_AERO/tripl; main HEAD
11e2f6e8954aa45e373c615d853d323aac053979; existing changes preserved.
Original source unchanged (all23 manifest hashes verified before/after build).
Extracted verified archive to new ASCII local directory:
C:/Users/tripl/AppData/Local/Temp/PeboAndroid-PC01-fb4640f8/PeboAndroid.
Generated ONLY local copy local.properties for this PC SDK. Dedicated cache:
C:/Users/tripl/AppData/Local/Temp/PeboAndroid-PC01-fb4640f8/gradle-cache.
Process-only JAVA_HOME=Studio JBR and GRADLE_USER_HOME=dedicated cache.
Pinned Wrapper9.3.1 downloaded successfully (configured checksum enforced);
original AGP9.1.1/Kotlin dependencies resolved. No version/source changes,
global PATH/environment/license edits or standalone Gradle install.

Command: gradlew.bat testDebugUnitTest assembleDebug --no-daemon --console=plain.
BUILD SUCCESSFUL in2m16s;40 actionable tasks,all40 executed;exit0.
JUnit XML verified:ProtocolTest38,SingleBlockTest14,ReadGateTest6;
TOTAL58,failures0,errors0,skipped0. This is an actual PC-01 build/test PASS.
App ID com.pebo.nfcprobe20261015;versionCode3/0.1-protocol-sample;
minSdk23,target37,NFC permission only. Java/Kotlin targets17 unchanged.
New APK916855bytes,SHA256
C2129CD776F1E06CBDA9799F61A55809BB5BCB2E399A4A05EE612EC4AD98AD4C.
apksigner verify PASS v1/v2. SDK XML3/4,signer native-access and META-INF
metadata warnings nonblocking,consistent with historical warning categories.

Saved durable evidence under Software/PeboAndroid/handoff/evidence/
pc01-20261015-build01/:pebo-v3-pc01-debug.apk,checksum sidecar,
pc01-build-test.log,pc01-apk-signature.log,build-summary.json,junit-xml/,
test-report/index.html. Original signed handoff APK/archive preserved.
No private key copied. Local debug certificate differs from original:
new SHA256080ad20bed8f84ed2e9ae80b300bb60a6b19ac1e5990302026ab733e20a7012b;
original SHA256e559eb3dd75ea241b28726488f7069c3abc3f93169eedba7c219f32007da0bb5.
Do not directly update an old-key install with the new-key APK or uninstall/
clear data to bypass mismatch. Use supplied original signed APK for existing
old-key App smoke test,or resolve signing separately when deploying changes.

No phone/device query/install,UI/RF/HIL,firmware/MCU,power/instrument operation.
Single-use build daemon exited;final process query observed adb only,no java
or studio64. Local temp build/cache retained for continued development; not
portable source. Only new evidence plus handoff/machine registry changed;
all saved locally,uncommitted/unpushed;no fetch/remote sync claim.
Next:authorized-phone version3 Sample UI smoke and separate engineering block
regressions. Real MCU transport/provisional protocol decisions remain unresolved.

## 2026-10-15: PC-01 installed version3 on authorized Sony phone

User explicitly requested installing App to phone. TripleC_AERO/main
11e2f6e8954aa45e373c615d853d323aac053979; existing work preserved.
adb devices -l found exactly one authorized Sony XQ-DQ72. Targeted all
operations to that one device. Existing Pebo versionCode2/version1.1 found.
Pulled only installed Pebo base.apk to a new local TEMP inspection directory;
apksigner verify PASS and public certificate SHA256 matched original v3
handoff APK:e559eb3dd75ea241b28726488f7069c3abc3f93169eedba7c219f32007da0bb5.
Rechecked supplied APK SHA256 E79CB668514AA061C4628E5909F5F87587FD6AE4BEBB5F06DF679D90726F35C7.

Installed ORIGINAL compatible-signature v3 handoff APK using adb install -r:
Success. The newly PC-01-built APK was NOT installed because its debug key
is different. Application source manifest is the same verified23 files; both
builds versionCode3/0.1-protocol-sample. No uninstall,clear-data,private-key
transfer or signing override. Launch am start -W MainActivity Status:ok,
COLD,TotalTime1086ms. Postinstall package query confirms versionCode3,
versionName0.1-protocol-sample,minSdk23,target37. No NFC transceive or MCU
protocol command requested. MainActivity may poll NFC while resumed; no
memory read without user Prepare. UI workflows/RF regressions not tested.

Historical 'version3 not installed' statements superseded for this Sony phone.
Sample UI smoke remains pending; real packet transport still unbound.
adb server remains available; App installed and launch succeeded; exact later
foreground state not confirmed by filtered activity query. No firmware,
board power/instruments or global environment changes. Only handoff/registry
appended,saved locally,no commit/push/fetch. Next:open Protocol/Sample button
and verify sections; engineering Block0 separately, retaining no-auto-read gate.

## 2026-10-15: Android nine-item technical handoff prepared on PC-01

User requested nine-item implementation report and handoff document. Added
 docs/android-development-handoff.md (Chinese):file provenance,protocol/packages,
packet/payload codecs,NFC transport/UI,58 local tests,firmware limits,provisional
assumptions,and protocol conflicts/open decisions. This is a technical report;
this file remains the canonical chronological handoff. Source code inspected;
no App changes or extra builds/phone/instrument operations in this report step.

Report explicitly distinguishes original compatible-signature v3 installed on
Sony from newly PC-01-built APK not installed;both checksums and evidence paths
included. Original portable ZIP is unchanged and does not contain this new
report/PC-01 evidence. Next computer must transfer current saved files too.
Document includes memory budget,History snapshot,sequence,idempotency,timezone,
flags/freshness,counts/LED and authentication questions before MCU integration.

TripleC_AERO/main11e2f6e8954aa45e373c615d853d323aac053979;existing changes
preserved. Only technical report/handoff/machine registry changed in this step.
All saved locally,uncommitted/unpushed,no fetch;cloud replication unverified.
Previous build daemon exited;adb previously available,App previously launched;
no fresh process/device/instrument state query made. Next:Sample UI smoke,
engineering physical regressions,then ratify wire/transport before real writes.
