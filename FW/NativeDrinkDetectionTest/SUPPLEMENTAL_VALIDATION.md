# Stage6 supplemental acceptance (2026-10-26)

## Publication

User subsequently explicitly authorized commit/push. Scoped supplemental
commit dbe4fe482976bb4e499bd01cee99a940acbcfc69 was pushed to origin/main
and verified with git ls-remote. Earlier local-only/no-commit notes below
describe the acceptance session before this later authorization.
Only tools,60 evidence files and related documentation were published;
no firmware/binaries, unrelated tests or conflicted HIL copy were added.
No tags or force push. JSON parsing59files, Python syntax and staged diff
checks passed; common private-key/token pattern scan had no matches.
No additional board/UART/instrument or recognition tests during publication.
Existing packed-refs.lock maintenance warning preserved; commit/push succeeded.

Session closed: user explicitly declines adding water this session. No water
was added and no actual drinking was tested. Empty-bottle supplemental cohorts
are complete; this is not closure of real-drinking or production accuracy.
Deadline cancellation automated evidence passes, but the user's final reply
only declines water and does not explicitly confirm that gesture; retain that
operator-confirmation limitation. No additional physical test after this reply.

## Frozen version

- HEAD/origin main: ebe7c06380fb9a3d59bd1441c1fe30e9cac0c1b5.
- Stage6 source: 3d6d70a9a60e5c0ea0ed8dd95b6a00ca1f72c1cf.
- Rebuilt and board-readback SHA256:
  09cdb9cc0d952a759e24917fd94fcdd09b01b44ad0b790af84c78a20238c3293.
- No firmware, threshold, backup-domain or hardware changes. No flashing,
  reset, instruments, commit, push or tags in this supplemental session so far.
- PC-02 TripleC; explicitly enumerated Prolific COM13; ST-LINK
  56FF6C066688555210351387. User supplies power; VTref readback 2.72-2.73 V.
- User confirms original direct-drink bottle/mount/direction and empty bottle.
- Fresh RAM upright calibration: (43, -1012, -95), accepted; RTC remains valid.

## Independent empty-bottle cohort (complete)

Each trial has a new 30-second RAW capture, existing manual LED1 double-flash
start cue, then 10.5 seconds extra merge observation. Trials do not overlap.
Capture-mode results are not proof of motion-triggered STOP recognition.
Previous release evidence is retained separately, not pooled into this cohort.

| Trial | Operator confirmation | Candidates | Events | Merge delay | C++ replay |
| --- | --- | --- | --- | --- | --- |
| single01 | One empty-bottle single gesture completed | 1 | 1 | 10000 ms | PASS: 3015 samples, 1 candidate, 0 shakes |
| single02 | One empty-bottle single gesture completed | 1 | 1 | 10000 ms | PASS: 3015 samples, 1 candidate, 0 shakes |
| single03 | One empty-bottle single gesture completed | 1 | 1 | 10000 ms | PASS: 3015 samples, 1 candidate, 0 shakes |
| single04-retry | One empty-bottle single gesture completed | 1 | 1 | 10000 ms | PASS: 3015 samples, 1 candidate, 0 shakes |
| single05 | One empty-bottle single gesture completed | 1 | 1 | 10000 ms | PASS: 3015 samples, 1 candidate, 0 shakes |
| single06 | One empty-bottle single gesture completed | 1 | 1 | 10000 ms | PASS: 3015 samples, 1 candidate, 0 shakes |
| single07 | One empty-bottle single gesture completed | 1 | 1 | 10000 ms | PASS: 3015 samples, 1 candidate, 0 shakes |
| single08 | One empty-bottle single gesture completed | 1 | 1 | 10000 ms | PASS: 3015 samples, 1 candidate, 0 shakes |
| single09 | One empty-bottle single gesture completed | 1 | 1 | 10000 ms | PASS: 3015 samples, 1 candidate, 0 shakes |
| single10 | One empty-bottle single gesture completed | 1 | 1 | 10000 ms | PASS: 3015 samples, 1 candidate, 0 shakes |

