# Native HAL B recovery baseline

Local annotated reference: baseline/native-hal-eventloop-v1 (never force-move).
The commit/tag creation is user-authorized; no push is authorized. Resolve the
exact commit using git rev-parse baseline/native-hal-eventloop-v1^{} and inspect
the annotation with git show --no-patch baseline/native-hal-eventloop-v1.

Expected native image SHA256:
e7e4e403763de5e1175f6b891f278817262aa5712ac69deb7c38a43426a367c0
Flash18164B, staticRAM1352B, minimumstack1024B/heap0. Full diagnostic variant B,
not a slimmed diagnostic build. Existing source and JSON evidence are retained.

This snapshot includes the full Arduino stage4 comparison/reference and only
TimeManagementTest/TimeLogic.h from stage3, because native and A use that pure
header. Other stage3 firmware and FeedbackTest are outside this baseline scope.
Vendor manifest is byte-exact; local .gitattributes disables text conversion for
vendor sources/licenses so checkout preserves their manufacturer SHA256 values.

## Rebuild without disturbing a dirty checkout

Use a new, unused directory, never reset/clean the active worktree:

```powershell
git worktree add --detach ../Pebo-native-baseline baseline/native-hal-eventloop-v1
Set-Location ../Pebo-native-baseline
$env:ARM_GCC_BIN = '<installed xPack GCC14.2.1-1.1 bin>'
python FW/NativeEventLoopStopTest/build.py --build-dir "$env:TEMP/Pebo-native-baseline-rebuild"
```

No Arduino installation is needed for B. Python3 standard library plus matching
GCC/newlib toolchain and vendored ST files suffice. Repository-relative shared
headers and host checks are included. See README for optional A comparison,
explicit safe flashing/target identity and HIL commands. Do not flash merely
to inspect/rebuild the baseline. No option-byte/RDP/mass-erase/power commands.

## Passed and remaining limits

Build repeatability, manifest, strong vectors and shared host logic PASS.
Flash/verify/readback/UART/parser, repeated STOP10/60/off/re-run1 PASS.
Motion26; awakeRF1; STOPRF2; mixedmotion31/GPO1 observations PASS with deadline
and UART recovery, no error/unknownWake/quarantine in these finite tests.
Backup equality covers five zero marks only, not nonzero valid-time retention.
Peak stack, clock/I2C fault injection, exact races/simultaneous edges, long-run
accuracy/reliability, electrical waveforms and power remain unverified. Previous
RF failure cause OPEN. This reference does not approve full product migration.

Stage5 must be a new independent NativeShakeProgressTest; do not edit or retag
this baseline to add shake/LED features. No subsequent commit/push is implied.
