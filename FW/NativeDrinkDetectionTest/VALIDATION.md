# Stage6 validation log (2026-10-25/26)

Final status: prototype implemented, build/replay/HIL evidence audit PASS.
Limited EMPTY-bottle acceptance only; expanded single-event cohort and fault/
power/stack tests remain pending below. Board off/debug/UART closed.

## Scope and versions

All gestures are EMPTY direct-drink bottle simulations, user-selected. Same
stage5 mounting; no measured XYZ-to-spout mapping, no actual water ingestion
or volume evidence. Training and independent validation are separate below.
Do not report production accuracy or pour/straw discrimination from this cohort.

Remote stage5 commit128d941f0c81c40e1cb0ad388f8817f40f65c13d is ancestor of
main a7c03e78c0a5e2c45d6b3c0e98d0c37e1e459304. Both annotated references
were verified and explicitly pushed without force:

- baseline/native-hal-eventloop-v1 ->820ecb3f2b9fb199ddf5cf95246698d4648b73b1.
- checkpoint/native-shake-progress-v1 ->128d941f0c81c40e1cb0ad388f8817f40f65c13d.

Stage5 image2ae93c94... unchanged. Stage6 source/docs/scripts/evidence only local,
NOT committed/pushed. Existing unrelated FeedbackTest/TimeManagementTest and
conflicted HIL file preserved. No instruments, option bytes or backup reset.

Stage6 training seed dca5abb3... (27924Flash/4696RAM). First frozen validation
image a17fc14b825531e0b9d2639c492f66162bf59f5a9fc0b68691c8ddd9ac54072b,
28228Flash/4704RAM, exact repeated build after interruption. Resume corrected
unreliable acceleration bridging a return timer, actual C++ regression added.
Current return-fix image09cdb9cc0d952a759e24917fd94fcdd09b01b44ad0b790af84c78a20238c3293,
same size. Previous image results are not silently relabelled as current image.

## Completed 2026-10-25 evidence

Paths relative to tests/results. A bench result PASS means acquisition/transport
checks passed, not necessarily recognition; use observed counts and operator
confirmations. Hardware ISR counts do not equal physical motions.

| Evidence | Actual result / interpretation |
| --- | --- |
| seed-flash / validation2-flash | Exact target425/STLINK verified, image flash/verify/full readback and UART PASS |
| calibration01 | Helper mistakenly ended before2200ms; FAIL retained, not a device calibration failure |
| calibration02 / calibration-validation2 | Static calibration PASS, references15,-1017,-32 /15,-1017,-33, RAM only |
| train-single01 | Operator motion reached end of15s window, incomplete capture; exclude from accuracy/tuning |
| train-single02 | Operator confirms one sip simulation. Seed wrongly SHAKE1/DRINK0; preserved failure. Dynamic-norm qualifier replay with actual C++ yields candidate1/shake0 |
| train-pickup01 | One user-confirmed normal pickup;0 candidate/0 shake |
| train-shake01 | One user-confirmed intentional shake;1 shake/0 candidate |
| train-tilt01 | One user-confirmed slow tilt; final conservative1200ms hold replay0 candidate/0 shake. Temporary800ms trial produced false candidate and was reverted |
| train-three01 | Operator subsequently says motions not correctly executed. Preserve data, EXCLUDE from miss-rate/parameter selection |
| train-three02 | Operator confirms3 simulated sips;3 candidates ->1 deferred event, output-last exactly10000ms, zero errors/drop/shake;3017 raw rows |
| validate-single-stop01 | Independent1 simulated sip, candidate1/event1, output-last10129ms; STOP count2 at candidate ->13 at output. Motion ISR/wake1; run40 deadline/debug/UART PASS, operator confirms |
| validate-pickup5a /5b | Independent5+5 pickups, operator confirms10/no LED;0 candidate/0 shake/errors, RTC deadline60/debug PASS. Motion windows3+2, not10 IRQ events |
| controls-final | Diagnostic sampling UART/invalid/overlong noninterference, mode-debug cancellation, idle register restore, off/PWM cleanup/re-run PASS |
| validate-shake5a | One preliminary shake before RUN_BEGIN plus3 formal shakes,4 total confirmations/0 drink; operator confirms. Final LED interrupted by run deadline, NOT full3s optics acceptance |

The120s debug shake test was NOT started before usage-limit interruption:
approval review failed before patch/process. No live helper remained afterward.

## 2026-10-26 resume evidence

User re-confirms2.8V, same empty bottle/mounting and STLINK; UART reconfigured.
Fresh inventory: single Prolific067B:2303 at COM13; Nordic COM6/7 not opened.
20261026-returnfix-flash PASS flash/verify/readback/COM13 and finite run1.
Fresh timeValid0 honestly reported; not a valid default calendar. Explicit PC
UTC auto regression then tests ordinary reset without backup reset. Calibration
must be re-established after that reset. Further manual results appended below.

