# Current: Pebo Protocol v0.1 Android engineering + SAMPLE (2026-10-15)

Same existing project/App ID,versionCode3. Read PROTOCOL-v0.1.md first for
architecture,wire layouts,explicit provisional assumptions,tests and blockers.
Original user requirement:protocol-user-request-v0.1.txt.
58unit tests PASS;debug/signature PASS. Phone absent at installation attempt:
version3 APK NOT installed,protocol screen/device use NOT physically verified.
SAMPLE data never means real Pebo protocol success. Real protocol transport is
explicitly unbound;legacy engineering NFC detection/single-block reader retained.
APK SHA256 E79CB668514AA061C4628E5909F5F87587FD6AE4BEBB5F06DF679D90726F35C7.
See PROTOCOL-v0.1.md for current artifact names and integration next steps.
No real tag/MCU write,tool/global configuration change,commit or push.

---
Previous single-block/discovery implementation notes below are historical:
# Current: ST25DV single-block read-only engineering test (2026-10-15)

Updated IN PLACE in the existing Pebo-NfcProbe-20261015 project; same
applicationId com.pebo.nfcprobe20261015, versionCode 2 / versionName 1.1.
Previous discovery implementation retained in MainActivity.discovery-backup.kt
(outside source sets). Existing discovery continues when no read is armed.
The old discovery-only notes below are historical and superseded for this build.

Only memory command: ISO 15693 Read Single Block (0x20), addressed mode:
22 20 <8 raw Android Tag.id bytes> <one-byte block number>.
22 = high data rate (02) + addressed (20), Option=0, Inventory=0, Select=0.
UID display is raw-order hex and never reversed for TX. It is not Pebo Device ID.
No CRC appended by App; Android stack handles CRC. Complete application RX is
shown (not RF SOF/EOF/CRC). Strict response contract: flags 00 plus exactly
4 data bytes => success; flags 01 plus exactly 1 error code => tag error;
nonzero reserved/unsupported flags, empty/short/long response => malformed.
All-zero/all-FF data are valid and not interpreted as battery/drinking data.
Bound: ST25DV04K/KC user EEPROM 128 blocks x 4 bytes, decimal 0..127.
No writes, locks, passwords, system/config commands, product payload or network.

Press Prepare once, then retap a tag. Arm consumed before background worker;
no automatic retry/repeat. Busy disables editing and Prepare. On pause, pending
operation is cancelled and an active link is closed on another executor to
cancel blocking I/O. Running worker keeps busy until completion; cancelled or
stale results cannot become success. Worker always attempts close in finally.
TagLostException, IOException, tag error, malformed reply and cancelled result
are distinguished. User can prepare again after success/failure. If NFC was
toggled externally, return to App to refresh ReaderMode status.

Screen shows NFC support/on state, UID/order, block, built TX with sent/not-sent
label, full RX, validated current data and explicit separate historical success.
Failure/invalid input clears current-data display. Logs include timestamp/zone,
block/TX/RX/result/error, copy/clear controls, latest 100 entries in memory;
activity saved state preserves bounded logs over recreation. No new permissions;
manifest remains NFC only. Existing private diagnostic status.json uses no
storage permission; read logs are not persisted to external files or network.
Clipboard copy is only triggered by user pressing Copy.

Validation: 20 JUnit tests PASS (14 command/parser/input; 6 one-shot/lifecycle
state), no failures/errors/skips. testDebugUnitTest assembleDebug succeeded in
29 s, 40 tasks (8 executed, 32 up-to-date), after final cancellation-close synchronization fix. Build/signature warnings unchanged
and non-blocking. Same SDK/JBR/Gradle/AGP versions as prior build.
Build log single-block-build-test.log; XML/HTML reports under app/build.
APK app/build/outputs/apk/debug/app-debug.apk, SHA256
BBEB950B61B57F83B02A1509330D4AB2AAC4A2352776948A6F3015FAE575E83E.
Earlier intermediate build: update install -r Success; Activity launch Status: ok. FINAL build above is NOT installed: user disconnected phone before final close synchronization fix. Initial PAUSED/locked
screen meant physical reads were not yet validated; see canonical handoff for
later measured outcomes. No uninstall or clear-data.

