# NFC data contract — draft requirements

This document defines desired phone-visible information, **not yet a final byte layout**. Confirm the selected ST25DV variant's usable memory and RF/I²C behavior against the manufacturer document and real board before freezing offsets.

## Verified board behavior

- ST25DV user/system I2C access is working.
- PA4 / ST25_LPD must remain GPIO Output High during STOP for low current.
- ST25DV GPO has been configured for an RF field-change bring-up event.
- In the current test, GPO idle is LOW and the event is active HIGH, so PA3 uses a rising-edge EXTI wake.
- Phone/NFC field activity successfully wakes STM32L031 from STOP.
- The wake handler reads the ST25 dynamic interrupt status and the GPO returns to idle after the event is cleared.
- This validates the hardware wake path only; it does **not** finalize the phone data protocol.

## Phone read requirements

| Group | Fields | Status |
| --- | --- | --- |
| Identity | Device ID; protocol version; optionally HW/FW versions | Device ID required; representation TBD |
| Battery | Measured voltage (mV), estimated level/status, measurement age | Voltage required; percentage is an estimate |
| Drinking | Today's detected count, last detected event time, recent event history | Required; history depth TBD |
| Clock | Time validity, device time or last sync | Proposed for time-sync diagnosis |

The device detects likely drinking events; it does **not** measure water volume.

## Format design decisions still needed

- NDEF payload for phone discovery and app handoff versus application-specific memory reads; test both iPhone and Android flows.
- Binary structure version, offsets, endianness, length, counter range, timestamp epoch, validity marker, and checksum/CRC if used.
- Memory allocation, write endurance, atomic snapshot/commit method, and behavior if phone reads during an MCU update.
- Authentication/access control for mutable commands.
- Time-sync command sequence, stale-command detection, confirmation, and host/RTC clock tolerance.
- Whether production firmware should wake on field appearance only, field disappearance only, or treat FIELD_CHANGE as a bring-up-only configuration.

## Candidate phone flow

1. Read a small, versioned status record with device ID, battery reading age/value, detected drink count, and last event.
2. Optionally read history using a documented layout.
3. If supported, write a versioned time/configuration request and read an acknowledgement after the MCU has processed it.

## Validation

Use PN532 for repeatable reads, writes, corrupt input, interrupted writes, and version compatibility tests. Separately verify actual iPhone and Android apps and RF placement.
