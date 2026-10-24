# Power budget and measurement

Target: **at least six months** from a CR2032 under a stated use profile. No battery-life claim is verified yet.

## Latest measured low-power results

Measured on Pebo v0.1 PCBA at **2.8 V** with **ST-LINK disconnected** unless otherwise noted.

| Firmware condition | Average current | Periodic/event behavior | Result |
| --- | ---: | --- | --- |
| Initial deep-sleep baseline | 302.11 µA | Not recorded | Too high; firmware/IO state not yet optimized |
| Lowest-current STOP baseline, ST25 LPD not actively held High | 56.68 µA | ~130 µA peak every ~40 ms | Dominated by ST25 state |
| ST25 LPD actively driven High during STOP | **6.84 µA** | ~16 µA peak every ~40 ms | Major reduction |
| Peripheral NVIC IRQs disabled, ST25 LPD unchanged | 56.7 µA | ~130 µA peak every ~40 ms | No meaningful effect |
| LIS2DW12 low-power motion-wake test | **~4.43 µA** | Motion correctly wakes MCU | Verified functional idle |
| LSE + RTC 10 s STOP-wake test | **~4.43 µA** | RTC wakes repeatedly every 10 s | Verified functional idle |
| ST25DV GPO RF-field STOP-wake test | **~4.43 µA** | Phone/NFC field correctly wakes MCU | Verified functional idle |

### Confirmed conclusions

- **PA4 / ST25_LPD must be explicitly driven High before and during deep sleep.**
- Do **not** convert PA4 to Analog/No-Pull while ST25DV is powered.
- Disabling peripheral NVIC IRQs alone did not change the 56.7 µA / 130 µA pattern.
- Current focused functional wake tests return to roughly **4.43 µA average** after adding LIS2 motion wake, RTC wake, or ST25 GPO wake.
- Do not treat 4.43 µA as the final product average. LED, piezo, active motion processing, NFC transactions, wake-event charge, temperature, CR2032 pulse behavior, self-discharge, and real use frequency still need to be included.

## Measurement worksheet

| State/event | Current or charge | Frequency/duration | Conditions | Status |
| --- | --- | --- | --- | --- |
| Earlier whole-board deep sleep, ST25 LPD High | **6.84 µA avg** | Continuous | 2.8 V, ST-LINK disconnected; earlier firmware | Historical measurement |
| Earlier residual sleep peak | ~16 µA peak | ~every 40 ms | Earlier test condition | Historical; source pending |
| Functional whole-board STOP | **~4.43 µA avg** | Continuous between events | 2.8 V, wake-test firmware | Measured |
| LIS2 motion wake | Active debug average observed ~357 µA while continuously shaken | Event-dependent | UART/I2C debug active | Functional test only; event charge pending |
| RTC wake | TBD charge/event | Current test every 10 s | LSE + RTC Wakeup Timer | Wake function verified |
| ST25 GPO / RF-field wake | TBD charge/event | Phone/NFC field event | FIELD_CHANGE bring-up | Wake function verified |
| Reminder piezo | Preliminary results below | TBD per day | Sound pattern, VDD | Initial HIL completed; anomalies pending |
| NFC data transaction | TBD | TBD per day | Reader/phone and field | Pending |
| Battery measurement and logging | TBD | TBD per day | ADC, UART mode | Pending |

Estimate average current from measured charge per event plus baseline sleep current, then check cell capacity under actual pulse loads, end voltage, temperature, and storage/self-discharge conditions. A simple capacity divided by MCU sleep current is insufficient.

## Bench method

- Use PPK2 as development supply and current meter. Test startup, sleep, wake, sound, NFC, and voltage points relevant to the selected cell. Document current ranges and sampling configuration.
- Repeat critical behavior with a real CR2032 and holder; a regulated supply does not reproduce the cell's pulse response.
- For later subsystem isolation, use R15 (STM32_VDD), R16 (LIS2_VDD), and R17 (ST25_VDD).

## Open questions

- Individual deep-sleep current contribution of STM32L031, LIS2DW12, and ST25DV via R15/R16/R17 isolation
- Charge per motion/RTC/NFC wake event in production-like builds without verbose UART debug
- Battery measurement topology and production debug connection
- I2C pull-up losses and possible 10 kΩ -> 22 kΩ optimization
- Antenna coupling effects
- Reminder duty cycle and real usage frequency

## Battery-voltage accuracy experiment (2026-10-13)

The independent BatteryVoltageTest sketch measures VDDA from internal
VREFINT; no external divider or new pin assignment is introduced.
Calibration is read through ST's VREFINT_CAL_ADDR symbol, calibrated
at VREFINT_CAL_VREF (3000 mV); voltage is 3000 * calibration / mean raw ADC.
Default averaging: 32 samples after one discarded conversion.
UART emits BATTERY_MV once, then ADC/internal buffer/clock and UART
are disabled before STOP; PA4/ST25_LPD remains Output High.

The PPK2 sweep compares this reading with requested supply setpoints
from 3.0 to 2.6 V, using a configurable +/-50 mV threshold.
This does not independently measure the supplied rail and does not
measure charge/current. Verify actual VDDA/VDD, wiring drops, discharge,
boot at each point and post-measurement current on the board.
All new accuracy and current results are pending HIL; the existing
6.84 uA baseline is not a measurement of BatteryVoltageTest.
No battery percentage algorithm or battery-life claim is added.

