# Pebo official firmware

This folder is reserved for the official Pebo product firmware.

The existing `Firmware/` folder contains hardware verification and bring-up work.
Keep new product firmware development here so it remains separate from board
validation experiments.

## Libraries

- `libraries/PeboLIS2DW12`: minimal LIS2DW12 I2C driver with probe, basic
  low-power sampling, raw XYZ readout, and accelerometer power-down.
- `libraries/PeboST25DV`: minimal ST25DV I2C driver with probe, user EEPROM
  read/write helpers, and LPD-pin low power-down control.

Before adding pin-specific code or low-power behavior, verify the current board
against:

- `docs/hardware.md`
- `docs/hardware_pinmap.md`
- `docs/firmware-architecture.md`
- `docs/nfc-protocol.md`
- `docs/power-budget.md`
- `docs/hil-test-plan.md`