All valid trials: 0 reported errors/dropped samples/shakes, merge=0 at end;
off/debug, LEDs/PWM/sampling/event sources off, UART closed after each helper.
Evidence directories: evidence/20261026-supplement-single01 and single02;
raw.json contains complete streamed capture, not the decimated last trace.
State/candidate/event UART output is in result.json. Replays use actual
Drink::coordinate/Drink::Detector/Shake::Detector C++ sources.

All ten operator confirmations received: 10 gestures, 10 candidates, 10 events,
0 misses, 0 duplicate events, 0 errors in this empty-bottle capture cohort.
One invalid tool-start trial is excluded, not a miss. No real drinking claim.
Trials01/02/03 quality audit:
3015 samples each, first23/24/24ms, last29992/29993/29992ms, maximum gap12ms,
no axis at either 12-bit range endpoint; monotonic timestamps checked by replay.
single04 original trial is INVALID_TOOL_START, excluded from
valid denominator: setup motion had already started Observation; capture
returned ERR acquisition_busy_or_disabled. One candidate and one event were
observed but no complete RAW capture exists, so this is not recognition PASS
or a miss. Original result.json preserved. Helper cleanup off/debug succeeded.

Host-only bench.py fix: cancel setup Observation with mode debug, require
CAPTURE_BEGIN before WINDOW_READY/LED cue. Syntax check PASS; single04-retry
confirms CAPTURE_BEGIN precedes cue; raw/replay/operator acceptance complete.
Trials04-retry..08 also3015 samples each, maximum gap12ms, zero range endpoints.
All ten detailed replays now saved separately as replay-detailed.json:
exact compiled C++ state transitions, source/raw SHA256, 3015 samples each,
maximum sample gap12ms and zero 12-bit range endpoints across all ten.
No firmware or parameter change, so same-image evidence remains separate from
invalid tool trial. No new complete firmware regression is implied by this fix.

## Final-version empty pickup cohort (complete)

pickup5a: five manual visual cues at5-second intervals inside one30s full RAW
capture, followed by10.5s merge observation. Zero candidates/events/shakes,
errors/drops; C++ replayPASS3015, no phase transitions, maxgap12ms, no range
endpoints. Human confirms all five gestures performed. Manual LED cues are
not classifier outputs. pickup5b also completed with zero candidates/events/
shakes/errors/drops, C++ replayPASS3015, maxgap12ms and no range endpoints;
human confirms its five gestures too. Final negative cohort:10 actual pickup/
place gestures,0 candidates,0 events,0 shakes,0 errors/drops. Both full captures
and exact C++ replays preserved separately from previous release negatives.

## Reproduce supplemental helpers

Use existing Python/pyserial and documented toolchain. Explicit COM13 is this
session only; enumerate again on another machine. Every helper verifies target
identity and exact image readback, uses new results directory, ends off/debug
and closes UART. No automatic flash except explicitly selected --test flash.

- Single: --test capture --seconds 30,
  --cue --uart-port COM13 --label empty_singleNN_independent --split validation.
- Five pickup cues: same capture command with --cue-count 5; still requires
  human confirmation and candidate/event audit, not just helper PASS.
- Cancellation: --test cancel-off or cancel-debug --cue; waits at most25s
  for a real candidate, cancels, checks11s quiet then run1/restart/11s quiet,
  RTC validity/BKP unchanged, source differences and PWM/sampling cleanup.
- Deadline: --test cancel-deadline --seconds 15 --cue; candidate and explicit
  pending cancellation must occur before deadline, otherwise invalid trial.
- All commands also require --image and --results as documented by --help.
- Water capture MUST specify --bottle-state water --water-ml N and actual
  --mount/--label. N is operator-estimated initial amount, not measured intake.
- replay.py now extracts state transitions from the actual compiled constexpr
  C++ detectors via objcopy, not a Python replacement classifier. Temporary
  host replay storage is not linked into MCU firmware. Existing reports kept.

## Cancellation semantics and physical acceptance

- mode debug cancels sampling, display and pending merge, stops automatic STOP
  and RTC WUT, but retains LIS2/GPO event sources.
