# Resource profiles validation — 2026-10-26, PC-02 TripleC

Resource work saved locally only. HEAD/main aac139c32a1fa5df927ae05d053e1cad55bbb59b,
remote main independently matched at start and closure. Only authorized checkpoint tag pushed.
See RESOURCE_REPORT.md for the corrected00 baseline tail/tag-annotation caveat and detailed costs.
Historical stage7 evidence and its pre-output-fix motion limitations remain in VALIDATION.md.

## Host and build evidence

- Exact original32712B rebuild; normal diagnostic31096/compact27684 within unchanged32KB/8KB linker.
- Independent output-directory rebuilds byte-identical per profile: reproducible-builds.json.
- Core Drink/Shake/records/time/elapsed and Board normalized source equality to1001ea0;
  core-source-equality.json explicitly normalizes CRLF only. No thresholds/merge interval altered.
- Both builds execute real constexpr policy/parser/RecordChecks/ConfigChecks (capacity32,
  four summaries, dates/leap/year boundaries, midnight pending, invalid-time no backfill,
  timezone/context edits, cancellation, saturation/sequence/overwrite, clear and isolation).
- Runtime host ARM emulation (Unicorn2.1.3, not physical board or native host C++ execution):
  actual formatter vs nano snprintf621 boundary/canary cases per profile; invalid format/null guards.
  Signed INT_MIN/UINT_MAX, widths, capacities0/1/2/7/31/63/128/256/384 covered.
- Same15 saved stage6 empty-bottle RAW clips per profile:10 independent single,
  two pickup5 cohorts and three multi-candidate clips. Actual C++ detector/coordinator
  compiled with profile defines; transitions identical and match saved reference.
  replay-summary.json plus per-clip hashes/logs. Replay is not new physical accuracy acceptance.
- Compact ELF has no diagnostic union or RAW/SAMPLE/synthetic-entry/register-dump strings.
  Final symbols and allocation lists checked. Required record/source-event strings retained.
- Missing/invalid profile and compact diagnostic-exclusion options fail; invalid-build-config.json.
- UART typed-limit storage/admission tests PASS (uart-bounds.json): DRINK275<288,
  RECORDS245<256, four-summary651, full counters555, status596<900; hardware TX ring1024 unchanged.

## Hardware: exact image scopes

Selected STLINK56FF6C066688555210351387, target0x425 RevX; current Prolific COM13
VID067b/PID2303 verified before opening115200. No Nordic/PPK2 ports opened, no power
measurement, hardware/option/protection/backup reset changes or full-chip erase.
Normal page erase/write/verify only. Before/after full32KB reads validate new image prefix,
zero fill to last128B page, and unchanged old bytes after that page. Shorter images leave
historical trailing pages intact: full32K SHA is deliberately distinct from image SHA.
Scripts always finally off/debug and close their owned UART. User-provided supply unchanged.

|Configuration|Image bytes/SHA256|Full32KB read SHA256|
|---|---|---|
|diagnostic|31096 / 8f22b3cb624c3009138dbab8d709f87ae1726c3c87c8907185ffdee242da3190|6b4e4cd47255db76161d7990e5a93a1712ed21ad004225587afb7e5e191f88fd|
|compact (final)|27684 / 1086703970e17547ef91f1c39521545be3f146a3aef886d489d9fb91799614c1|195f2b7ac873fb15db2349d5d038302e0d95815e90b6f322ba2b79a5ada616e8|

Diagnostic evidence hil-diagnostic-records:40 isolated SYNTHETIC additions,32 retained,
seq9..40/overwrite8, leap-day late confirmation retained previous summary and current39,
all32 one-row pages, invalid32/partial/NUL/overlong/explicit clear, real records and
recognition counters unchanged; RTC valid/BKP/UTC progression unchanged. Run1 deadline,
register diagnostics, PWM off/sensor restoration PASS. hil-diagnostic-capture: stationary
upright calibration, capture5 outputs501 RAW rows, trace92 SAMPLE rows, dropped0,
TRACE_END, no real record, sensor restored, PWM off. Diagnostic feature mask15 confirmed.

Compact evidence hil-compact-auto: removed commands reject, fixed parser boundaries,
run10 and run60 deadline PASS,70 STOP transitions, unknownWake0/source errors0,
records/today/list queries. RX commands sent during STOP are not claimed as delivered;
UART RX wake remains unsupported by inherited design. Separate hil-compact-uart keeps
CPU awake with progress display across run deadline:24 finite queries,6 complete APP/DRINK,
TODAY/LIST_BEGIN/LIST_END and at least6 counters blocks, no error/truncation.

Compact manual windows use two PC pure tones1600/1100Hz700ms each. User confirmed:
- hil-compact-single: one correct EMPTY motion,1 candidate/1 confirmation/1 real record,
  seq1, valid UTC/local date/summary1 candidate. Elapsed merge10336ms (>=10000, RTC/STOP
  service granularity), one addition only. Actual drinking remains unverified.