| Final return-fix evidence | Result |
| --- | --- |
| build-final-repeat-20261026.json | Independent rebuild identical09cdb9cc..., all actual C++ boundary/vector/vendor checks PASS |
| 20261026-auto | Explicit Windows PC UTC (NTP unverified), ordinary reset keeps valid RTC/BKP0..4, calibration resets invalid; LED0..8 timeout/PWM/parser, run10/run60/off/restart, PC elapsed anchor PASS |
| 20261026-calibration | PASS, fresh upright reference21,-1013,-109, RAM only |
| 20261026-controls | Sampling UART/invalid input noninterference, mode-debug cancellation, idle register restore, PWM/off/restart PASS |
| 20261026-shake5a /shake5b | User confirms5+5 correct level5 displays/about3s off. Firmware5+5 shakes, zero drink candidates/events/errors. Debug optics cohort, NOT STOP evidence |
| 20261026-three1 | User saw prompt late, motions not correctly executed; exclude from accuracy, retain raw data |
| 20261026-three1-retry | Independent user-confirmed3 simulated sips ->3 candidates/1 event; last890699/output900699 elapsed-ms, exactly10000ms; zero shake/errors/drops;3015 raw samples and actual C++ replay PASS |
| 20261026-three2 /three3 | Two further independent user-confirmed groups:3 candidates/1 event each, exact10000ms defer, zero shake/errors/drops;3015 raw rows each; actual shared C++ replay3/0 PASS |
| 20261026-awake-rf | FIELD0 precondition; GPO ISR4/confirmed3/unknown1/errors0, wake0 as expected awake; zero shake/drink |
| 20261026-stop-rf | FIELD0 precondition; GPO ISR/wake/confirmed1, motion ISR/wake1 also observed; STOP47, RTC60/deadline/debug PASS, zero drink/shake/errors/unknownWake. Not a motion-isolated RF trial |
| 20261026-mixed | User confirms one simulated sip then phone detected tag. Motion ISR/wake1, GPO ISR/wake/confirmed1; candidate1/event1, stops9->20 during merge,10489ms defer; STOP54, RTC60/deadline/debug/UART PASS; no errors/unknownWake |
| evidence-audit.json | Automated audit of12 final-image result sets: exact image, event counts/defer, actual GPO/STOP and final cleanup invariants PASS |

Starting with three1-retry, manual progress0 double flash cues the operator;
it is explicitly logged, not a classifier-triggered display or new firmware.
Latest helper closes UART and returns off/debug after each finite test.
Final board09cdb9cc..., valid RTC, reference21,-1013,-109 retained in RAM,
progress0 (manual cue), sources/display/PWM/sampling off, COM13 closed.
User-supplied2.8V unchanged; no power or measurement instrument commands.
RF status field is cached last-serviced state, NOT fresh field proof; field
removal wake and NFC application transfer are not established. ISR counts are
not phone approach counts. Prior historical RF root cause remains OPEN.

## Resource budget

| Item | Stage5 | Stage6 return-fix |
| --- | ---: | ---: |
| Flash including data initial values |23996|28228|
| Flash spare |8772|4540|
| .text |21484|24896|
| .rodata |2204|3024|
| .data |104|104|
| .bss |3152|4600|
| Static RAM |3256|4704|
| RAM after static |4936|3488|
| Linker minimum stack / heap |1024 /0|1024 /0|
| RAM outside static+minimum reservation |3912|2464|

Trace320x8=2560B and appTX1024B dominate static RAM. Fixed samples feed both
classifiers; no duplicate large sensor buffer. Peak stack NOT_MEASURED; local
compiler stack frames are not a call-chain/ISR high-water measurement, minimum
reservation is not runtime safety proof. No live-stack painting. No power or
long-term RTC accuracy measurement. All configurations retain diagnostics;
no diagnostic-stripped image is being compared as equivalent.

## Logic and remaining acceptance

Actual C++ static assertions cover calibration magnitude/movement/preservation,
tilt/hold/return, incomplete/timeout/invalid reference, unreliable return reset,
shake exclusion/rearm, missing samples, merge before/equal/after10s, cancellation,
uint32 rollover, elapsed midnight/backward rejection/rebase, parser and LED.
Real raw replay uses exact shared Drink::coordinate/Detector/Shake, not a Python
classifier imitation. Synthetic tests are not physical recognition acceptance.

Completed final-image cohorts above include10 shakes and3 three-sip groups,
plus mixed single/STOP, RF, auto/controls. Do not pool training with validation.
Still pending:10 independent single-event simulations (only1 on final image,
plus1 on prior a17...); repeat10 pickup/place on final image (prior a17... had
user-confirmed0/10 false displays), pending-group physical off/deadline cancel,
continuous motion and ambiguous pouring. Cancellation/timeout paths have pure
logic tests and sample/off integration tests, not pending-group physical proof.
LED0..8 logical/timeout regression PASS; full stage6 nine-level optics not
independently confirmed (level5 was confirmed10 times; stage5 full optics PASS).
Real-water tests, broad installation/
fill/bottle variation, clipping tolerance, fault injection/exact IRQ races,
peak stack, power and historical RF root cause remain OPEN.

## Re-run and recovery

Build instructions in README. tests/check_evidence.py audits saved evidence;
tests/replay.py evaluates measured raw XYZ with exact C++ logic. For new HIL,
use unused result directories and explicit currently verified UART/target IDs.
Source uses no Arduino core; pins/timers/RTC ownership retained from stage5.
Return stability edge fix and matching C++ assertion are stage6 only. Earlier
false positives, failed replay trials and invalid operator attempts remain.
No stage6 commit/push. Only explicitly authorized historic checkpoint tags
were published. Handoff/HIL/machine notes saved locally with current state.
