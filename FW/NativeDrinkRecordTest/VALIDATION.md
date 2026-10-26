# Stage7 validation — 2026-10-26, PC-02 TripleC

Stage7 implementation and bounded engineering validation completed. All water /
actual drinking acceptance remains DEFERRED_BY_USER. No classifier tuning,
scheduling, reminders or automatic progress. No commit/push of stage7.

## Baseline and image identity

Start branch main, HEAD and directly queried remote main:
274e08467371a4b357b156d5f87f519a30ed3561. Unrelated untracked FeedbackTest,
TimeManagementTest and `docs/hil-test-plan [conflicted].md` preserved.
GCC14.2.1 baseline rebuild equals actual board readback,28228 bytes:
09cdb9cc0d952a759e24917fd94fcdd09b01b44ad0b790af84c78a20238c3293.
No source/include/vendor/build/linker difference from3d6d70a to HEAD.
Annotated checkpoint/native-drink-detection-v1 created and explicitly pushed:
tag object7b27cb1aaa7cd543329f6c57f03e59f24dcfdc03, peeled target
3d6d70a9a60e5c0ea0ed8dd95b6a00ca1f72c1cf. Existing tag was absent locally and
remotely; no force or other ref push. Supplement dbe4fe4 and documents8421a48 /
274e084 were distinguished from firmware. See baseline-verification.json.

Two stage7 image versions are explicitly distinguished:

| Image | Exact SHA256 / role |
| --- | --- |
| Integration image | 9f5a869fbe3850107cb1a60564dd8f58a33c3212aede2d85d1ccc66cabfd88d1;32704 bytes; manual empty-bottle, calibration, RTC recovery, retention, nonempty isolation and nonempty reset below |
| FINAL | c648aef249ce4e2f1c3bc419291a5c546f3c979febcd40c35983879e5703e958;32712 bytes; only recordsReport temporary snprintf buffer240->256; final full synthetic/transport, awake/STOP RF and closure below |

The maximum formatted status line is245 bytes, exceeding the first240-byte
buffer. This was found by boundary analysis and fixed, not left as truncation.
No classifier, time, parser, record logic or event hook changed for this fix.
Final-buffer-fix.json documents the source delta and unchanged module hashes.
Manual gestures were NOT repeated after this output-only fix; they remain
integration-image evidence, not falsely relabelled final-image acquisitions.
Both versions independently rebuilt twice identically. Final reports and .su
are saved separately from integration reports. Final32KB full Flash readback SHA:
1e911996c6a277b394a72e20e36189e1d7532ef0abfbedf4f973caa088c1348b.
The32712-byte loaded image prefix exactly matches final build; unused56 bytes
are included only in the32KB hash, not the loaded-image SHA above.

## Host logic / resources

Actual C++17 pure modules execute as constexpr assertions in the GCC compiler
on this PC, with -Wall -Wextra -Werror -fsyntax-only. No Python imitation of the
record logic. Native host executable execution was NOT performed: no native
host C++ compiler was verified. RecordChecks.cpp has a direct host main entry.
Checks PASS for:

- Consumed Pending confirms once; duplicate/non-increasing seq rejected;
  multiple candidate count is retained independently from event count.
- 41 events/32 slots in pure tests,40-event same-day total remains40 after ring
  replacement; retained ranges/overwrite counters checked independently.
- Midnight first-completion attribution, prior-day delayed confirmation,
  natural day change, month/year boundaries, leap day and UTC-zone edge dates.
- Invalid time remains undated after validity recovery, explicit forward /
  backward time/zone/context changes reject old pending groups; summary FIFO
  capacity4 and missing-summary accounting never assign old events to today.
- UINT32 counter saturation including candidates, events, overwrite and undated;
  last sequence UINT32_MAX accepted once, then rejected; context exhaustion
  blocks recording, invalid timezone rejected;32-byte aligned event layout.
- Independent synthetic instance/flag, parser partial/overlong/NUL/invalid and
  bounded list indices, explicit clear. Existing detector/merge/elapsed/LED/
  IRQ/vendor checks still PASS. Lifecycle preservation also tested physically.