### Initial BatteryVoltageTest HIL (2026-10-13)

With ST-LINK disconnected and USB-UART RX/GND connected, a 2.8 V PPK2
setpoint boot printed VREF_RAW=1790, VREF_CAL=1661, BATTERY_MV=2784,
SAMPLE_COUNT=32. A separate five-point sweep passed +/-50 mV against source
setpoints (maximum absolute error 26 mV); see docs/handoff.md for all rows.

After a 2 s UART/boot window, approximately 5 s of PPK2 serial capture
decoded 496128 samples: mean 3.161851 uA, median 3.155366 uA, min 2.451175 uA,
max 5.576022 uA. Metadata Calibrated=0 was recorded; its semantics and
absolute current calibration remain unverified. Treat this as initial
low-consumption evidence, not a certified value or a comparison proving an
improvement over the old baseline. Firmware shutdown/PA4 states are verified
in source; physical PA4 voltage and direct peripheral-register inspection
have not been measured in this run.

Artifacts (metadata/UART summary and raw serial capture):
Software/PPK2Automation/results/battery_post_measurement_current_20261013_161228_072850.json
Software/PPK2Automation/results/battery_post_measurement_current_20261013_161228_072850.bin

The default 1 s OFF interval produced one boot and four UART timeouts.
A rerun with --off-wait 15 --timeout 5 produced five successful boot readings.
Longer discharge improving reset reliability is observed; the VDD decay
waveform and exact cause are still pending independent measurement.
PPK2 output OFF was commanded in finally after each session.
No average-use battery-life estimate is implied by this sleep-only window.

User DMM follow-up in a controlled 2.8 V powered window measured VDD and
PA4/ST25_LPD both at 2.805 V; this confirms the physical LPD High level in
the post-measurement state. That boot's UART value was 2788 mV (-17 mV
versus the reported rail reading). OFF-state VDD after a long wait was
reported as 0.27 V; exact discharge time/cause remain unmeasured.

## Piezo current measurement automation (2026-10-14)

Added a dedicated piezo current and LIS2 interaction test. It is designed to
measure the charge cost of 2 kHz, 3 kHz and 4 kHz single-ended piezo beeps and
to detect whether those beeps create LIS2DW12 motion false triggers.

Firmware uses PA0 only, through R20 to Piezo A, with Piezo B grounded by the
single-ended board option. PA1 is held LOW. No differential drive is added.
The waveform is 50 percent duty square wave from TIM2_CH1 on PA0; each
frequency runs three 500 ms beeps with 1500 ms gaps.

The PPK2 automation computes, per beep:

- 300 ms pre-event baseline average current.
- Active average current.
- Raw peak and 99th percentile peak.
- Duration.
- Baseline-subtracted charge:
  `integral(max(I_uA - baseline_uA, 0) * dt_seconds)`.
- Charge in uAh as `charge_uC / 3600`.
- LIS2 false-trigger flags from firmware markers.

The script does not set a product current limit yet; current sanity only checks
that active average and p99 peak rise above baseline and that the duration is
approximately 500 ms. Large piezo current is recorded, not failed, until an
audible-volume and energy tradeoff is selected.

ppk2-api 0.9.2 current streaming provides decoded current samples through
`get_samples()` but no per-sample timestamp or simultaneous DUT voltage
waveform in this path. The script aligns UART markers and current samples with
the PC monotonic clock and the configured PPK2 sample rate, default 100 kHz.
Voltage sag fields are left blank and console reports
`VOLTAGE_SAG=NOT_AVAILABLE_FROM_API`. Verify instantaneous VDD sag with an
oscilloscope before drawing CR2032 pulse-load conclusions.

Initial piezo HIL started on 2026-10-14 at 2.8 V. The first firmware run
produced 9 UART events, but the user heard only one beep; the cause was a PWM
re-arm bug after returning PA0 to GPIO LOW between beeps. After fixing
`startPiezo()` to force TIM2_CH1 PWM mode on PA0 for every beep, the user
confirmed hearing 9 beeps.

Post-fix current automation result:
`Software/PPK2Automation/results/piezo_current_20261014_164204.csv`.
Functional UART result passed, but overall result failed because LIS2 false
triggers occurred on 6 of 9 events and current sanity failed on two events.
Measured summary from that run:

| Frequency | Mean active current | Mean p99 peak | Mean charge | False triggers |
| ---: | ---: | ---: | ---: | ---: |
| 2000 Hz | 6140.033 uA | 7992.162 uA | 787.334 uC | 1 / 3 |
| 3000 Hz | 6895.840 uA | 7432.605 uA | 36.836 uC | 3 / 3 |
| 4000 Hz | 6856.046 uA | 7629.217 uA | 59.418 uC | 2 / 3 |

These are first HIL automation outputs, not final product energy numbers. The
large seq=1 charge and high active baseline after boot need review, and ppk2-api
still does not provide instantaneous VDD sag; use a scope before drawing CR2032
pulse-load conclusions.
