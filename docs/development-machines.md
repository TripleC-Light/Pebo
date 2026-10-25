# Development machine registry

## 2026-10-26: Stage6 committed and published

User explicitly authorizes commit/push after prototype validation. Scoped
stage6 commit 3d6d70a9a60e5c0ea0ed8dd95b6a00ca1f72c1cf successfully pushed to origin/main;
earlier local-only notes below are historical validation-time state.
Includes native source/settings/vendor licenses/scripts/raw evidence and related
status corrections/handoff. Evidence audit PASS; no new firmware or physical
tests during publication. Last board09cdb9cc... off/debug/COM13 closed.
Existing untracked FeedbackTest/TimeManagementTest and conflicted HIL copy
excluded/preserved; build binaries/secrets not included. Existing maintenance
packed-refs.lock warning left untouched; commit and push succeeded.
Next: extended independent single-event/final pickup cohorts, real-water/fault/
stack/power tests as listed in stage6 VALIDATION; no claim of production accuracy.

## 2026-10-26: Stage6 prototype and bounded HIL complete; extended acceptance OPEN

PC-02 TripleC, main HEADa7c03e78c0a5e2c45d6b3c0e98d0c37e1e459304.
New FW/NativeDrinkDetectionTest source/build/scripts/README/VALIDATION/results
saved locally, NO stage6 commit/push. Stage5 README/VALIDATION historical
uncommitted wording corrected only. Native baseline tag820ecb3 and stage5
checkpoint/native-shake-progress-v1 ->128d941 were verified/explicitly pushed
to origin without force2026-10-25. No baseline code or unrelated work removed.
See [stage6 report](../FW/NativeDrinkDetectionTest/VALIDATION.md) for versions,
failed/invalid training attempts, raw XYZ/replay and exact per-image evidence.

Final image09cdb9cc0d952a759e24917fd94fcdd09b01b44ad0b790af84c78a20238c3293,
two independent builds identical; Flash28228/staticRAM4704 (.data104/.bss4600),
Flash spare4540, RAM after static3488, minstack1024/heap0; peak NOT_MEASURED.
Same HAL/startup/linker/RTC/STOP/pins. Adds RAM-only stationary calibration,
one shared100Hz LP1 sampler, integer tilt/hold/return FSM, shake exclusion/rearm,
10s deferred merge using RTC elapsed, bounded raw capture5..30s. Return stability
after unreliable acceleration fixed and regression-tested. No drink scoring,
progress increments, audio, NFC app data or NVM writes.

Only EMPTY direct-drink bottle simulations, same mounting per user; no real
intake/volume/pouring/straw accuracy claim. Final-image10 shakes user confirms
correct level5/about3s off,0 drink. Three independent user-confirmed3-sip groups
each3 candidates ->1 event exactly10s after last, raw3015 rows/group and exact
C++ replay PASS. Prior-image a17... pickups10/no false display retained as
prior-version evidence, not silently promoted to final image. One invalid
three-sip attempt and one late-prompt attempt excluded by operator confirmation.

Final flash/verify/full readback, COM13 UART, explicit Windows PC UTC/NTP
unverified, ordinary reset preserves valid RTC/BKP0..4, run10/60/off/restart,
LED0..8 timeout/parser/elapsed anchor, controls/cleanup PASS. Awake RF ISR4/
confirmed3/unknown1/errors0; STOP RF ISR/wake/confirmed1, STOP47/unknownWake0
(motion1 also observed). Mixed user confirms single then phone:1 candidate/
event,10.489s defer, stops9->20 during merge, motion/GPO wake1 each, STOP54,
RTC60/debug recovery/zero errors. Saved evidence audit12 result sets PASS.

Current UART reconfigured from historical COM5 to single Prolific COM13
(067B:2303),1152008N1; selected explicitly and identity checked. Nordic COM6/7
not opened. STLINK56FF6C066688555210351387, MCU425/RevX, VTref2.72..2.73V.
User provides2.8V; no PPK2/measurement/power/option-byte/backup-reset operations.
Last observed board final image, off/debug, all sources/display/PWM/sampling
off, RTC valid, RAM reference21,-1013,-109, progress0 manual start cue; UART
closed, no active helper. Physical axes/spout direction not independently mapped.

Remaining: independent10 separate single events (final only1, prior image1),
repeat10 pickup cohort on final image, pending-group physical off/deadline
cancel, continuous motion/pouring/confusion/fill variants, RF exact-overlap/
fault/race stress, peak stack, power and long-term accuracy. Pure cancellation/
timeout boundaries tested, not equivalent to unexecuted physical cases. Historic
RF root cause OPEN. Next session recheck supply/ports/image, recalibrate after
reset, use visible double-flash capture cue and confirm actual trial count;
do not guess timing or pool training with validation. Handoff saved locally.

## 2026-10-26: Stage6 resumed after usage-limit interruption

PC-02 TripleC/main HEADa7c03e7. Stage6 FW/NativeDrinkDetectionTest is local,
uncommitted. Stage5 status documentation corrected, sources preserved.
Both annotated tags verified/pushed2026-10-25: native baseline820ecb3 and
checkpoint/native-shake-progress-v1 ->128d941. No stage6 commit/push.
Last verified stage6 image a17fc14b825531e0b9d2639c492f66162bf59f5a9fc0b68691c8ddd9ac54072b,
Flash28228/staticRAM4704, minstack1024/heap0, peak unmeasured. Last HIL helper
finished off/debug/closed UART; current power/live state awaiting user check.
User selected EMPTY direct-drink bottle simulation, same stage5 mounting.
Training single/pickup/shake/tilt collected; initial sip falsely classified as
shake, preserved. Dynamic-norm shake qualifier fixes training replay. First
three-sip attempt operator says not executed correctly: exclude from accuracy.
Retry train-three02:3 candidates merged into1 event10s after last, user confirms.
Independent validate-single-stop01:1 candidate/event,10.129s defer, stops2->13,
motion ISR/wake1, deadline40/debug/UART PASS. User confirms action.
Independent pickup5a+5b: user confirms10 pickups/no LED; candidates/shakes/errors0.
validate-shake5a:1 preliminary gesture before run +3 formal gestures,4 confirms
total,0 drink. User confirms counts; final display truncated by deadline, NOT
full3s optics acceptance. Long debug shake window NOT started: approval review
failed on usage limit before patch/execution. No instruments operated.
Next: recheck board/image/calibration; finish bounded shake, separate three-sip
validation, RF awake/STOP/mixed, auto10/60/LED/cleanup; report missing scenarios
plainly. Power/stack/fault/race/real-water acceptance remain unverified.

