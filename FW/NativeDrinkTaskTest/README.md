# NativeDrinkTaskTest - INTERRUPTED WIP, NOT ACCEPTED

User requested immediate stop for shutdown on2026-10-27. No stage8 image flashed.
Stage8 automatic Git policy in docs/stage8-git-policy.md overrides the old
no-commit clauses in REQUEST.md. No acceptance tag may be created for this WIP.

Baseline annotated checkpoint/native-drink-record-compact-v1 pushed:
87b10e4410047eab0f76a8118e7842c8ed4262e9 -> b7b9171b1987041acd2dd29430742c8b2ef071c6.
Compact rebuild27684/static3256/image1086703970e17547ef91f1c39521545be3f146a3aef886d489d9fb91799614c1.
Original source/detection/records/Board/linker preserved; new independent project.

Implemented WIP: TaskLogic fixed16 slots, atomic staging, current/previous day,
catchup, event generation/serial dedupe,900s default elapsed gap, integer progress;
TaskParser and TaskApp connect first-candidate snapshot to confirmed record and
task progress. Real progress override rejected; diagnostic tasks test isolated.
This code is not accepted or physically validated.

Current blocker: compact33348B first; selective noinline+checks33488B (720 over).
Task-only diagnostic35516B (2748 over). Linker remains32KB/8KB. No safety/capacity
removal or limit relaxation. Map paths build/<config>/NativeDrinkTaskTest-<config>.map.
Large stage7 trace/capture/register dumps stay in NativeDrinkRecordTest; stage8
diagnostic intends task-test-only. Build still fails; no runnable stage8 image.

Resume: inspect maps, quantify/revert ineffective noinline experiment and fit
compact first. Review core semantics/parser/format/TX bounds; expand and execute
host tests (runtime emulator and saved replay). Then diagnostic/compact HIL,
small empty actions,RTC/RF/STOP/mixed/PWM cleanup and exact readback. Never label
old compact hardware evidence as stage8 PASS. No water/reminder/NVM/NFC added.

Build (currently expected to fail linker capacity):
`python FW/NativeDrinkTaskTest/build.py --config compact --toolchain-bin <ARM-GCC-bin>`
Use --config diagnostic for the task-only test build. No PPK2 operations allowed.
See VALIDATION.md and canonical handoff for exact interrupted state.
