# Bench automation plan

## 2026-10-27: Stage8 interrupted WIP - hardware NOT TESTED

User requests stop for shutdown. Stage8 compact33488/diagnostic35516 exceed32KB;
no image flashed. Host constexpr TaskChecks only; integrated policies/runtime/
parser/TX/replay,synthetic,empty-event task chain,RTC/RF/STOP/mixed/PWM pending.
No acceptance tag. Baseline compact checkpoint pushed. UART cleanup could not
identify/open expected port; UART closed,current board/supply UNVERIFIED.
No PPK2/power operation. Resume from NativeDrinkTaskTest README/VALIDATION;
keep old-image HIL distinct from stage8 acceptance. Scoped WIP Git allowed.

## 2026-10-26: Resource profile publication confirmed

PC-02 TripleC/main. User authorized publication after resource-work closure.
Commit b7b9171b1987041acd2dd29430742c8b2ef071c6 contains121 scoped files: diagnostic/compact
implementation, tests, reports/evidence and related docs. Normal push to origin/main succeeded;
independent git ls-remote matched the full commit. Source manifest, firmware hashes, Python/JSON,
staged scope/common secret-marker scan PASS. Source/docs whitespace check PASS; raw tool
strings retain meaningful trailing whitespace and were excluded from that formatting check.
Existing packed-refs.lock maintenance warning occurred during commit; commit and push succeeded,
lock left untouched. No source edits, new build/HIL/UART/board/reset/instrument actions during
publication. Last validated compact/off/debug/UART-closed state and image remain unchanged.
Unrelated FeedbackTest/TimeManagementTest/conflicted HIL copy preserved, uncommitted.
Earlier local-only notes below are historical and superseded by publication. This follow-up
saves publication confirmation; no stage8 work. Next: review resource/stack risks, then separately
request stage8 scope. Canonical handoff/current-machine/HIL saved.

## 2026-10-26: Resource profile publication authorized

PC-02 TripleC/main. User explicitly authorized commit/push after resource-work closure.
Pre-publication HEAD aac139c matches freshly fetched origin/main (0 ahead/behind).
Source manifest, Python syntax/evidence JSON and final image hashes rechecked; diff check PASS.
Only NativeDrinkRecordTest resource implementation/tests/evidence/docs plus canonical
handoff/current-machine/HIL are in publication scope. No source changes or new hardware,
UART, flash/reset or instrument operation during publication. Last verified board remains
compact1086703970e17547ef91f1c39521545be3f146a3aef886d489d9fb91799614c1,
off/debug/UART closed (prior validated closure; not reconnected here). Unrelated
FeedbackTest/TimeManagementTest/conflicted HIL copy preserved. Earlier local-only notes
below describe validation-time history. Publication outcome will be recorded after push.

## 2026-10-26: Post-stage7 diagnostic / compact resource work (local only)

PC-02 TripleC, Windows/Git2.46.2.windows.1/Python3.12.10/xPack ARM GCC14.2.1.
HEAD/main aac139c32a1fa5df927ae05d053e1cad55bbb59b; remote main directly verified equal.
Baseline1001ea0 rebuilt32712B/c648aef… and actual full32KB1e911996… matched.
Authorized annotated checkpoint/native-drink-record-v1 pushed normally: tag05f3aa1a… ->1001ea0.
Annotation correction: it says FF tail, but verified baseline has56 ZERO bytes (00).
Initial FF assertion failed; shell continued to authorized tag push. Hash/source were correct;
tag not overwritten. Exact corrective evidence is resource-evidence/baseline-board/verification.json.
Stage7 prior empty-motion evidence partly predates final output fix; original scope preserved.

Same core/thresholds/32 records/four summaries/Board/startup/linker; new BuildConfig + bounded
TextFormat, conditional diagnostic parser/trace/test/dump paths, deferred bounded counters.
Explicit build --config diagnostic|compact, no silent default. Final diagnostic31096 Flash,
.data24/.bss5820/static5844; compact27684 Flash,.data24/.bss3232/static3256 (5084 Flash spare,
below~6KiB target). Heap0/minstack1024; peak NOT_MEASURED. Partial diagnostic stack chain1064B
exceeds minimum reservation; no safety proof or arbitrary free-RAM feature budget.
Independent builds reproducible, both policy/parser/records tests and621 formatter cases pass;
same15 saved empty RAW clips replay identically. Rejected compiler/parser experiments retained.

