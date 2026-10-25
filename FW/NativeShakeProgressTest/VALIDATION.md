# Stage 5 validation - 2026-10-25

## Version and scope

Final image SHA256: `2ae93c94b3038495a94f9af0101a858ac326cdcc8427ec7c3f88d162f5828ed2`.
UART identity: `NativeShakeProgressTest-v1`.
Baseline commit: `820ecb3f2b9fb199ddf5cf95246698d4648b73b1`;
annotated tag: `baseline/native-hal-eventloop-v1`. No stage5 commit or push.
Repeat build matches the final image: results/build-final-repeat-20261025.json.
Build instructions, source versions and timer ownership are in README.md.

## Final physical cohort

Device mounted on bottle in intended product orientation, confirmed by user.
Exact XYZ-to-bottle axis mapping remains unmeasured. Results below are bounded
prototype tests, not production accuracy or resistance to deliberate cheating.
Evidence paths are relative to tests/results/.

| Evidence directory | Actual result |
| --- | --- |
| controls-sustain02 | Flash, verify, full readback, UART, cancellation, invalid input noninterference, off/debug, restart PASS |
| bottle-pickup5a / bottle-pickup5b | User completed 5+5 normal pickups, no LED: 0/10 false displays; 6+10 observation windows rejected, zero errors |
| bottle-shake5a / bottle-shake5b | User completed 5+5 intentional shakes, all displays correct: 10/10; five confirmations per group, zero errors |
| bottle-tilt | Three user-confirmed slow tilts, no LED; six windows rejected, zero errors |
| bottle-continuous | About eight seconds continuous shaking: one confirmation/display, about three seconds then off, no extension or repeated display (user confirmed) |
| bottle-awake-rf | Awake RF: GPO ISR1, source confirmed1, no shake; initial FIELD0 |
| bottle-stop-rf | STOP RF: GPO ISR5, wake observations4, source confirmed4, unknown source1, source errors0; returned to STOP and RTC deadline/debug |
| bottle-mixed | Same run: motion then RF; shake confirmed1, motion wake1, GPO ISR/wake/confirmed1; RTC deadline/debug PASS |
| final-auto | Full image readback, all LED levels, timeout/PWM cleanup, parser boundaries, run10/run60/off/run1, UART recovery and RTC reset retention PASS |
| final-state.json | Read-only final register inspection PASS; core resumed, no reset or power action |

Final LED playback: user confirms all levels0..8, zero two short flashes,
low/high distinction and LED1-to-LED4 cumulative ordering correct.
ISR, source confirmation, handling and physical trial counts are different
quantities; latched sources can merge events. RF ISR counts are not approach
counts. Field removal wake is not established under the LPD policy. Cached
status field is not a fresh RF measurement; RF is not application NFC transfer.
Real XYZ replay uses the actual C++ detector; synthetic tests prove logic only.
Stored traces generally represent the last observation, not every trial.

## RTC, timing and resources

Final auto explicitly set UTC from Windows PC UTC (NTP accuracy unverified).
Ordinary MCU reset with power held retained valid RTC and BKP2=544D0101,
BKP3=6ADDDB76; UTC advanced from1792924534 to1792924535. BKP0/1/4 unchanged.
No backup-domain reset or option-byte operation. Earlier supply interruption
actually lost markers/time validity: power-off retention is NOT established.

| Resource | Bytes |
| --- | ---: |
| Flash including data initial values | 23996 |
| Flash remaining | 8772 |
| .text / .rodata | 21484 / 2204 |
| .data / .bss | 104 / 3152 |
| Static RAM | 3256 |
| RAM after static allocation | 4936 |
| Minimum linker stack / heap | 1024 / 0 |
| Space outside static allocation and minimum reservation | 3912 |

Trace buffer1280 and application TX1024 are major RAM consumers. Peak stack
NOT_MEASURED; minimum reservation and remaining RAM do not prove stack safety.
No unsafe live-stack painting. No power measurement or battery-life claim.

## Historical failures and fixes

Preserved old pickup5a: four normal pickups falsely displayed; fifth possibly
outside deadline. Old shake10 captured seven confirmations in a60s window;
operator did ten actions, later ones possibly outside window: not a7/10 rate.
New detector retains amplitude threshold and adds20ms qualified direction dwell
and600ms gesture span. Real truncated old false-positive prefix rejects in
replay, followed by fresh full bottle-cohort tests above. Mounting also changed;
do not attribute all improvement to the detector alone.

Earlier RTC snapshot/cache timing defects were corrected with bounded coherent
RTC reads, elapsed-time guards, and refresh after STOP/before observation.
Final tests include PC elapsed anchor and fresh sample timing. Historical
logs remain available and are not relabelled as final-version successes.
controls-sustain first flash failed at STLINK0V without programming; user
restored power, controls-sustain02 then completed successfully.

## Remaining limits and final state

- Small single-bottle/operator cohort; fill level, other bottles and mounting
  variability remain untested. Some stored positive samples approach +/-2g
  clipping; full-scale/threshold tuning needs further data.
- Precise RF overlap with sampling/LED, simultaneous IRQ/race stress, electrical
  waveforms/VDCG and fault injection remain unverified.
- Peak stack, long-term RTC accuracy, power, sensor/buzzer interaction and
  production recognition performance remain OPEN. Historical RF root cause OPEN.
- Final board: image above, off/debug, display/PWM/sampling off, sources disabled,
  progress retained7, RTC valid; UART closed. User-supplied2.8V unchanged.
- No PPK2, other measurement instruments, hardware modification, mass erase,
  option-byte changes or further commit/push. Existing unrelated work preserved.