## 2026-10-25: GitHub authentication resolved; main published

User completed device login. Normal git push origin main succeeded:
remote main advanced c8dd924..3283604, including stage5 commit128d941 and
native baseline history. Earlier authentication blocker below is resolved.
This follow-up records successful publication; remote HEAD checked after its
own push. Baseline annotated tag remains local, not explicitly pushed.
Unrelated FeedbackTest/TimeManagementTest files and conflicted HIL copy remain
untracked and preserved. No firmware, UART, target power or instrument action.

## 2026-10-25: Commit saved; push blocked by GitHub authentication

Stage5 commit128d941f0c81c40e1cb0ad388f8817f40f65c13d saved on main.
Normal HTTPS push waited in git-credential-manager, cancelled without changing
remote. Noninteractive retry explicitly failed: no usable GitHub username/
credential, terminal prompts disabled. Remote main last checked remains
c8dd924bc6a022979da2a70315cf8ee1f48b7b6b. NOT synchronized/pushed.
Next: user completes GitHub authentication, then retry git push origin main
and verify remote SHA. Baseline tag remains local (not pushed). Existing
packed-refs.lock maintenance warning did not prevent commit; left untouched.
No UART, target board or instrument operations. Unrelated files preserved.

## 2026-10-25: Stage5 publication checkpoint

User authorizes commit/push. PC-02 TripleC, branch main; fetched origin and
confirmed remote main is an ancestor (no force push needed). Scoped publication
contains NativeShakeProgressTest source, ST timer sources/licenses, reproducible
build settings, host/HIL scripts and actual evidence, plus related handoff/HIL
records. Existing FeedbackTest/TimeManagementTest untracked work and conflicted
HIL copy are excluded and preserved. Earlier local baseline/history will also
be published by the normal main push. Final push result is verified against
remote HEAD and reported in chat; this entry records publication scope.
No firmware edits or new physical tests during publication; prior validated
image2ae93c94... and off/debug/UART-closed state are last observed, not freshly
measured. No instruments operated. Next steps remain prototype robustness,
stack/fault/race/power validation, not additional functionality.

## 2026-10-25: Stage5 final validation complete (PC-02 TripleC)

Final image2ae93c94b3038495a94f9af0101a858ac326cdcc8427ec7c3f88d162f5828ed2.
See [stage5 final report](../FW/NativeShakeProgressTest/VALIDATION.md) for exact
evidence, resource analysis, historical failures and remaining limits. This
completed entry supersedes the in-progress status below, preserved as history.
Bottle cohort: intentional shakes10/10, normal pickup false displays0/10,
slow tilt false displays0/3; continuous shaking one non-extended display.
User confirms final0..8 LED ordering/brightness/zero double flash correct.
Flash/verify/readback/UART/parser, RTC valid ordinary-reset retention,
run10/60, RF awake/STOP, sequential motion+RF, off/restart and cleanup PASS.
Flash23996/staticRAM3256; minstack1024/heap0, peak stack NOT_MEASURED.
Final board off/debug, sources/display/PWM/sampling off, RTC valid, progress7;
UART closed. User2.8V supply unchanged; no instruments or power measurement.
Branch main, baseline820ecb3f2b9fb199ddf5cf95246698d4648b73b1/tag
baseline/native-hal-eventloop-v1 retained. Stage5 only saved locally, no new
commit/push; unrelated dirty files and conflicted HIL file preserved.
Next: review prototype across bottle/fill/mount variants, clipping and stack;
fault/race stress, exact overlapping events and power remain unverified.
Earlier power loss invalidated RTC markers; final explicit PC UTC/reset PASS
does not establish power-off retention. Historical RF root cause remains OPEN.

2026-10-25 PC-02 supply-restored follow-up: user2.8V/STLINK2.73V, same serial/
COM5;2ae93c94... flash/verify/readback and controls-sustain02 PASS. Now mounted
to bottle in intended product orientation (user confirmed), axes not measured.
RTC markers/timeValid actually0 after supply interruption, not a valid default;
final UTC/reset retest pending. Bottle pickup90s debug test ongoing, UART held
only by bounded helper/finally off-close. No instruments or new commit/push.

2026-10-25 PC-02 latest stage5: native3256staticRAM/23996Flash2ae93c94... compiled,
not flashed: STLINK0.00V/no target on controls-sustain attempt; awaiting user
supply/VTref confirmation, no instruments operated. Last verified16272d9c...
off/debug/UARTclosed; live MCU state unverified. auto05 RTC/LED/run10/60 PASS,
normal pickup first4 falsely light (user confirmed), recognition FAIL. New20ms
direction dwell/600ms span rejects old truncated prefix only, fresh physical
positives/negatives/RF/mixed pending. All stage5 local/uncommitted; baseline
820ecb3/tag preserved/no push. Canonical handoff and operator notes have evidence.

2026-10-25 PC-02/TripleC stage5 resumed after usage limit: main820ecb3 and
annotated native HAL baseline saved/no push. Independent NativeShakeProgressTest
uncommitted; GCC14.2.1/HAL1.10.7 build repeated b2ad066f...23888Flash/3240staticRAM.
Peakstack unmeasured, min1024/heap0. Fixed RTC elapsed backward snapshot bug;
auto04 in progress after user re-confirms2.8V/COM5/STLINK/reset authorization.
Current measured STLINK2.73V, same serial56FF6C066688555210351387, ID425/RevX.
No instruments/power changes. Bottle-relative sensor axes user says unconfirmed.
COM5 temporarily owned by bounded bench; finally off/debug/closed. See handoff.

2026-10-25 PC-02/TripleC native B recovery capture: user authorizes scoped commit
and annotated baseline/native-hal-eventloop-v1 only, no push. Preserve unrelated
FeedbackTest/stage3 work; vendor byte hashes protected against line conversion.
Resolve baseline commit through tag; build instructions in native BASELINE.md.
No board/UART/instrument/power operations in capture. Stage5 follows only after
baseline save, no further commit/push. Existing source/HIL results unchanged.