- hil-compact-shake: one shake,1 SHAKE_CONFIRMED, progress5 displayed ~3s then off;
  no new candidate/event/record. Original harness erroneously required a trailing space
  after token; original FAIL retained and raw log reanalysis PASS_REANALYZED after exact-token
  fix. No firmware or repeat physical action was needed; user confirmed normal display.
- hil-compact-cancel: one new candidate, immediate off cancels,0 new confirmed/record;
  11s wait and run1 plus14s wait yield no stale event. Old seq1 record/summary retained.
- hil-compact-retention: off/debug/newrun preserve the nonempty real record.
- hil-compact-reset: ordinary MCU reset with seq1 retained beforehand resets RAM/sequence
  to0 and calibration invalid; RTC stays valid, BKP0..4 equal, UTC advances. No backup-domain reset.
- hil-compact-rf-awake:3 new GPO ISR/confirmations, errors0/unknown0; repeated query service,
  no drink record. hil-compact-rf-stop:3 new ISR/wake/confirmations, errors0/unknown0,
  run25 deadline,30 cumulative STOP/unknownWake0. User confirmed both phone windows,
  bottle stationary. No NFC application payload or update protocol acceptance claimed.

Physical coverage differs: diagnostic was tested for diagnostics/capture/synthetic pipeline;
compact for real empty-event integration/shake/cancel/reset/RF awake+STOP. Shared core and
replay do not prove all physical behavior equivalent. No repeated full classifier accuracy cohort,
no actual water, power test, environmental accuracy or runtime peak stack measurement.

## Final state and remaining work

hil-final verifies exact compact prefix/full32KB identity above; off/debug, RTC/motion/GPO
sources0/0, lpd1 (PA4 high), display0/PWM0/sampling0, RTC valid, records0/sequence0,
calibration invalid following ordinary reset. UART closed; no owned background serial helper left.
Historical beyond-image bytes preserved; do not hash zero-padded shorter image and call it full read.

Minimum stack1024/heap0 unchanged, runtime peak NOT_MEASURED. Partial diagnostic path1064B
already exceeds minimum reservation; actual2348B after static is not safe feature budget.
Compact5084B Flash spare falls below~6KiB expectation; scoped stage8 planning can start,
but implementation capacity/stack safety need continued measurement and no future fit guarantee.
No stage8 features added. All unrelated tests/conflicted HIL copy preserved. No resource commit/push.

## Reproduction (repository-root PowerShell)

```powershell
$armTools = 'C:/Users/Edward/AppData/Local/Arduino15/packages/STMicroelectronics/tools/xpack-arm-none-eabi-gcc/14.2.1-1.1/bin'
python FW/NativeDrinkRecordTest/build.py --config diagnostic --toolchain-bin $armTools
python FW/NativeDrinkRecordTest/build.py --config compact --toolchain-bin $armTools
# Optional host-emulation dependency only; never linked into firmware:
python -m pip install --target FW/NativeDrinkRecordTest/build/host-deps --no-deps unicorn==2.1.3
python FW/NativeDrinkRecordTest/tests/resource_host.py --config diagnostic --toolchain-bin $armTools
python FW/NativeDrinkRecordTest/tests/resource_host.py --config compact --toolchain-bin $armTools
python FW/NativeDrinkRecordTest/tests/resource_replay_batch.py --toolchain-bin $armTools
python FW/NativeDrinkRecordTest/tests/resource_uart_bounds.py
# Connected-board actions: correct supply/selected STLINK/COM13 must be available.
# Each results directory must be new. These helpers always close UART/off/debug.
python FW/NativeDrinkRecordTest/tests/resource_diagnostic_bench.py --config diagnostic --flash --results FW/NativeDrinkRecordTest/resource-evidence/new-diagnostic
python FW/NativeDrinkRecordTest/tests/resource_board.py --config diagnostic --test capture --results FW/NativeDrinkRecordTest/resource-evidence/new-capture
python FW/NativeDrinkRecordTest/tests/resource_board.py --config compact --test auto --flash --results FW/NativeDrinkRecordTest/resource-evidence/new-compact
python FW/NativeDrinkRecordTest/tests/resource_board.py --config compact --test final --results FW/NativeDrinkRecordTest/resource-evidence/new-final
```

Normal Cube write used by helpers: `-c port=SWD sn=56FF6C066688555210351387 mode=UR
freq=4000 reset=HWrst -w <profile.bin> 0x08000000 -v -rst -score -run`.
Read: `-c port=SWD sn=... mode=HOTPLUG freq=4000 -u 0x08000000 32768 <read.bin> -score -run`.
No erase-all argument. Future board identity/ports must be independently checked per machine.
Manual single/shake/cancel-off/awake/stop helper tests produce audible action windows;
do not run them without operator readiness. Historical stage7 helpers target historical
build paths; use resource helpers for these profiles.