Physical checklist (results require actual user-operated tags):
[ ] Block 0 succeeds with exactly 4 data bytes.
[ ] Move away and prepare/retap 5 times; no extra read without Prepare.
[ ] Remove tag during I/O; observe error and recovery after reprepare.
[ ] -1, 128, nonnumeric inputs rejected without TX.
[ ] Prepare, background App, return: no armed operation; can prepare anew.
Also check Copy/Clear and history label after failure.

Manufacturer documents directly opened/checked:
- KC: DS13519 Rev 8, p3 capacity, pp80-83 flags/errors, pp88-89 single read.
  https://www.st.com/resource/en/datasheet/st25dv04kc.pdf
- K: DS10925 Rev 11, p3 capacity, pp75-78 flags/errors, pp83-84 single read.
  https://www.st.com/resource/en/datasheet/st25dv64k.pdf
  This is ST's K-family document covering ST25DV04K/16K/64K; the alternate
  st25dv04k.pdf URL returned an error, so that URL was not used as evidence.
- Android CRC and NFC I/O/cancellation:
  https://developer.android.com/reference/android/nfc/tech/NfcV
  https://developer.android.com/reference/android/nfc/tech/TagTechnology

Rebuild with original process-only environment commands below. Test command:
.\gradlew.bat testDebugUnitTest assembleDebug --no-daemon --console=plain
Project remains local TEMP; no production-source move or duplicate project.

---
Historical discovery-only implementation notes:
# Pebo NFC discovery probe (2026-10-15)

Scope: NFC-V discovery via Android ReaderCallback. No transceive, NDEF check,
memory read/write, formatting, lock command, authentication or MCU changes.
This is discovery-only RF evidence, not Pebo payload freshness or chip-variant
identification. NFC-V detection alone cannot uniquely identify ST25DV or Pebo.
Use only the intended Pebo board near the phone to attribute the result.

App ID: com.pebo.nfcprobe20261015. Screen: Pebo NFC 唯讀測試.
UID is displayed in Android Tag.id byte order without reversing it; UID is not
included in host logs. Discovery metadata only is saved in private files:
status.json (session/state/count), last_detection.json
(session/count/timestampMs/nfcV/uidLength/tech). No card contents are collected.
The count is discovery callbacks in the current Activity instance, not a claim
of unique tags. Reader mode runs only while Activity resumed; leaving it pauses
polling. The last detection message remains visible until another event/restart.

Build PASS: Gradle Wrapper 9.3.1, AGP 9.1.1 / built-in Kotlin 2.2.10,
JBR 25.0.3, SDK 37.0 / Build Tools 36.0.0, 34 tasks in 20 s.
Signature verification PASS. SDK XML / apksigner warnings were non-blocking.

Rebuild in this directory (PowerShell, process-only variables):
```powershell
$env:JAVA_HOME = 'C:\Program Files\Android\Android Studio\jbr'
$env:GRADLE_USER_HOME = "$env:TEMP\Pebo-Android-EnvCheck-20261015\.gradle-user-home"
.\gradlew.bat assembleDebug --no-daemon --console=plain
```
APK: app/build/outputs/apk/debug/app-debug.apk.
SHA256: 621E1743BAAB736D356B710B5017B58079E500B74765BB9F0FD9A1E3FA30CD56.
Installed as a new App and started successfully; initial READY/count=0.
Physical discovery outcome belongs in project docs/handoff.md after observation.
Keep phone unlocked and App in foreground; place only Pebo by phone antenna,
move slowly/hold 2 seconds, remove and retap. Record board revision, fitted chip,
power state, case/battery/ferrite and geometry before broader RF claims.
No instrument/power changes made. This directory is temporary/local.

References:
https://developer.android.com/reference/android/nfc/NfcAdapter
https://developer.android.com/reference/android/nfc/tech/NfcV
https://www.st.com/en/nfc/st25dv04kc.html
Physical discovery PASS: actual NfcV callback and user visual confirmation. See physical-detection.log. Board power/variant/geometry not verified; no memory/payload test.


2026-10-15 final-build physical Block0 PASS: final APK update install Success; user-operated read, session1792039552199; full RX00 00 00 00 00, four data bytes00 00 00 00. readBusy=false,armedBlock=-1 afterwards. Evidence in original TEMP project/single-block-physical-result.log. Five repeats/interruption/invalid UI/background and Copy-Clear UI remain unverified;see handoff.
