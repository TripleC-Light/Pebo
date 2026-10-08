# Power budget and measurement

Target: **at least six months** from a CR2032 under a stated use profile. No battery-life claim is verified yet.

## Latest measured low-power results

Measured on Pebo v0.1 PCBA at **2.8 V** with **ST-LINK disconnected**. The measurements below include the powered STM32L031, ST25DV04KC, and LIS2DW12 branches unless otherwise noted.

| Firmware condition | Average current | Periodic peak | Result |
| --- | ---: | ---: | --- |
| Initial deep-sleep baseline | 302.11 µA | Not recorded | Too high; firmware/IO state not yet optimized |
| Lowest-current STOP baseline, ST25 LPD not actively held High | 56.68 µA | ~130 µA every ~40 ms | Major improvement, but still dominated by ST25 state |
| ST25 LPD actively driven High during STOP | **6.84 µA** | ~16 µA every ~40 ms | Current best whole-board low-power baseline |
| Peripheral NVIC IRQs disabled, ST25 LPD unchanged | 56.7 µA | ~130 µA every ~40 ms | No meaningful effect |

### Confirmed conclusion

Driving **PA4 / ST25_LPD High** during system deep sleep reduces measured whole-board average current from about **56.68 µA to 6.84 µA**, a reduction of about **49.84 µA (~88%)**.

Therefore:

- **PA4 / ST25_LPD must be explicitly driven High before entering deep sleep.**
- Do **not** convert PA4 to Analog/No-Pull while ST25DV is powered.
- The earlier ~40 ms periodic peak is not explained by peripheral NVIC IRQs, because disabling those IRQs did not materially change the 56.7 µA / 130 µA pattern.
- With ST25 LPD High, the ~40 ms periodic peak is still present but falls to roughly 16 µA. Its remaining source is still under investigation.
- Do not treat the 6.84 µA value as final battery-life average; LED, piezo, NFC, sensor-active time, wake events, temperature, CR2032 pulse behavior, and self-discharge still need to be included.

## Measurement worksheet

| State/event | Current or charge | Frequency/duration | Conditions | Status |
| --- | --- | --- | --- | --- |
| Whole-board deep sleep, ST25 LPD High | **6.84 µA avg** | Continuous | 2.8 V, ST-LINK disconnected | Measured |
| Residual sleep peak | ~16 µA peak | ~every 40 ms | Same test condition | Measured; source pending |
| Motion wake and classification | TBD | TBD per day | Motion trace | Pending |
| Reminder piezo | TBD | TBD per day | Sound pattern, VDD | Pending |
| NFC and snapshot update | TBD | TBD per day | Reader and field | Pending |
| Battery measurement and logging | TBD | TBD per day | ADC, UART mode | Pending |

Estimate average current from measured charge per event plus baseline sleep current, then check cell capacity under actual pulse loads, end voltage, temperature, and storage/self-discharge conditions. A simple capacity divided by MCU sleep current is insufficient.

## Bench method

- Use PPK2 as development supply and current meter. Test startup, sleep, wake, sound, NFC, and voltage points relevant to the selected cell. Document current ranges and sampling configuration.
- Power-cycle by commanding PPK2 output and checking that VDD actually falls sufficiently before a cold-start test. Board capacitance and external signal back-power can invalidate it.
- Check ST-LINK reference/IO, USB-UART TX, PN532 wiring, and scope probe grounds for their electrical effect. A grounded scope probe must never be clipped onto a non-ground node.
- Repeat critical behavior with a real CR2032 and holder; a regulated supply does not reproduce the cell's pulse response.
- For subsystem isolation, use the board's 0-ohm links:
  - R15: STM32_VDD
  - R16: LIS2_VDD
  - R17: ST25_VDD
- Next useful isolation measurement is to compare MCU-only, MCU+LIS2DW12, MCU+ST25DV (LPD High), and full-board deep-sleep current.

## Open questions

- Source of the remaining ~40 ms periodic current peak when ST25 LPD is High
- Individual deep-sleep current contribution of STM32L031, LIS2DW12, and ST25DV
- Battery measurement topology and production debug connection
- I2C pull-up losses and possible 10 kΩ -> 22 kΩ optimization
- Antenna coupling effects
- Reminder duty cycle and real usage frequency