2026-10-25 PC-02/TripleC NativeEventLoopStopTest: main fdacaf6/cached ahead3,
local only/no fetch/commit/push. GCC14.2.1-1.1/newlibnano4.4, HAL1.10.7/CMSIS
Device1.9.4/Core6.1(distribution6.3) vendored+license+hash checked; no Arduino
link. Python CLI build (CMake/Ninja absentPATH) reproducible twice e7e4e403...,
ELF/vector/sharedhostlogic PASS. A re-created31480/2860; B18164/1352 (-42.30%/
52.73%). Same diagnostics; fixed-board wrappers/timing differ, no slimC.
STLINK56FF6C066688555210351387/ID425/32KB/Cube2.11.0/SWD4MHz flash/verify/readback
PASS. COM5Prolific115200 baseline10/60 twice,off/re-run1/parser, motion26,
awakeRF1,STOPRF2,mixedmotion31/GPO1 PASS; backup/prescaler comparisonPASS(all0
marks, valid/nonzero retention pending). Current board nativeB off/debug,
UARTclosed/no helper; manual2.8V unchanged, instruments/COM6/7 not opened.
Sources/ports are dated current checks, no copy of other PC readiness. Source,
build-report/README/HIL and canonical handoff saved; no formal migration yet.

2026-10-25 PC-02/TripleC RF historical regression: main fdacaf6/cached ahead3,
local only/no fetch/commit/push. Existing STM32duino3.0.0/GCC14.2.1 tools verified.
Unchanged historical11e2f6e build/flash/verify PASS; threeRF windows PASS. Exact
old binary/tool manifest unavailable. Restored integrated a5cd6df4... unchanged:
awake RF45s PASS;STOP RF60s PASS2;baseline10/60/restart PASS;motion30 PASS27;
mixed60 PASSmotion14/GPO2,STOP75/no error. COM5Prolific115200 fullRX, STLINK
56FF6C066688555210351387/ID425/32KB/Cube2.11.0/SWD4MHz. Initial original-STOP
HOTPLUG tool failure recovered via under-reset flash, not RF failure. Host PASS.
Firmware Flash31480/staticRAM2860 unchanged; no source fix justified, prior
failure root cause OPEN. Supply user-controlled2.8V unchanged, no instruments/
COM6/7 opened, final off/debug/UARTclosed/no helper. Handoff/report/HIL saved.
See FW/EventLoopStopTest/RF_REGRESSION.md for differences, evidence and next step.

2026-10-25 PC-02 / TripleC EventLoopStopTest: main fdacaf6, saved locally only,
no commit/push. STM32duino3.0.0/GCC14.2.1/LTO build and host checks PASS.
Actual image31480B (Arduino31260), staticRAM2860B, Flash spare1288B; stack/heap
peaks unknown. STLINK56FF6C066688555210351387/ID425/32KB/Cube2.11.0 SWD4MHz
flash/verify/final readback PASS. COM5 Prolific115200 baseline10/60/off/re-run1
and final motion20 PASS; MCU EXTI route inspection PASS only. RF/mixed failed
to observe GPO IRQ; debug FIELD1 is not wake evidence. See canonical handoff
and FW/EventLoopStopTest/tests/results. User supply2.8V unchanged; no PPK2 or
other instruments opened, final off/debug, UART closed, no helper running.
RTC timeValid0 correctly reported; no calendar/backup reset or calibration.
Next resolve RF physical wake path and Flash headroom, then RF/mixed retest.

2026-10-25 PC-02 / TripleC TimeManagementTest: main fdacaf6, no commit/push;
prior FeedbackTest preserved. Existing STM32duino3.0.0/GCC14.2.1/LTO build PASS:
ArduinoFlash21220B/actualimage21440B, staticRAM2880B, minheap/stack1536B, peak
unmeasured. Host datetime2400 vectors/7200 constexpr assertions+boundary PASS.
Fresh STLINK56FF6C066688555210351387/ID0x425/32KB, Cube2.11.0/SWD4MHz flash/
verify/ordinaryreset PASS; COM5Prolific115200 RX66/71 lines, LSE/RTC/ticking/
sim/real-date/context/reset/STOP10+60/UARTrecovery PASS. Full end-register
probe PASS; backup domain never reset, BKP2/3 used without core index1 conflict.
User supplies2.8V and authorizes MCUreset; no PPK2/instruments/COM6/7 opened.
Supply unchanged, UART closed, no helper left running. RTC restored to PC
Windows UTC2026-10-24T18:47:07Z/+08 local; W32Time not running, NTP accuracy
unverified. Power-loss retention/drift/fault injection/current not tested.
New folder and HIL/handoff/profile saved locally; see canonical handoff/README.

2026-10-25 PC-02 / TripleC FeedbackTest: main fdacaf6, no commit/push. Same
STM32duino3.0.0/GCC14.2.1/LTO build PASS; Arduino Flash17012B, actual image17232B,
staticRAM2184B, minimum heap/stack reserve1536B (not peak). constexpr checks PASS.
Fresh STLINK56FF6C066688555210351387/0x425/32KB, Cube2.11.0 SWD4MHz flash/verify
PASS. ProlificCOM5/115200 bench01 553 and bench02 539 actual UART replies PASS;
visual01 accepted by user (red, sound counts/sync/tone/volume). New FeedbackTest
and relevant handoff/HIL files saved locally. User authorized PPK2 supply-only
COM7/HW1518/2800mV and confirmed sole source/battery removal/common GND; every
run sent OFF, ports closed, no helper remains. No current capture or independent
OFF voltage measurement. Future blue/Piezo TIM2_CH1 arbitration remains required.

2026-10-25 PC-02 / TripleC publication: local work committed in `b3adc7b`;
remote updates through `c8dd924` integrated by `d6d9f2b`. Push is blocked by
destination approval: explicitly confirm https://github.com/TripleC-Light/Pebo.
No instrument, UART or power state changed in this Git session.

Last updated: 2026-10-25 (Asia/Taipei).

Use this file for per-computer environment state and `handoff.md` for the
current task and session history. These are dated observations, not live status.

User preference (2026-10-14): ignore MCP Bridge in future routine development
and machine-readiness checks unless explicitly requested. Its historical
verification below is retained, but it is not a setup requirement or blocker.
At each session, match the Windows computer name before updating a profile.
Do not assume matching Google Drive paths mean matching tools or Git state.

## Three-computer inventory

| ID | Computer name | Identity evidence | Last checked | Readiness |
| --- | --- | --- | --- | --- |
| PC-01 | TRIPLEC_AERO | Observed computer name; user tripl | 2026-10-14 | Local firmware builds and software tests PASS; hardware not connected |
| PC-02 | TripleC | Observed computer name; sandbox user triplec/codexsandboxoffline | 2026-10-14 | Toolchain present; firmware builds not rerun in this check; hardware not verified |
| PC-03 | Edward-PC | User identifies historical echou computer | 2026-10-13 historical session | Battery build/HIL evidence recorded; current setup requires recheck |