- off additionally stops this test's event sources; preserves progress,
  calibration and valid RTC.
- run deadline cancels unfinished pending application state before returning
  debug. A new run initializes sources and cancels old pending state.
- These existing source semantics were exercised on the unchanged target below.

| Scenario | Evidence | Result |
| --- | --- | --- |
| off | cancel-off; real candidate3940957ms;717 full RAW-prefix rows, exact C++1candidate/0shake | PASS; user confirms one gesture |
| mode debug | cancel-debug; real candidate4023285ms;746 full RAW-prefix rows, exact C++1candidate/0shake | PASS; user confirms one gesture |
| run deadline | cancel-deadline; real motion ISR/wake1, candidate4148621ms at STOPcount1, deadline15s at STOPcount9, explicit DRINK_CANCELLED | Automated checks PASS; user gesture confirmation pending |

Each:1 candidate,0 confirmed events,cancelled=1,merge=0; LEDs off,TIM2/TIM22
clock gates off, sampling off. off disables LIS2/GPO; debug/deadline retain
enabled healthy LIS2/GPO and low-power CTRL1/CTRL4=20. off CTRL1/CTRL4=00.
After cancellation observe11s, then run1 and observe11s again: no new/old
candidates or confirmed events. RTC remains valid, progresses normally and
BKP0..4 unchanged. No setutc, reset or backup writes performed.
off/debug RAW scope is complete until explicit cancellation, not30s. Maximum
gap12ms, no range endpoints. Deadline only118 decimated SAMPLE rows (23..6418ms),
NOT a100Hz complete capture and not claimed as full C++ recognition replay.
Candidate/event/STOP/cancellation evidence comes from actual UART/ISR counters,
not software event injection. All helpers end off/debug with UART closed.

## Resources and remaining work

Unchanged rebuild: Flash 28228 B; .data 104 B, .bss 4600 B, static RAM 4704 B;
minimum stack 1024 B, heap 0, peak stack NOT_MEASURED. Linker RAM total 5728 B
includes the minimum stack reservation; it is not static object RAM alone.
Host logic/vector/vendor checks PASS. No long-term accuracy or power claims.

Compiler LTO .su inspection (not runtime measurement): service448 B,
counters416 B, drinkReport336 B, clockReport224 B, finishRun200 B per-frame.
These are not a call-chain or peak bound; C library/IRQ/context contributions
still need analysis. No stack painting performed.

Host-only tooling adds explicit empty/water initial-volume/mount metadata,
finite cancel-off/cancel-debug/cancel-deadline scenarios and up to5 visual
capture cues at5-second intervals. Syntax and real pickup/cancellation runs
passed; deadline operator confirmation pending. No added MCU buffers/firmware.

OPEN: deadline operator confirmation; real-water acquisition explicitly
DEFERRED_BY_USER this session; optional confusing gestures NOT_TESTED in this
supplement. No real-water denominator, hit rate or intake-volume claim.
No firmware/parameter changes,
so full unrelated RTC10/60/RF/LED regressions are retained as historical only,
not re-executed this supplement. Current run15/restarts/RTC/STOP/PWM cancellation
checks are separately recorded. Previous three
empty-bottle three-sip groups remain historical PASS for this unchanged image.
No real drinking or water-volume inference may be claimed from current data.

## Final state and next session

Board remains NativeDrinkDetectionTest-v1, verified image SHA256
09cdb9cc0d952a759e24917fd94fcdd09b01b44ad0b790af84c78a20238c3293;
last helper explicitly off/debug, sampling/display/PWM/sources off, UART closed.
No test/helper started by this session remains running. Instrument processes
were not inspected or controlled. No commit/push/tag;
host scripts, evidence and documentation saved locally. Unrelated untracked
FeedbackTest/TimeManagementTest and conflicted HIL copy preserved.
Rebuild and original compiler stack evidence saved under
evidence/20261026-supplement-calibration/rebuild-report.json and stack-usage.su.
Next session: reconfirm power/port/mount, agree water amount and natural small
sip batch, recalibrate upright if needed, then acquire new labelled full RAW
without changing thresholds first. Do not start stage7 automatically.
