# Pebo Android — portable handoff

## Current source update: 2026-10-25

Read Status percentage-format crash is fixed in ProtocolActivity.kt via the
pure StatusText.kt formatter, covered by four StatusTextTest regressions.
The original 58 tests plus these four pass (62 total); debug build also passes.
Wire/models, commands, Gradle/dependencies and firmware are unchanged.
Current evidence and 25-file source manifest:
handoff/evidence/pc02-20261025-status-fix/.
The original handoff/source-manifest.csv and ZIP remain immutable transfer
snapshots: ProtocolActivity now intentionally differs; two formatter/test files
are new. Use the current evidence manifest for this fixed source, not the old
ZIP as a replacement. Do not overwrite newer source with the original archive.
The Sony phone still has original v3: local debug signer differs, so no update
or data clearing was attempted. Device fix verification remains pending.
No instrumentation runner/dependencies are configured; local emulator has no
hypervisor driver. This session ran JVM tests, not instrumentation/UI acceptance.
Local build uses an isolated JBR cacerts copy with the existing Windows-trusted
AVG inspection public root; JAVA_TOOL_OPTIONS points to it only in build
subprocesses. No system/JBR trust store or global environment was modified.
For local rebuild, see docs/handoff.md; never copy this machine's trust files
or local.properties as another machine's verified setup.
Minimal v0.2 is documentation only: docs/pebo-nfc-protocol-v0.2-minimal-draft.md
and its decisions companion; neither replaces the implemented v0.1 protocol.

This is the original Android project transferred from the source computer's
TEMP directory on 2026-10-15, not a new implementation. Source and Wrapper were
copied unchanged and 23 files matched SHA256; handoff/source-manifest.csv records
them. The original TEMP project remains intact. Use this saved source for future
handoff; do not rely on the old computer's TEMP path.

Application ID: com.pebo.nfcprobe20261015; versionCode 3; versionName
0.1-protocol-sample. Full architecture, layouts and provisional decisions:
PROTOCOL-v0.1.md. Original requirement: protocol-user-request-v0.1.txt.
Canonical current task/history: repository docs/handoff.md. Per-machine facts:
docs/development-machines.md. This README is a setup guide, not a competing log.

## Confirmed and pending

- 58 unit tests passed on the source computer: 38 protocol + 20 engineering.
- Debug build/signature passed. Version 3 was NOT installed: no phone was present.
- Previous version's physical NFC-V discovery and Block 0 read (00 00 00 00)
  passed; this does not prove version 3 UI or real Pebo application packets.
- Sample is clearly labelled. Real application packet transport is unbound;
  no real protocol writes, MCU firmware implementation or fabricated readings.
- All five extended block-reader physical checks and Sample UI smoke remain
  pending as detailed in the canonical handoff/HIL plan.

## Next computer: start here

1. Ensure the complete folder/archive arrived. A matching cloud path is not
   evidence of synchronization. Verify source-manifest.csv and archive hash
   against the handoff record. Do not overwrite existing local work.
2. Read repository AGENTS.md, docs/handoff.md, docs/development-machines.md,
   docs/nfc-protocol.md and PROTOCOL-v0.1.md. Confirm actual computer name,
   Git branch/HEAD/status and dated machine profile before development.
3. Extract/copy this project to an ASCII-only local path, e.g.
   C:/Dev/PeboAndroid. Prefer a local Git checkout for regular development.
   Avoid simultaneously running Git/build activity on the shared Drive checkout.
   Windows AGP may reject non-ASCII paths; no override or system change is needed
   if the local build path is ASCII. The shared folder is the saved transfer copy.
4. Verify Android Studio JBR and local SDK. Historical source build used JBR25,
   Gradle Wrapper9.3.1, AGP9.1.1, Kotlin2.2.10, SDK platform37.0 and BuildTools36.0.0.
   Use full tool paths. No global Gradle or emulator is required. Do not upgrade
   dependencies, accept SDK licenses or alter global PATH/JAVA_HOME automatically.
5. Generate local.properties for THIS computer using Android Studio's SDK
   selection, or the PowerShell example below. Never copy the original machine's
   local.properties. It and caches/IDE state/private keys are excluded.