The user provided TripleC and Edward-PC names on 2026-10-14 and confirmed
that the historical echou development computer is Edward-PC. Purpose labels
are intentionally omitted at the user's request. Verify the actual computer
name on arrival; do not infer current readiness from its historical profile.

## PC-01: TRIPLEC_AERO

| Field | Observed state |
| --- | --- |
| Windows user | tripl |
| OS version | Windows NT 10.0.26200.0 |
| Workspace | G:/我的雲端硬碟/Application/Project/Pebo |
| Git remote | https://github.com/TripleC-Light/Pebo.git |
| Branch | main; status reports main...origin/main against cached remote ref |
| Local HEAD | 11e2f6e8954aa45e373c615d853d323aac053979 |
| Remote freshness | Not verified; last fetch did not complete |
| Arduino CLI | nightly-20260521, commit dd407d4 |
| Shared CLI path | G:/我的雲端硬碟/Application/Arduino-cli/arduino-cli.exe |
| IDE bundled CLI | C:/Program Files/Arduino IDE/resources/app/lib/backend/resources/arduino-cli.exe; 1.4.1, e39419312, version query PASS |
| Default package directory | C:/Users/tripl/AppData/Local/Arduino15/packages |
| Installed cores | arduino:avr 1.8.6; arduino:sam 1.6.12; esp32:esp32 2.0.11; STMicroelectronics:stm32 3.0.0 |
| Required STM32 core | 3.0.0 installed; local compile/link PASS |
| ARM compiler | xPack GCC 14.2.1-1.1; version query and actual builds PASS |
| Git | 2.53.0.windows.3; local status/version PASS; remote fetch still unverified |
| CubeProgrammer | v2.11.0, version query PASS |
| CubeProgrammer executable | C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe |
| PATH discovery | arduino-cli and STM32_Programmer_CLI not found by Get-Command; use verified absolute paths |
| Python/dependencies | Python 3.9.10 for PPK2; Python 3.10.11 for MCP; machine-local environments below |
| ST-LINK / UART / PPK2 / scope | User reports no connected hardware; physical tests SKIPPED |
| Codex executor | Default sandbox fails with helper_unknown_error: setup refresh had errors; outside-sandbox commands work |
| Local LED test | Remote branch exists; not synchronized or built locally |

Preserved local work at inspection:

- Modified: README.md, docs/hil-test-plan.md, docs/power-budget.md.
- Untracked: FW/BatteryVoltageTest/, Software/PPK2Automation/, docs/handoff.md.
- This registry and new handoff rules are additional session changes.
- No commit or push was performed for these local changes.

Earlier STM32 installation/download was interrupted. The user subsequently
completed installation; core list and three actual local builds now verify it.
No download, build, MCP server or instrument session was left running.

Next: synchronize the LED test without overwriting local work, then build it
locally. Physical testing requires fresh instrument discovery when connected.

### PC-01 software verification, 2026-10-14

| Check | Result |
| --- | --- |
| BatteryVoltageTest Debug ON, LTO | PASS; Flash 13856 / 32768 bytes; static RAM 1780 / 8192 bytes |
| BatteryVoltageTest Debug OFF, LTO | PASS; Flash 13692 / 32768 bytes; static RAM 1780 / 8192 bytes |
| HardwareTest, LTO | PASS; Flash 32148 / 32768 bytes; static RAM 2180 / 8192 bytes |
| PPK2Automation | 13 mock regressions PASS; CLI --help PASS; pip check PASS |
| PeboBridge | 6 regressions PASS; actual README read PASS; MCP stdio initialize/list_tools/get_project_status PASS; pip check PASS |
| Python source/Tcl | Five tool files parse successfully; Tcl 8.6.12 loads without a GUI window |
| CubeProgrammer | Version query PASS only; hardware programming not tested |

Build FQBN:
`STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX,xserial=generic,usb=none,opt=oslto,dbg=none,rtlib=nano,upload_method=swdMethod`.
Debug OFF adds `--build-property build.extra_flags=-DPEBO_BATTERY_DEBUG=0`.
ASCII build directories under `C:/Users/tripl/AppData/Local/Temp/`:
`Pebo-ToolsCheck-Battery-on`, `Pebo-ToolsCheck-Battery-off`,
`Pebo-ToolsCheck-HardwareTest`. These artifacts are local and must be rebuilt
on other computers. RAM figures do not measure peak stack/heap.

PPK2 Python executable (verified):
`C:/Users/tripl/AppData/Local/Pebo/venv-py39/Scripts/python.exe`.
Dependencies: pyserial 3.5, ppk2-api 0.9.2. The system Python did not contain
ppk2-api; use this environment for scripts/tests. Codex's Windows package may
redirect this LocalAppData path internally; do not copy its venv to another PC.

MCP Python executable (verified Store-Python actual location):
`C:/Users/tripl/AppData/Local/Packages/PythonSoftwareFoundation.Python.3.10_qbz5n2kfra8p0/LocalCache/Local/Pebo/venv-mcp-py310/Scripts/python.exe`.
Python 3.10.11; mcp 1.30.0. The shorter requested LocalAppData path did not
resolve for this environment. Unbounded requirements originally installed
mcp 2.3.0, whose own runtime error reports FastMCP API removal. Requirements
now use `mcp>=1,<2`; no bridge source/API migration was needed.
Launch from repo root with this interpreter and `Software/PeboBridge/server.py`.
Tests initialized the actual stdio protocol and closed the child process.

Arduino IDE GUI, CubeProgrammer GUI, physical COM/USB drivers and hardware
operations were not exercised. No board flashed, supplied or reset.

## PC-02: TripleC

### Full Git publication requested, 2026-10-25

User authorizes all uncommitted project files commit/push. Fetch now succeeds;
origin/main c8dd924 is5 commits ahead of local11e2f6e, documentation changes.
Save local source/docs/evidence then merge remote preserving history; normal push,
no force. Existing ignored caches/build/rawbin artifacts stay local. No device,
serial or supply operations in this step. Completion recorded in canonical handoff.

### UART manual-power retest PASS, 2026-10-25

User corrected UART to COM5, freshly verified Prolific067B:2303. NordicCOM6/7
not opened; no PPK/instrument/reset/flash operations. Manual user supply unchanged.
Initial help->ERR too_long retained as failed attempt; LF boundary resync then
7/7 exact real PC UART replies PASS(help,8,off,showCRLF,invalid,overlong,off).
Previous PC RX blocker now resolved; root cause not proven. Evidence new firmware
tests/results/uart-manual-power-20261025-01 and -02/session.json.
Serial closed, LEDs off/level8 retained, no helper running. TripleC/main11e2f6e,
prior work preserved; documentation/evidence saved locally, no commit/push/fetch.

