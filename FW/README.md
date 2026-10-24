# Pebo firmware and bring-up tests

This folder contains focused Pebo v0.1 firmware tests and is also the intended home for future integrated product firmware. Keep passed bring-up tests intact as regression references.

## Verified test firmware

- `LowestCurrentStopBaseline`: STOP-mode current baseline.
- `StopBaseline_LPDHigh`: verifies the critical ST25DV LPD-high low-power condition.
- `StopBaseline_DisableIrq`: isolates peripheral IRQ contribution to sleep current.
- `LIS2WakeupStopTest`: LIS2DW12 INT1 motion wake from STM32 STOP. Verified; functional idle measured around 4.43 µA at 2.8 V.
- `RTCStopWakeTest`: external LSE + RTC Wakeup Timer from STM32 STOP. Verified with repeated 10 s wake cycles; idle remains around 4.43 µA.
- `ST25GpoWakeTest`: ST25DV GPO RF-field event wake from STM32 STOP. Verified with phone/NFC field; idle remains around 4.43 µA.
- `RTCDeepSleepTest`: earlier RTC/deep-sleep exploration retained for reference.

## Libraries

- `libraries/PeboLIS2DW12`: minimal LIS2DW12 I2C driver with probe, low-power sampling, raw XYZ readout, and accelerometer power-down.
- `libraries/PeboST25DV`: minimal ST25DV I2C driver with probe, user EEPROM read/write helpers, and LPD-pin low power-down control.

## Confirmed low-power rule

While ST25DV is powered, **PA4 / ST25_LPD must remain GPIO Output High during STOP**. Do not include PA4 in generic Analog/No-Pull shutdown code.

Before adding pin-specific code or low-power behavior, verify the current board against:

- `docs/hardware.md`
- `docs/hardware_pinmap.md`
- `docs/firmware-architecture.md`
- `docs/nfc-protocol.md`
- `docs/power-budget.md`
- `docs/hil-test-plan.md`