6. Run the 58 tests and debug build. Installing Android Studio alone is not proof
   of build readiness. Recheck actual results, versions and network on arrival.
7. Discover phone with local SDK platform-tools/adb.exe devices -l; accept RSA
   authorization. Only install/update when exactly one authorized phone exists;
   with multiple devices ask the user. Do not uninstall or clear any App data.
8. Smoke-test version3: main screen opens Protocol/Sample; permanent Sample
   labels, DeviceInfo, Status (3 drinks / 1 completed period), config edit/readback,
   History40/40/5 pages, SyncTime, Debug/invalid HEX. Real mode must clear Sample
   fields and explicitly report unavailable. Regress engineering Block0 separately.
9. Resolve provisional CRC/ACK/History and atomic EEPROM/mailbox binding before
   any actual MCU protocol integration. See PROTOCOL-v0.1.md for open questions.

## Build example (PowerShell, process-only settings)

Run from the local ASCII project directory. This does not change global settings.
Verify paths exist first; adjust the local SDK location if needed.

```powershell
$peboJbr = 'C:\Program Files\Android\Android Studio\jbr'
$peboSdk = Join-Path $env:LOCALAPPDATA 'Android\Sdk'
if (!(Test-Path "$peboJbr\bin\java.exe")) { throw 'Studio JBR not found' }
if (!(Test-Path "$peboSdk\platforms\android-37.0\android.jar")) { throw 'SDK37.0 missing' }
if (!(Test-Path "$peboSdk\build-tools\36.0.0\aapt2.exe")) { throw 'BuildTools36.0.0 missing' }
$peboSdkText = $peboSdk.Replace('\','/')
$peboEscapedSdk = -join ($peboSdkText.ToCharArray() | ForEach-Object {
    if ([int]$_ -gt 127) { '\u{0:x4}' -f [int]$_ } else { [string]$_ }
})
[IO.File]::WriteAllText((Join-Path $PWD 'local.properties'), "sdk.dir=$peboEscapedSdk", [Text.Encoding]::ASCII)
$env:JAVA_HOME = $peboJbr
.\gradlew.bat testDebugUnitTest assembleDebug --no-daemon --console=plain
```

Use a fresh shell to return to previous process environment. Initial Gradle/Maven
resolution may need network and a new cache on the destination PC. Wrapper pins
the distribution checksum. SDK auto-download is disabled; missing SDK/license
errors must be reported, not bypassed or auto-accepted.

## Artifacts and signing

handoff/evidence/pebo-v3-protocol-sample-debug.apk is the exact built version3 APK:
SHA256 E79CB668514AA061C4628E5909F5F87587FD6AE4BEBB5F06DF679D90726F35C7.
Reports/logs and historical physical-read evidence are under handoff/.
Historical source backups are outside app/src, so they are not compiled.
No private signing keys, credentials, local.properties, Gradle cache or SDK were
transferred. A newly built debug APK on another computer may use a different
signing key and fail to update the existing App. Do not uninstall/clear data to
fix this. The supplied APK retains its original signature for a same-App update;
report INSTALL_FAILED_UPDATE_INCOMPATIBLE before deciding signing-key management.
Never put private keystores in the repository or handoff documents.

All files were saved locally to the shared workspace, without commit or push.
Cloud replication and destination readiness are unverified until checked there.
## Portable restore verification, 2026-10-15

An archive of this saved source was extracted to an independent ASCII-only
TEMP directory and built from scratch. All 58 tests passed (0 failures/errors/
skips); assembleDebug PASS in 1m1s, all 40 tasks executed. Evidence and fresh
reports are under handoff/evidence/portable-restore-pass.log and
handoff/test-reports/portable-restore-html/.
The first attempt exposed a missing transformed-directory in the OLD machine's
Gradle cache. No source or existing cache was altered. A separate clean cache
was seeded only with the existing Wrapper distribution and Maven modules;
transforms were regenerated, and the offline rebuild succeeded. Neither cache
is included in this package. This proves this archive restores/builds with the
source computer's verified tools; destination machine readiness still needs
its own check. No firmware/phone operations were performed for this handoff.
