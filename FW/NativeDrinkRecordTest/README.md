# NativeDrinkRecordTest — stage7 RAM observation records

Publication: resource implementation/tests/evidence committed as `b7b9171b1987041acd2dd29430742c8b2ef071c6`,
pushed to origin/main and independently verified after explicit user authorization.
Local-only statements below describe validation-time history; no new hardware tests during publication.

Independent STM32L031 native HAL project derived from the frozen stage6 source.
No classifier thresholds changed. Drinking-like motion is not verified intake;
candidate counts are neither sip counts nor millilitres. No water acceptance,
scheduling, scores, reminders, audio driver, automatic progress, NVM, or NFC/App
protocol is added. PC audio in the host test helper is only an operator cue.

## Current resource profiles

Post-stage7 local resource work provides **diagnostic** (complete diagnostics) and
**compact** (development configuration, not production). Explicit `--config` required;
outputs `build/<config>/NativeDrinkRecordTest-<config>.{elf,bin,hex,map}`.
Startup/status identify config and feature mask. Full commands, differences and resources:
[RESOURCE_REPORT.md](RESOURCE_REPORT.md); exact tests/final image and reproduction:
[RESOURCE_VALIDATION.md](RESOURCE_VALIDATION.md). Diagnostic31096B/static5844B;
compact27684B/static3256B,32 events/four summaries unchanged. Heap0/min-stack1024,
runtime stack peak NOT_MEASURED. Compact Flash spare5084B, below~6KiB aspiration.

## Historical stage7 baseline and build

Annotated `checkpoint/native-drink-detection-v1` points to
`3d6d70a9a60e5c0ea0ed8dd95b6a00ca1f72c1cf`; explicitly pushed without force.
Current source/remote main at start: `274e08467371a4b357b156d5f87f519a30ed3561`.
The baseline source/build/linker/vendor diff from that commit to HEAD is empty.
Rebuild and actual board image both match
`09cdb9cc0d952a759e24917fd94fcdd09b01b44ad0b790af84c78a20238c3293`.
Supplemental host evidence is dbe4fe4; later commits are documentation. See
`evidence/baseline-verification.json` and `baseline-rebuild.json`.
Stage7 published after explicit user authorization as commit
1001ea0e34e51ccb73634a40e177a938b0ddcde0 on origin/main; remote SHA verified.

```powershell
python FW/NativeDrinkRecordTest/build.py --config diagnostic --toolchain-bin C:/Users/Edward/AppData/Local/Arduino15/packages/STMicroelectronics/tools/xpack-arm-none-eabi-gcc/14.2.1-1.1/bin
```

Same pinned HAL/CMSIS/TIM manifests, 32KB/8KB linker, GCC14.2.1, Os/LTO,
bounded integer formatter (current profiles), strong vectors, heap0, minimum stack1024. Build executes
actual pure C++ constexpr checks in the compiler, including RecordChecks.cpp.
The HAL-free tests can also run directly with a host C++17 compiler:
`g++ -std=c++17 tests/RecordChecks.cpp -o record-checks && ./record-checks`.
This PC has no verified native host compiler: compiler-evaluated assertions
are reported separately from runtime HIL; no native executable run is claimed.
Stage7 historical implementation retained all diagnostics. Current profiles and
different physical coverage are described in the resource reports above.

## Event and time contract

`src/DrinkRecords.h` has no HAL, sensor, UART or dynamic allocation. APIs:
Store.initialize / observe / change / append / find / at / clear;
Pending.candidate / confirm / cancel. Callers supply valid UTC only after
validating the inherited UTC 2000..2099 clock. `change` validates offset
-840..+840 minutes. Default is +480; there is deliberately no UART tz command.
On an explicit time edit, cancel Pending and the detector merge before any
further observation. Pending also retains its original context and rejects
confirmation into another context. `observe` detects validity changes and
non-adjacent/backward date jumps; explicit same-day clock edits use `change`.
A natural adjacent day in the same context creates a new summary, not a new
context. Firmware routes its existing setutc through this mechanism, including
failed edits that may have partially touched the clock.

Only the existing 10-second elapsed merge confirmation consumes Pending.
Each completed valid candidate captures a wall stamp immediately. Event first
is the FIRST CANDIDATE COMPLETION, last is the last completion, and confirmed
is the later merge output. Merge/timeout still use the unchanged cross-STOP
elapsed clock; no wall-clock subtraction reconstructs candidate timestamps.
Exactly 10s flushes the old group before the next candidate. A consumed Pending
cannot confirm twice; Store also rejects non-increasing event sequence numbers.
Off/debug/new run/deadline cancel unfinished state and retain recorded events.
RF, shake, queries and rejected candidates never call record confirmation.

