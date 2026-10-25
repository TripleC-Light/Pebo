# Operator evidence (2026-10-25 Taiwan)

- User confirms same Pebo, user-supplied2.8V, common ground and freeCOM5,
  permits flashing and ordinary MCU reset. No instruments operated.
- LIS2 installation direction relative to bottle explicitly UNCONFIRMED.
- Pre-final b2ad066f motion01: actual ISR18/wake6/source confirmations20,
  two observation windows, one shake accepted, one rejected, errors0.
  User visually confirms fifth level (two high + third low) and ~3s off.
  Last successful48-sample real trace replays accepted through actual detector.
- Pre-final shake10: user reports actually doing10; LEDs stop from8th.
  Timed60s capture has7 shake confirmations at12.4..55.7s, not10.
  This is NOT established 7/10 recognition sensitivity: later operations may
  extend beyond run deadline/off; trial-by-trial timestamps aren't independently
  observed. Repeat in5-trial windows to avoid this confound.
- Real trace from shake10 starts at386ms. Found pre-STOP elapsed cache reused
  while HAL tick had not advanced after wake. Final16272d9c... forces RTC
  elapsed refresh after STOP and before motion observation, without changing
  sensor settings or detector thresholds. New trace assertion first row<=100ms.
- auto05 on final16272d9c...: flash/verify/readback/UART, valid nonzero RTC
  reset retention, PC elapsed anchor, LED patterns/timeouts/clock release,
  parser, run10/60/off/re-run1,149 real diagnostic samples + idle restore PASS.
  Diagnostic observe is NOT physical motion or STOP wake proof.
- Remaining operator trials are recorded separately below as they finish.

## Confirmed false-positive round and follow-up

pickup5a on16272d9c...: user explicitly confirms normal pickup/place,
first4 lit, fifth didn't. Four SHAKE_CONFIRMED, one rejected window, no sensor
errors, RTC60s deadline/debug and STOP48. This is a FAIL, not accepted recognition.
The fifth may be outside deadline; don't use this as a clean4/5 statistical rate.
Read-only capture after off preserved45 samples from the last false acceptance;
full original window beyond that early acceptance is not available.
New20ms qualified-direction dwell +600ms repeated-motion span rejects that
truncated prefix in actual C++ replay, but needs fresh complete physical tests.

New image2ae93c94b3038495a94f9af0101a858ac326cdcc8427ec7c3f88d162f5828ed2
build PASS, Flash23996/data104/bss3152/staticRAM3256, minstack1024/heap0.
controls-sustain flash attempt found STLINK0.00V/no target and did NOT flash.
Board last verified image remains16272d9c... off/debug/UARTclosed; user asked
to restore supply/VTref. No power or measurement instrument was operated.

## Final follow-up (supersedes blocked status above)

User restored supply; controls-sustain02 flashed and verified2ae93c94... .
Device now mounted on bottle in intended product orientation (user confirmed).
Two groups of5 normal pickups: user confirms all10 no LED. Two groups of5
intentional shakes: user confirms all10 correct display. Three slow tilts:
user confirms none lit. Continuous~8s shaking: user confirms only one display,
off after~3s without repeated lighting or extension. Final0..8 playback:
user confirms zero double flash, low/high brightness and cumulative order all
correct. RF awake, STOP RF and sequential shake/RF evidence in bottle-* logs;
final-auto and final-state.json confirm cleanup and final valid RTC reset test.
See ../../VALIDATION.md for counts, limitations and preserved earlier failures.
Board off/debug, UART closed; no instruments or new commit/push.