Final Flash32712/32768 (56 spare); .data104, .bss5836, staticRAM5940/8192.
2252 bytes remain above static objects. Linker minimum stack1024, heap0;
.heap_stack section1028 includes alignment. That reservation is NOT a peak-stack
safety proof. Peak stack NOT_MEASURED; no stack painting or power measurement.
Compiler frames: service496, counters416, drinkReport336, clockReport232,
finishRun200 bytes; not a call-chain/IRQ/libc peak bound.
Event32 B x32=1024 B; Summary24 B x4=96 B; Store1156 B. Independent real Store;
synthetic Store shares mutually exclusive2560-byte diagnostic trace union.
Snapshot is1 event/32 B. Placement construction uses no heap.
Largest final objects: diagnostic2560, realRecords1156, TX1024 bytes. LTO service
contains9556 bytes of inlined foreground work; Console::begin1076 / UART IRQ840 /
initializeEventSources672 are next large functions. Build report includes map
analysis. First link35776 Flash /8216 RAM including stack FAILED; preserved
first-link-overflow.txt. Fixed redundant nonzero object initialization, shared
out-of-line record operations, shorter help and smaller bounded query snapshot.
No recognition, event protection, trace/capture or existing regression command
was deleted. No slim-versus-diagnostic equivalence claim.
Worst new status245<256 bytes; today with all4 max-counter rows651<900-byte
admission. Each event row175<240 and list header107<128. See uart-bounds.json.

## Physical setup and finite results

Current PC TripleC / PC-02, Git2.46.2.windows.1, Python3.12.10, GCC14.2.1.
STLINK56FF6C066688555210351387, target0x425 Rev X /32KB, VTref2.72..2.73V as
reported by programmer. COM13 verified Prolific067B:2303,1152008N1. Nordic
COM6/COM7 enumerated only, never opened. User supplies power; no PPK2 or other
instruments, power setting, option bytes, hardware modification or backup-domain
reset. PA4 remains High in STOP. Bottle is EMPTY, same existing mounting;
axes/spout geometry and actual water intake not independently measured.

Evidence folders below are under evidence/. Automated PASS is distinct from
operator confirmation. All helpers clean up off/debug and close their UART.