Real HIL selected STLINK56FF6C066688555210351387 and Prolific COM13 verified (067b/2303).
Diagnostic: flash/verify/readback/identity,40 isolated synthetic records/32 retained/8 overwritten,
leap-day summaries/query/parser/clear/isolation; capture5 RAW501/trace92/dropped0. Compact:
flash/verify/readback, disabled commands,run10/60 (70 STOP,unknownWake0/errors0),24 awake
queries across run deadline complete; one user-confirmed empty event ->1 record, shake
progress5~3s/no added record, off cancels candidate/no later stale event, off/debug/run retain,
ordinary MCU reset clears nonempty RAM/seq but valid RTC/BKP stays. RF awake3 confirms,
STOP3 new wakes/confirms/errors0/unknown0; user confirmed both phone windows/bottle still.
Shake harness trailing-space assertion fixed, original failure/raw log retained, reanalysis PASS;
no firmware change or repeated motion. Diagnostic/compact physical coverage differs; no blanket
physical equivalence, water acceptance, power, NFC application or runtime stack claims.

Final board COMPACT27684B SHA256:
1086703970e17547ef91f1c39521545be3f146a3aef886d489d9fb91799614c1
Full32KB read SHA256:
195f2b7ac873fb15db2349d5d038302e0d95815e90b6f322ba2b79a5ada616e8
New prefix/last-page00 and untouched older trailing pages verified (no full-chip erase).
Last off/debug, rtc/motion/GPO sources0/0, PA4 lpd1, PWM/display/sampling0, valid RTC,
records/seq0 and calibration invalid after normal reset. UART closed; finite helpers exited.
User supply/hardware/option bytes/backup domain unchanged; PPK2/other instruments not opened,
no power measurement. MCP Bridge untouched. No owned persistent process or pending operator action.

Changed FW/NativeDrinkRecordTest build.py/main/parser/Observation/LogicChecks plus BuildConfig,
TextFormat, resource host/replay/HIL helpers/evidence, README/VALIDATION and RESOURCE_REPORT /
RESOURCE_VALIDATION; canonical handoff/current-machine/HIL updated. All resource changes saved
locally only, NO resource commit/push. Existing unrelated FeedbackTest/TimeManagementTest and
conflicted HIL copy preserved. Next: review resource report; scoped stage8 design may begin only
on request, budget Flash/stack carefully and separately validate actual water later. No stage8 added.

## 2026-10-26: Supplemental evidence publication

Supplement dbe4fe482976bb4e499bd01cee99a940acbcfc69 pushed to origin/main
and independently verified after user authorization. No new physical validation
or firmware change during publication. Empty10single/10pickup and cancellation
evidence/limitations remain as recorded below; water tests deferred, deadline
gesture confirmation pending.60new evidence files published;59JSON parse,
Python syntax and diff checks PASS. No instrument operation or new tag.

## 2026-10-26: Stage6 supplemental cohort; session ended

SESSION END: user declines water. Final supplement10independent empty singles
->10candidates/10events,0miss/duplicate/error;10pickups->0candidate/0event,
all operator-confirmed and exact C++ replayed. off/debug cancellationPASS,
deadline automaticPASS with operator gesture confirmation pending. Actual
drinking/water negatives/confusing gestures DEFERRED or NOT_TESTED, no invented
denominator. Firmware frozen; no power measurement, peak stack NOT_MEASURED.
Last off/debug, COM13closed. Full conditions/evidence/limitations in
FW/NativeDrinkDetectionTest/SUPPLEMENTAL_VALIDATION.md. Do not pool historical
or incomplete capture with this cohort; no new full RF regression claimed.

Current supersedes progress below:10/10 empty singles (10candidate/10event),
10 final pickups (0candidate/0event), all operator-confirmed and C++ replayed.
off/debug cancellation real candidate then0events, RTC/BKP/PWM/source cleanup
and restartPASS. Deadline run15 cancels real candidate after STOPcount1->9,
automatedPASS, human confirmation pending. Deadline trace only118 decimated
rows, no full RAW/replay claim. Water/optional confusing motions still pending.
Board unchanged09cdb9cc...3293 off/debug COM13closed. No instrument operations.