### UART-only retest / port clarification pending, 2026-10-25

User now controls supply; no PPK access/control allowed for this retest.
Fresh inventory: COM5 Prolific067B:2303; COM6 Nordic1915:C00A MI03 and
COM7 same device MI01, both serialDC01B057091B. User identifies UART asCOM6;
bridge role/physical PA9/PA10 wiring not verified. Requested clarification and
manual-power readiness; no ports opened/commands sent, no reset/flash/helper.
No new UART result. TripleC/main11e2f6e, prior work preserved; only handoff/
registry saved locally, uncommitted/unpushed. Next: verify interface, then UART
only help/off/show without PPK operations. Supply remains user-controlled.

### Blue LED progress test, 2026-10-25

TripleC/main11e2f6e, existing dirty work preserved; no commit/push/fetch/cloud claim.
Standalone BlueLedProgressTest build PASS STM32duino3.0.0/ARM GCC14.2.1-1.1/LTO:
17160B Flash/2352B static RAM; compiler constexpr parser/state/mapping tests PASS.
Pin AF verified locally: PA5/8 TIM2_CH1,PA6 TIM22_CH1,PA7 TIM22_CH2. Uses hardware
PWM25%/1000Hz for sole low LED,100% GPIO for high; TIM21 unused. User visually
accepted physical PA5->PA6->PA7->PA8 fill order/low-high/zero double flash.
User authorized PPK supply and confirmed sole supply/cell removed/GND. Current
PPK control COM7/HW1518, UART ProlificCOM5, STLINK56FF6C066688555210351387;
2.8V setpoint, target0x425/32KB, SWD4MHz flash/verify PASS. No current measurement.
PC UART RX remains unresolved despite incoming commands and correct MCU TX ring;
run05 via SWD verifies GPIO/duty/timer-off plus42 replies, classified PARTIAL.
Full UART round-trip not passed. Results/images in new firmware tests/results.
Every session sent PPK OFF/closed ports; physical OFF VDD unmeasured. No helper
left running at final check. Next: adapter/RX path check and normal bench rerun.
Handoff/pinmap/HIL notes saved locally, uncommitted/unpushed. See canonical handoff.

### Status source fix and Minimal draft, 2026-10-25

TripleC/main11e2f6e, existing dirty work preserved; no fetch/commit/push/cloud claim.
New pure Status formatter fixes literal-percent String.format crash;4 new JVM
regressions.62 tests0 failures/errors/skips and assembleDebug PASS2m51s/40 tasks.
Evidence/current25-file manifest in Android handoff/evidence/pc02-20261025-status-fix;
source matches ASCII local copy; only original Activity changed plus2 new files.
Wire/models/Gradle/dependencies/firmware unchanged. Original archive retained.
JBR25.0.3, SDK37.0/BT36.0.0, Gradle9.3.1. Existing PKIX issue resolved locally
using isolated JBR cacerts copy plus existing Windows AVG public root at parent
build-trust/cacerts; process-only JAVA_TOOL_OPTIONS. No global trust/env changes.
Local root C:/Users/Edward/AppData/Local/Pebo/Android-PC02-20261016 retained.
Sony authorized; installed and local APK signers differ, so no install/data clear;
phone still has originalv3, no physical fixed-UI PASS. No configured instrumentation
runner; emulator accel-check says hypervisor driver absent, not installed/launched.
Added independent Minimal v0.2 draft/decision table, all new protocol rules proposed.
Resource estimates/core304B static/config224B/totalEEPROM352B not MCU validation.
Final process query: adb PID24208 only, no java/studio64; build daemon exited.
Notes/evidence saved locally, uncommitted/unpushed. Next: compatible-signature UI
validation and Minimal P0 review. No new NFC/firmware/power/instrument operations.

### v0.2 documentation progress audit, 2026-10-25

Original attached requirements checked against existing v0.2 draft/decision table:
requested19 sections and additional resource/v0.1/validation sections present.
Draft authored; all review-record decisions OPEN/unassigned, not approved or
implemented.1152B persistent proposal exceeds1KB estimate; product/history,
resource, atomic/retry/auth/wake decisions required before implementation.
Prior Status UI crash is newer than draft comparison. No code/spec changes,
build/phone/instrument operations or fresh process query. TripleC/main11e2f6e,
existing work preserved; handoff/registry saved locally, no commit/push/fetch.

### Physical UI/NFC test, 2026-10-25

Sony XQ-DQ72/Android15/v3: Main launch, Sample labels/DeviceInfo PASS;
Read Status crashes reproducibly (UnknownFormatConversionException at
ProtocolActivity.kt188, literal percentage in formatted string). No source fix/install.
User-assisted NfcV block0 PASS: RX00 00 00 00 00 -> data00 00 00 00.
Evidence pc02-20261025-ui-nfc under Android handoff/evidence. Full UI and extended
RF regressions pending; real MCU protocol unbound. No writes/firmware/power work.
Gradle PKIX/signing update concerns remain. main11e2f6e, existing work preserved;
notes/evidence saved locally, no commit/push/fetch. ADB available. See canonical handoff.

### Phone development connection, 2026-10-25

Exactly one authorized Sony XQ-DQ72 via local SDK adb37.0.1; Android15/API35,
USB debugging enabled and shell/package queries PASS. Pebo v3/0.1-protocol-sample
installed/debuggable for user0; MainActivity resolves, NFC permission granted,
hardware present/NFC on. Screen OFF_LOCKED; UI/RF not tested. JBR25.0.3 rechecked.
Phone connection ready; prior Gradle PKIX blocker remains, no new build attempted.
APK signing compatibility not checked; no install/clear-data/launch/NFC/MCU/power
operations. adb PID24208 available, no java/studio64 observed. main11e2f6e,
existing work preserved; only handoff/registry updated locally, no commit/push/fetch.
Next: unlock for UI/RF tests, fix trusted HTTPS/build, compare signer before update.

### Android recheck, 2026-10-25