Aligned Event is exactly32 bytes: seven uint32 values (sequence, context,
first, last, confirmed, candidates, local day), int16 timezone minutes and
uint16 flags. Bit0 means valid UTC seconds; otherwise times are wrapping boot
elapsed milliseconds and day is UINT32_MAX. Bit1 marks SYNTHETIC. There is no
packing or unaligned access. Local day is floor((UTC+zone*60)/86400), days since
1970-01-01 in the event's saved timezone. It may include 1999-12-31/2100-01-01
at UTC-range timezone edges. Dates and offsets are never reinterpreted later.
An unknown event remains unknown after time becomes valid; it is never backfilled.
A group across midnight stays one event assigned to the first completion day.
A time-context change explicitly cancels a pending group and reports the reason.

## RAM limits and summaries

32 events cost1024 bytes. Four aligned Summary objects cost96 bytes; Store is
1156 bytes including bookkeeping. Every saved event retains its context/day/zone.
Full ring overwrites oldest and saturates the reported overwrite count. All
counters saturate at UINT32_MAX. Sequence reaches UINT32_MAX once and subsequent
adds reject rather than wrap. Context exhaustion blocks further recording and
is exposed by exhausted=1. Explicit clear restarts sequences; normal off does not.

Summaries are independent of the ring: event/candidate totals survive event
replacement. Four (context,day) summaries use FIFO BY CREATION, never renewed
by a late event. Normal midnight retains current/previous summaries, allowing
an event first completed before midnight to confirm afterward. Context edits
preserve old summaries until this bounded capacity evicts them. Missing old-day
summary updates increment summaryLost and do not contaminate today. Evicted
totals cannot be reconstructed from the limited event list. `records today`
shows every retained summary and marks the current pair; these are observations,
not completed scheduling tasks. lastFirstUtc is the latest event's occurrence
(first completion) timestamp in that summary.

The SYNTHETIC Store occupies a union with the320-row/2560-byte diagnostic trace.
Entry requires off, debug and no sampling/merge/display/trace/probe/run. Placement
construction changes the active union member; there is no heap allocation.
Trace cannot run in test mode and its old diagnostic rows are explicitly lost.
Exit reconstructs the trace member and sets its row count to0. Real Store,
calibration, progress, detector counters and RTC are independent and retained.

## UART

Existing commands retained. New fixed32-byte-parser commands:

- `records status`: context/day/zone/validity, retained count and first/last seq,
  overwrites, undated count, summaryLost/rejected/exhausted, struct costs, RAM reset warning.
- `records today`: readable local date/current context plus up to4 summaries.
  Invalid date prints0000-00-00 with valid=0; it is not an actual date.
- `records list` or `records list 0..31`: oldest-index page, one event per page.
  Header and event are copied into a bounded single-event snapshot at admission.
  Header advertises the then-retained range. Each loop emits at most one row,
  continuing RTC/GPO/application services; page activity prevents STOP. Pages
  are individually consistent, NOT one global snapshot across all32 commands.
  Clients must use returned seq/range to detect changing retention across pages.
- `records clear confirm`: explicitly clears events, summaries, counts and seq;
  retains time context, RTC and calibration; rejects while sampling/merge/list.
- `records test enter`: requires fully off/idle; resets only synthetic/trace space.
- `records test add`: SYNTHETIC pair of completions2s apart, confirm12s after first;
  starts at2024-02-28 23:59:55 +08, advances20s per add. Does not write actual RTC.
- `records test day`: advances isolated test time1 day.
- `records test exit`: clears synthetic instance, restores empty trace, retains REAL.

Every synthetic record/query/ack carries SYNTHETIC. Application/source-changing
commands are rejected while testing; off/debug remain safe. No continuous record
printing, serialization framework, floating point or persistent write path.

## Validation and hardware operation

See VALIDATION.md for exact current-image results and limits. Host helpers
verify specific STLINK/MCU and Prolific COM13 (enumerate again next session),
read all32KB Flash, and close with off/debug and UART closed. They never operate
PPK2 or other instruments, power, option bytes or backup-domain reset.
Publication status above supersedes validation-time local-only notes. Use `records_bench.py --flash --results NEW_DIRECTORY`
for finite synthetic/transport checks; `motion_bench.py --test ... --audio`
uses Windows PC audio plus manual LED cue for a small EMPTY-bottle window.
`--clear-before` is explicit and clears only application RAM records/statistics.
Operator must confirm motions; automated count PASS alone is insufficient.

Final loaded-image SHA256: c648aef249ce4e2f1c3bc419291a5c546f3c979febcd40c35983879e5703e958.
Flash32712 (56 spare), .data104/.bss5836, staticRAM5940; Event32 B, Store1156 B.
Minimum stack1024/heap0, actual peak NOT_MEASURED. Integration-image physical
evidence and the final UART-only buffer fix are distinguished in VALIDATION.md.
