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
| Functional whole-board STOP | **~4.43 µA avg** | Continuous between events | 2.8 V, wake-test firmware | Measured |
| LIS2 motion wake | Active debug average observed ~357 µA while continuously shaken | Event-dependent | UART/I2C debug active | Functional test only; event charge pending |
| RTC wake | TBD charge/event | Current test every 10 s | LSE + RTC Wakeup Timer | Wake function verified |
| ST25 GPO / RF-field wake | TBD charge/event | Phone/NFC field event | FIELD_CHANGE bring-up | Wake function verified |
| Reminder piezo | TBD | TBD per day | Sound pattern, VDD | Pending |
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
- Piezo pulse current, voltage sag, and reminder duty cycle
- Antenna coupling effects and real usage frequency