Hostname TripleC/profile Edward; Git2.46.2.windows.1, main11e2f6e8954aa45e373c615d853d323aac053979.
Existing changes preserved, no fetch/commit/push/cloud verification. All23 source
hashes/lengths and archive hash match; existing local copy also matches manifest.
Studio JBR25.0.3, SDK37.0/BuildTools36.0.0, adb37.0.1 verified; PATH Java is
Oracle Java8. Use explicit JBR and process-only settings for build.
Local copy: C:/Users/Edward/AppData/Local/Pebo/Android-PC02-20261016/PeboAndroid;
sibling gradle-cache/local SDK properties. Reconcile edits with shared saved source.
Fresh build blocked before tests by Gradle9.3.1 HTTPS PKIX certificate error;
log in parent/build-20261025.log. No PC-02 test/APK PASS, TLS bypass or global
environment/certificate/license changes. Next: trusted HTTPS fix, 58 tests/build,
then authorized-phone signer/UI/RF checks. v0.2 remains an unadopted draft.
adb query: no devices; server available(PID24208 observed), no java/studio64.
No emulator/install/MCU/power/instrument operations; physical tests skipped.
Default shell sandbox initialization fails; approved escalated commands work.
Handoff/registry saved locally, uncommitted/unpushed; canonical details in
docs/handoff.md 2026-10-25 entry. Earlier observations below remain historical.

Observed on 2026-10-14 as the active development computer for the next work
session. This is a setup/readiness check only; no firmware build, flash, reset,
power command or instrument command was issued.

| Field | Observed state |
| --- | --- |
| Windows computer name | TRIPLEC |
| Observed user context | triplec/codexsandboxoffline |
| Workspace | G:/Google Drive/Application/Project/Pebo |
| Git remote | https://github.com/TripleC-Light/Pebo.git |
| Branch | main tracking origin/main |
| Local HEAD | 11e2f6e8954aa45e373c615d853d323aac053979 |
| Git status | Local handoff/BatteryVoltageTest/PPK2Automation changes present and preserved |
| Git version | 2.46.2.windows.1 |
| Arduino CLI | nightly-20260521, commit dd407d4 |
| Shared CLI path | G:/Google Drive/Application/Arduino-cli/arduino-cli.exe |
| Installed STM32 core | STMicroelectronics:stm32 3.0.0 |
| CubeProgrammer | v2.11.0, version query PASS |
| Python on PATH | C:/Users/Edward/AppData/Local/Programs/Python/Python312/python.exe |
| Hardware | Not checked in this session |
| LedBrightnessTest build | 2026-10-14 PASS; Debug ON 15056 flash / 1900 RAM; Debug OFF 12200 flash / 1212 RAM |
| LedBrightnessTest upload | 2026-10-14 PASS with user-controlled PPK2 GUI power and with Python PPK2 API power; CLI target voltage 2.71..2.73 V; visual brightness PASS; COM8 UART log PASS |
| PiezoLIS2InteractionTest build | 2026-10-14 PASS; Flash 25500 / 32768 bytes; static RAM 2160 / 8192 bytes |
| Piezo current automation tests | 2026-10-14 PASS; Python syntax and 12 unittest regressions; pytest not installed on PATH Python |
| Piezo HIL | 2026-10-14 PARTIAL/FAIL; fixed one-beep PWM re-arm bug, post-fix 9 audible beeps and 9/9 UART events, but LIS2 false-trigger/current sanity acceptance failed |

Current local changes at inspection:

- Modified: AGENTS.md, README.md, Software/PeboBridge/README.md,
  Software/PeboBridge/requirements.txt, docs/hil-test-plan.md,
  docs/power-budget.md.
- Untracked: FW/BatteryVoltageTest/, Software/PPK2Automation/,
  docs/development-machines.md, docs/handoff.md.

Routine work should continue to ignore MCP Bridge unless explicitly requested.
Before flashing or measuring, rediscover board power, ST-LINK, UART, PPK2 and
scope connections on this computer.

### PC-02 piezo automation update, 2026-10-14

Added the standalone PiezoLIS2InteractionTest firmware and PPK2 current
automation. The build used the shared Arduino CLI at
`G:/Google Drive/Application/Arduino-cli/arduino-cli.exe`, STM32duino 3.0.0,
Generic L031G6Ux, LTO, and ASCII build path
`C:/tmp/Pebo-PiezoLIS2InteractionTest-debug-on`. Build output:
25500 bytes flash, 2160 bytes static RAM.

Software checks on PATH Python 3.12:

- `python -m py_compile Software/PPK2Automation/piezo_current_test.py` PASS.
- `python -m unittest Software/PPK2Automation/tests/test_piezo_current_test.py -v`
  PASS, 12 tests.
- `python -m pytest ...` could not run because pytest is not installed; the
  existing PPK2Automation test style is unittest and no pytest dependency was
  added.

No HIL was performed for the piezo test on this update. No PPK2 supply,
acquisition, flash, target reset or UART hardware capture was issued. Current
COM ports and wiring remain to be rediscovered before physical testing.

Later the same day, HIL started on TRIPLEC. Current ports: PPK2 control COM7
(HW=1518), PPK2 secondary CDC COM6, Prolific USB-UART COM8. pyserial 3.5 and
ppk2-api 0.9.2 were installed into PATH Python 3.12 from the project
requirements. The initial firmware flashed and ran but the user heard only one
beep; firmware was fixed and rebuilt at 25252 bytes flash / 2160 bytes RAM.
Post-fix flash returned CubeProgrammer exit code 0. The post-fix PPK2 run at
2.8 V produced 9/9 UART events and the user confirmed 9 audible beeps, but
RESULT remained FAIL because LIS2 false triggers occurred on 6/9 events and
current sanity failed on two events. CSV:
`Software/PPK2Automation/results/piezo_current_20261014_164204.csv`.
All wrappers/scripts sent PPK2 OFF; no intentional hardware process was left
running.

## PC-03: Edward-PC

The user confirmed on 2026-10-14 that historical user `echou` belongs to
Edward-PC. The following evidence comes from the 2026-10-13 handoff, not a
current connection to this computer:

| Field | Historical observation |
| --- | --- |
| User | echou |
| Workspace | G:/我的雲端硬碟/Application/Project/Pebo |
| Arduino CLI | nightly-20260521, dd407d4; shared G-drive path in handoff |
| STM32 toolchain | STM32duino 3.0.0, ARM GCC 14.2.1-1.1, LTO |
| CubeProgrammer | 2.11.0; historical executable path not confirmed |
| Python | 3.9.13; pyserial 3.5; ppk2-api 0.9.2 |
| Build output | ASCII temp directory under C:/Users/echou/AppData/Local/Temp |
| BatteryVoltageTest Debug ON | PASS: Flash 13856 bytes, static RAM 1780 bytes |
| BatteryVoltageTest Debug OFF | PASS: Flash 13692 bytes, static RAM 1780 bytes |
| Flash/HIL | Debug ON flash and five-point source-setpoint sweep PASS; qualifications in handoff |
| Instruments | Historical PPK2 COM10, UART COM14, ST-LINK 56FF6C066688555210351387; rediscover before use |
| Git | Session ended with uncommitted/unpushed changes; current branch/HEAD unknown |
| LED test | No local build/flash recorded for Edward-PC |
| Session end | Historical instrument ports closed and PPK2 OFF command sent |