Latest: valid singles01..10 (04-retry replaces excluded failed tool start):
each operator-confirmed1candidate/1event/10000ms, C++ replayPASS3015, zero
errors/drops/shakes. All ten maxgap12ms/no range endpoints. pickup5a zero
candidates/events, detailed C++ replayPASS3015; five gestures human-confirmed.
pickup5b zero candidates/events, detailed C++ replayPASS3015; confirmation pending.
Board off/debug, COM13 closed. Final pickups/water/cancellation still in progress.

Frozen board/readback SHA09cdb9cc0d952a759e24917fd94fcdd09b01b44ad0b790af84c78a20238c3293.
New independent empty singles01/02: user confirms each one gesture; each1
candidate/1event/10000ms merge, zeroerrors/drops/shakes;3015RAW rows each,
actual C++ replay PASS. Third also operator-confirmed1/1/10000ms, replayPASS.
Fourth original INVALID_TOOL_START, capture busy after setup motion; no complete
RAW, excluded. Host-only bench.py start-order fix verified capture accepted
before cue on fourth retry; operator-confirmed1/1/10000ms, C++ replayPASS3015.
Four valid singles complete; fifth collecting. Firstthree max samplegap12ms,
no 12-bit range endpoint saturation. Remaining cohorts not yet passed.
Fresh calibration43,-1012,-95; user-powered board, COM13, no instruments,
flash, reset, backup writes or firmware changes. Helpers clean up off/debug
and close UART. Full conditions, cancellation semantics and outstanding work:
FW/NativeDrinkDetectionTest/SUPPLEMENTAL_VALIDATION.md. Prior release PASSes
remain historical, not pooled with this supplemental acceptance.


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

2026-10-25 supply restored:2ae93c94... controls-sustain02 flash/verify/full
readback/UART/sample cancellation/invalid noninterference/off/debug/re-run PASS.
Bottle now physically mounted in intended product direction (user says);
mount+algorithm changed, separate cohort. RTC marker0/timeValid0 after supply
interruption honestly reported; final valid UTC/reset repeat pending. New90s
awake-pickup window running. No power/instrument action or new commit/push.

## 2026-10-25: Stage5 latest - pickup recognition FAIL, new revision not flashed

auto05/16272d9c PASS RTC valid reset retention, LED/UART/STOP10/60/restart and
fresh149 diagnostic samples. Physical pickup5a: user normal pickup/place,
first4 LEDs lit,4 false accepts, FAIL (fifth possibly after cutoff). Read-only
45-sample prefix preserved. New20ms sustain/600ms span prefix-replay rejection
is NOT fresh full-window physical proof. New2ae93c94... compiled23996Flash/
3256staticRAM, minstack1024/heap0/peakunmeasured. controls-sustain flashing
blocked by STLINK0.00V/no target; user asked to restore external supply. Board
last verified16272d9c... off/debug, live state now unverified, UARTclosed.
No instruments/optionbytes/backup reset. Next fresh positive/negative/tilt/
continuous/RF/mixed tests after target restored. Failed JSON stays FAIL.

## 2026-10-25: Stage5 native shake/progress, validation ongoing

Independent FW/NativeShakeProgressTest/tests/results. auto01/02 passed initial
UART/LED/RTC reset/run10/60 checks. auto03 diagnostic148 measured fresh XYZ +
replay rejected PASS and idle restore PASS; total elapsed spuriously jumped24h,
so NOT accepted cooldown evidence even though its original script said PASS.
Fixed b2ad066f... applies ES0322 bounded SSR recheck/backward rejection and PC
elapsed-anchor regression. auto04 PASS: flash/verify/readback, valid nonzero
RTC reset retention with other backup markers unchanged, native LED states/
timeouts/clocks off/GPIO low, parser, run10/60/off/restart,149 fresh diagnostic
samples and idle CTRL1..6 restore. Physical motion/RF/mixed/visual10+10 trials
still ongoing/pending; diagnostic observe is not motion/STOP wake evidence.
User2.8V supply only; no instruments/power/optionbyte/backup-domain-reset.

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

## 2026-10-25: Independent RTC / time management