| Evidence | Result and limits |
| --- | --- |
| synthetic01 | Integration image flash/verify/all32KB readback PASS.40 SYNTHETIC events ->32 retained,seq9..40,overwrite8. First across-midnight event belongs previous day; today's39 events/78 candidates survives ring replacement. All32 one-row pages, parser errors/partial command, explicit clear, source-mode gate, real-counter/RTC isolation and run1 STOP PASS. |
| calibration01 | RAM calibration PASS, reference55,-1011,-105 counts. No threshold edits. |
| empty-single01 | Automated1 candidate/1 event but user says motion incorrect; INVALID_OPERATOR_MOTION, EXCLUDE from acceptance. Raw UART retained. Explicit records clear confirm before retry; RTC/calibration retained. |
| empty-single02 | User confirms one correct empty gesture. New1 candidate/1 event, merge exactly10000ms,seq1 retained once; RTC invalid, flags0/dayUINT32_MAX, elapsed first316804/output326804. No dated summary. Original SystemSounds cue inaudible; direct two-tone winsound cue separately confirmed audible. |
| rtc-sync01 | RTC was already INVALID before stage7 flashing (BKP0..4 zero), contrary to last-session historical valid state. Normal existing setutc uses actual Windows UTC2026-10-26 08:33:25; NTP accuracy NOT independently verified. ctx1->2, validity restored; oldseq1 remains undated inctx1, today's valid summary0; no guessed backfill, no arbitrary midnight time / backup reset. |
| retention01 | Existingseq1 retained through mode debug/off/run1; RTC/STOP deadline and PWM/source register cleanup PASS. |
| empty-three01 | User confirms audible cue and three correct empty gestures.3 new candidates ->1 event, first546776/last558237/output568237 elapsedms; final wait10000ms. Storedseq2 UTC first1793003716/last1793003727/confirmed1793003737,candidates3,ctx2/day20752/tz480. Today1 event/3 candidates; undatedseq1 separate. |
| empty-shake01 | No candidate/event/record; SHAKE absent. User reports wrong timing; INVALID_OPERATOR_TIMING / EXCLUDE, not accepted shake result. |
| empty-shake02 | User confirms one correct shake, manual level5 display about3s/off. SHAKE1; no new candidates/events/records, PWM and LEDs released. |
| cancel-off01 | User confirms one correct empty gesture. Candidate963827ms, off sent before merge; explicit DRINK_CANCELLED,0 new events/records.11s quiet, then run1 and14s quiet: no stale confirmation. Existing2 records retained, cancellation count1, RTC valid, PWM/source cleanup PASS. Debug awake selected because UART RX is not a STOP wake source. |
| synthetic-real-retained01 | Repeats full40-event synthetic/32-page/clear suite while REAL has2 records and6 candidate/3 detection-event counters (includes excluded historical trial). REAL records/counters/calibration/progress/RTC unchanged. This proves nonempty isolation; synthetic events never count as actual drinking. |
| reset-real-records01 | Ordinary MCU software reset with REAL2 records ->0 records/seq0/undated0, RAM calibration lost as expected; valid RTC advances and BKP0..4 unchanged. No backup-domain reset. Integration-image evidence. |
| final-synthetic01 | FINAL flash/verify/all32KB readback, complete synthetic suite, parser, queries, clear, RTC preservation and run1/STOP PASS. Reset/flash keeps valid RTC. No physical gesture on this image is claimed. |
| final-rf-awake01 | FINAL image, operator confirms phone near/remove and stationary bottle. GPO ISR0->3/confirmed0->3/errors0, concurrent bounded list/today queries; no candidate/event/record increase. |
| final-rf-stop01 | FINAL image, operator confirms phone near/remove and stationary bottle. GPO ISR3->8,wake0->4,confirmed3->7,errors0,unknown0->1. RTC run25 deadline/stops32/unknownWake0; no candidate/event/record increase. One unknown GPO service retained openly; RF root cause/all-edge reliability not claimed solved. |
| final-close01 | FINAL all32KB readback matches; off/debug, all sources/display/PWM/sampling off, PA4High, GPIO LEDs Low, TIM2/TIM22 clocks off, LIS2 CTRL1/4=00, RTCvalid, REAL records0 after deliberate reset. UART closed. |

User confirmations are stored in per-run result.json. Aggregate detector counters
include excluded acquisition and do NOT form an acceptance denominator. Only
single02,three01,shake02,cancel-off01 are accepted empty-motion cases. These are
small integration checks, not accuracy statistics. No full100Hz raw capture or
new recognition replay is claimed for this stage; frozen stage6 replay remains
historical. A tool-only RF helper first start failed from a missing Path import
before any hardware/port/audio action, was fixed and then run; no failed motion
was silently replaced. Shell sandbox startup initially failed; authorized
fallback shell succeeded. No permission review rejection or bypass was used.

## Limits / next steps

No real water, intake, volume, pouring/straw confusion, population accuracy,
power, six-month battery, long-duration/race/fault stress or peak-stack claim.
Time/zone boundary suite is pure logic / isolated synthetic time; no real RTC
was moved to midnight. No UART timezone command exists; timezone changes are
verified only through the pure API. Hardware pending-context-change and pending
mode-debug/deadline cancellation were not newly repeated; off is current HIL,
other guards have compiler checks and inherited stage6 historical evidence.
No new full0..8 optical cohort; current level5/PWM cleanup is the relevant check.
Inter-page views are independent bounded snapshots; not a global list snapshot.
Four summary creations are finite: old summaries are evicted, not persisted.
RAM records and sequences reset; RTC survives an ordinary reset only while the
RTC backup state remains powered/valid. Current valid date is not a promise
of absolute RTC/NTP accuracy.

Stage7 is stopped here. Before any stage8 implementation, define scheduling
semantics and address56-byte Flash headroom; do not silently remove regression
coverage or squeeze in reminders. Next motion session must recalibrate after
current reset. All stage7 files/evidence saved locally, uncommitted/unpushed;
cloud replication not verified. Only the authorized baseline annotated tag was
pushed. Canonical handoff/machine/HIL updated separately; other machines' history
and unrelated/conflicted files preserved. No session helper left active;
instrument processes were not inspected or controlled.