Next visit: check current Git state, versions and ports before relying on
these observations. See handoff for raw result paths and remaining measurement
qualifications; this profile does not upgrade historical evidence to a new PASS.

## Profile fields to check on each computer

For each computer record:

- Computer name, OS/user, date checked.
- Workspace path, Git remote, branch, HEAD and synchronization status.
- Uncommitted/untracked work and where it has been preserved.
- Arduino CLI path/version, package directory, core version, compiler and
  CubeProgrammer path/version; Python and required packages when used.
- Last successful build, sketch/commit, command, Debug setting, Flash/RAM.
- Instruments actually observed: serial number, COM port, firmware,
  board revision, supply and wiring; timestamp each physical observation.
- Blockers, missing setup, next action and whether processes/supply remain on.

Historical evidence for Edward-PC is in `handoff.md`:
user-path `echou`, STM32duino 3.0.0, ARM GCC 14.2.1-1.1, CubeProgrammer 2.11.0,
Python 3.9.13, pyserial 3.5, ppk2-api 0.9.2, and BatteryVoltageTest build/HIL.
Its historical COM10/COM14 assignments must be rediscovered on every computer.

## Switching workflow

1. Before leaving a computer, update its profile and `handoff.md`: task,
   changed files, branch/commit, tests, blockers, next action and instrument state.
2. Record how work is transferred. Saved files alone are not a Git commit/push.
   Preserve uncommitted work before pulling, switching branches or merging.
3. Prefer separate local Git checkouts per computer with GitHub as the code
   transfer point. For the current shared Drive checkout, finish Drive sync
   and close Git/build activity on the previous computer before opening it
   elsewhere; do not run two computers against the same synced .git at once.
4. On arrival, identify the computer, read both documents, check git status
   and HEAD, then verify the incoming branch/commit and relevant tool versions.
5. Rediscover instruments and wiring before flash, power or measurement.
   A prior machine's successful HIL does not verify this machine's setup.
6. Update only the current machine's observations. Keep other machines' dates
   and states until they are checked there. Mark unknown items explicitly.

Canonical local handoff: `docs/handoff.md`. The LED draft branch currently has
`docs/development-handoff.md`; consolidate its useful LED/CI history into the
canonical handoff when integrating that branch, preserving both histories.
Do not maintain two competing latest-task records after integration.

## Unmapped computer observation: LAPTOP-1JNDJFQA, 2026-10-15

Computer name read directly from Windows. It differs from TRIPLEC_AERO,
TRIPLEC and Edward-PC; identity mapping remains unverified, so all three dated
profiles above are preserved. Windows 10.0.19045; workspace
G:/我的雲端硬碟/Application/Project/Pebo. Git 2.53.0.windows.3; main at
11e2f6e8954aa45e373c615d853d323aac053979, cached main...origin/main only,
no fetch/commit/push. Existing local changes preserved; full list in handoff.

Android-only environment verification:

| Item | Observed result |
| --- | --- |
| Android Studio | C:/Program Files/Android/Android Studio; AndroidStudio2026.2.1; AI-262.9437.185.2621.16467767 |
| Bundled JBR | 25.0.3; executed successfully; used for actual build |
| CLI Java | Oracle 26.0.1 through Oracle javapath; JAVA_HOME unset; global settings unchanged |
| Android SDK | C:/Users/echou/AppData/Local/Android/Sdk |
| Platforms | android-37.0 revision 2; Android 17/API 37.0, extension 22 |
| Build Tools | 36.0.0 |
| Platform Tools | 37.0.1-15733141; adb 1.0.41 works by full path, absent from PATH |
| SDK CLI management | sdkmanager not found under installed Android locations; Studio manager UI not exercised |
| Android repository project | None found; no production Android files added |
| Independent build | AGP 9.1.1 / Wrapper 9.3.1 / Kotlin 2.2.10 / JBR 25.0.3; assembleDebug PASS, 34 tasks |
| APK verification | v1/v2 signature PASS; min API 23, target 37, launcher Activity confirmed |
| Phone | adb devices -l empty; install/launch/NFC support and sensing unverified |
| MCU/instruments | Not exercised or rediscovered in this Android-only session |

Project: C:/Users/echou/AppData/Local/Temp/Pebo-Android-EnvCheck-20261015.
APK: app/build/outputs/apk/debug/app-debug.apk beneath it. Build/version logs
and README retained there. App ID com.pebo.envcheck20261015; screen contains
only the requested environment success text. Project-local Gradle cache;
no global Gradle installation, tool upgrades, SDK downloads/license acceptance
or system configuration edits. SDK XML parser and apksigner warnings were
non-blocking; details in handoff. Java 26 is outside selected Gradle 9.3.1's
supported runtime range; process-local JBR 25 build succeeded.

Session edits only docs/handoff.md and docs/development-machines.md;
saved locally, uncommitted/unpushed. No remaining java process observed;
adb server PID 53788 left running, no MCU instrument process started.
Next: connect/authorize a physical phone, rerun adb, install and launch the
independent test APK, then query NFC feature declaration separately. No NFC
sensing implementation or test was performed. See canonical handoff for
checksum, command logs, cautions and preserved work.
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
### PC-01 Android handover verification, 2026-10-15

Current hostname TripleC_AERO/user tripl verified. main at
11e2f6e8954aa45e373c615d853d323aac053979; Git2.53.0.windows.3;
existing modified/untracked work retained. No fetch/commit/push.
Read current Android handoff (historical source computer LAPTOP-1JNDJFQA).
Original C:/Users/echou/AppData/Local/Temp/Pebo-NfcProbe-20261015 absent on
this PC. No Android Kotlin/APK/Gradle wrapper found in shared Project tree;
no local Pebo Android TEMP project. Source transfer requested; no rebuild
or duplicate implementation attempted. Suggested destination Software/PeboAndroid
is pending, not an existing verified source location.

Android Studio installed at C:/Program Files/Android/Android Studio.
Bundled JBR version query PASS:25.0.3 (25.0.3+-15898627-b508.16).
SDK C:/Users/tripl/AppData/Local/Android/Sdk contains android-37.0 and
Build Tools36.0.0 directories. adb version query PASS:1.0.41/package37.0.1.
java/adb/gradle not found on PATH; JAVA_HOME/ANDROID_HOME/ANDROID_SDK_ROOT
not observed; C:/Users/tripl/.gradle absent. No global settings changed.
Build/runtime readiness is NOT established by these version/path checks.