FW/TimeManagementTest stage3 build/host/HIL PASS on PC-02. HAL shipped with
STM32duino3.0.0, LSE PC14/15, 127/255, WUT ck_spre. No backup reset/default
valid date; BKP2/3 schema/floor allocation avoids core index1/HID4. First boot
properly invalid until setutc. Host datetime oracle2400 vectors/7200 asserts
plus input/range/context checks PASS (constexpr evaluated by host compiler).
COM5 Prolific115200 RX66/71 lines; STLINK56FF6C066688555210351387 target0x425
flash/verify/reset PASS. Date/weekday/leap/year/zone, sim isolation/multi-day,
natural midnight once, no adjustment fake-day, invalid/overflow/partial/CRLF
PASS. Ordinary reset retains RTC and resets volatile sim/zone context.
sleep10/60 twice: RTCelapsed10/60, RTC_WUT evidence, UART restored/WUT disabled;
real wall phase may be up to1s shorter, millis explicitly excludes STOP.
End-of-test full100B RTC/register inspector PASS, no SWD during timed STOP.
Final real UTC restored from PC Windows clock (+08 context), W32Time not
running, no absolute UTC/drift claim. User manual2.8V; no PPK2/instrument
control/current capture, supply unchanged, UART closed. No complete power-off
retention, LSE/source/backup fault injection or forced early-wake test done.
Evidence under new tests/results; existing bring-up tests preserved.

## 2026-10-25: Independent red LED / piezo FeedbackTest

FW/FeedbackTest build/constexpr/flash/verify PASS on PC-02. Existing toolchain,
STLINK56FF6C066688555210351387, ProlificCOM5/115200. bench01 553 and strengthened
bench02 539 actual UART round trips PASS: all effects, sampled timings/counts,
switch/restart/off, help/status preservation, invalid/overlong/NUL/CRLF. Exact
software boundaries/rollover checked by compiler; UART timing tolerates host
latency and is not an electrical waveform measurement. SWD end states confirm
PA0/PA1/PA15 Low and TIM2 CEN/CCER/DIER off, PA4 High; visual run ends naturally.
User accepted red/sound counts/synchronization and current tone/volume. Evidence
in new tests/results; prior sketches/evidence unchanged. Future blue/Piezo
TIM2_CH1 conflicts require arbitration, not currently integrated.
PPK2 initial prohibition was superseded by explicit user supply authorization
and battery removal/sole source/common-GND confirmation. COM7/HW1518 source
2800mV only, no current measurement started. OFF sent/ports closed after all
runs; physical OFF voltage unmeasured. STOP/LIS2 interference/power deferred.

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

## 2026-10-26: Stage7 RAM record validation

FW/NativeDrinkRecordTest README/VALIDATION and evidence/ define exact per-image
scope. Authorized baseline annotated tag pushed to3d6d70a; stage7 uncommitted.
Pure compiler-evaluated C++ tests cover capacity, independent day summaries,
midnight/calendar/context/invalid-time/saturation/isolation/parser; no native
host executable run claimed. Integration image: user-confirmed empty single1/1,
three3/1,shake/no record,pending off/no record +11s/restart14s quiet. Invalid
single01/shake01 excluded/preserved. Normal UTC recovery keeps undated event;
nonempty isolation,off/debug/run retention,ordinary reset/valid RTC retention
PASS. No water or threshold tuning. PC pure-tone cue is host-only.

Final UART-buffer-only fix240->256 (worst status245B), no manual gesture rerun
or relabelling. Final40SYNTHETIC/retained32/seq9..40/overwrite8;previous-day
first/today39 totals,32pages,clear/parser/RTC/STOP PASS. Final RF awake3confirmed;
STOPdeltaISR5/wake4/confirmed4/errors0/unknown1; phone/stationary bottle confirmed,
no records. Final loaded SHA c648aef249ce4e2f1c3bc419291a5c546f3c979febcd40c35983879e5703e958;
all32KB SHA1e911996c6a277b394a72e20e36189e1d7532ef0abfbedf4f973caa088c1348b.
Flash32712/static5940(data104/bss5836),Event32B/Store1156B,minstack1024/heap0/
peakNOT_MEASURED. off/debug/PA4High/PWM+sources off,COM13closed,RTCvalid;RAM0/
calibration invalid after intentional reset. No instruments,power changes,
option bytes,backup-domain reset. Stop stage7; stage8 requires schedule and
resource planning (Flash spare56B), new motion session requires recalibration.

2026-10-26 publication follow-up: stage7 commit1001ea0e34e51ccb73634a40e177a938b0ddcde0 pushed and directly verified on origin/main after user authorization; no new physical checks during publication. Earlier local-only wording is historical.