Android build/tests and phone/HIL checks SKIPPED:source unavailable; phone
connections not queried. No firmware/instrument/power/NFC/install commands.
Last process check saw no adb/java/studio64; no background work started.
Only docs/handoff.md and this registry appended; saved locally, no commit/push
or verified cloud replication. Next:transfer complete existing Android source,
inspect configuration, run 58 historical regressions plus debug build, then
Sample UI/engineering regressions as documented. See canonical handoff.

### PC-01 Android environment smoke tests, 2026-10-15

User requested explicit environment verification. Studio build
AI-262.9437.185.2621.16467767 (AndroidStudio2026.2.1); bundled Java/javac25.0.3
PASS. SDK platform37.0 revision2/BuildTools36.0.0 metadata verified.
aapt2 2.20-13193326, apksigner0.9, D8 8.10.9-dev version queries PASS.
Standalone Java compile/run PASS; explicit --release17 -> D8 --min-api26
with installed android.jar generated1368-byte classes.dex PASS. Default25
bytecode first failed D8 major69; target17 resolved diagnostic, no install
changes. Full Pebo APK/Gradle/Kotlin compatibility still unverified.

adb1.0.41/package37.0.1 device query found one unauthorized phone; server
started and left available. No install/device data access/phone tests.
Emulator executable/Medium_Phone_API_37.0 AVD .ini present, not launched.
cmdline-tools absent; sdkmanager/Java/adb/Gradle not on PATH. License file
exists; no licenses or persistent environment changed. JAVA_HOME was set only
inside tool subprocess. Gradle wrapper/source/default .gradle cache absent;
no Gradle distribution/dependency readiness claimed. Original project required
for full build and historical58-test rerun. No global Gradle installed.

main11e2f6e8954aa45e373c615d853d323aac053979; only handoff/registry appended,
existing work preserved; saved locally,no commit/push/fetch. No firmware or
instrument operations; no emulator/build daemon launched. Smoke artifacts in
allowed visualization directory; canonical handoff contains path and details.

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

### PC-01 Android transfer arrival verified, 2026-10-15

TripleC_AERO/main11e2f6e8954aa45e373c615d853d323aac053979. Source now present
at Software/PeboAndroid; prior missing-source blocker resolved. ZIP/sidecar
and supplied APK SHA256 match canonical handoff, all23 source-manifest files
match hash AND byte length on this PC. App versionCode3, Wrapper9.3.1 and
explicit Java/Kotlin target17 confirmed. Read portable README and protocol spec.
PC-01 full Android build/58 tests still pending; source-machine portable restore
PASS is not destination readiness. Next: ASCII build copy/local SDK/JBR,
Wrapper build/tests, then authorized-phone Sample UI checks. No app source,
build/download/install/device query or global setup changes this check.
adb observed from previous check, no java/studio64. Only handoff/registry
appended,saved locally,no commit/push/fetch; original work preserved.

### PC-01 full Android build/test PASS, 2026-10-15

TripleC_AERO/main11e2f6e8954aa45e373c615d853d323aac053979. New ASCII local
copy C:/Users/tripl/AppData/Local/Temp/PeboAndroid-PC01-fb4640f8/PeboAndroid;
dedicated sibling gradle-cache. Process-only Studio JBR25.0.3/Gradle cache;
local SDK37.0/BuildTools36.0.0,original Wrapper9.3.1/AGP9.1.1/dependencies.
Wrapper and dependencies downloaded successfully; no global setup/license or
source changes. testDebugUnitTest assembleDebug --no-daemon --console=plain:
PASS2m16s,40/40 tasks executed. XML58 tests(38protocol+14block+6gate),zero
failures/errors/skips. All23 source hashes unchanged after build.
APK version3/0.1-protocol-sample,916855bytes,signature v1/v2 PASS,SHA256
C2129CD776F1E06CBDA9799F61A55809BB5BCB2E399A4A05EE612EC4AD98AD4C.
New PC debug certificate differs from original supplied APK; no install attempted,
no uninstall/data clear. Canonical handoff records both public fingerprints.

Evidence/APK/XML/HTML/logs/JSON saved under Software/PeboAndroid/handoff/evidence/
pc01-20261015-build01/. Original transfer APK/ZIP intact. Source-machine
readiness is now supplemented by actual PC-01 build/test evidence. Existing
nonblocking SDKXML/native-access/META-INF warnings retained. No UI/RF/HIL,
device queries,MCU/flash/power/instruments. adb observed,no java/studio64 after
build;single-use daemon exited. Local cache retained. New evidence and these
two handoff records saved locally,uncommitted/unpushed,no fetch;prior work
preserved. Next:authorized-phone Sample UI and engineering physical regressions;
real protocol transport remains unbound. No private key included in evidence.

### PC-01 Sony version3 update PASS, 2026-10-15

User authorized phone installation. Exactly one authorized Sony XQ-DQ72;
existing Pebo version2 found. Installed base.apk public certificate verified
against original v3 handoff APK; matched e559eb3dd75ea241b28726488f7069c3abc3f93169eedba7c219f32007da0bb5.
Original supplied APK checksum verified;install -r Success,MainActivity cold
launch Status:ok1086ms. Package confirms versionCode3/0.1-protocol-sample.
PC-01 newly built APK NOT installed due different debug signer; compatible
original APK used with identical verified source. No uninstall/data clear or
private-key transfer. Sample UI/RF/HIL not tested;no NFC memory/protocol
commands. adb available,App launch succeeded,later foreground not confirmed.
No firmware/power/instrument/global settings changed;main unchanged,existing
work preserved. Handoff/registry saved locally,uncommitted/unpushed,no fetch.
Next:Sample UI smoke and engineering block regressions separately.

### PC-01 Android technical handoff prepared, 2026-10-15

Added docs/android-development-handoff.md covering user's nine report items,
local58-test/build PASS,original-signer v3 installed vs local-build signer,
source/evidence locations,firmware blockers and provisional protocol risks.
Canonical chronology remains docs/handoff.md. Original ZIP snapshot unchanged;
new report/evidence require current folder transfer. No source/build/install/
MCU/power/instrument operations this documentation step. Process/phone status
not requeried;prior daemon exit/adb available/App launch observations retained.
main11e2f6e8954aa45e373c615d853d323aac053979,prior work preserved. Report,
handoff and registry saved locally,uncommitted/unpushed,no fetch/cloud claim.
Next:Sample UI/physical regression and formal wire/transport agreement.
