# NFC data contract — draft requirements

This document defines desired phone-visible information, **not yet a final byte layout**. Confirm the selected ST25DV variant's usable memory and RF/I²C behavior against the manufacturer document and real board before freezing offsets.

## Phone read requirements

| Group | Fields | Status |
| --- | --- | --- |
| Identity | Device ID; protocol version; optionally HW/FW versions | Device ID required; representation TBD |
| Battery | Measured voltage (mV), estimated level/status, measurement age | Voltage required; percentage is an estimate |
| Drinking | Today's detected count, last detected event time, recent event history | Required; history depth TBD |
| Clock | Time validity, device time or last sync | Proposed for time-sync diagnosis |

The device detects likely drinking events; it does **not** measure water volume. Do not label event count as volume or a medically validated hydration goal. A “daily target” is a configurable product feature, not yet a fixed requirement.

## Format design decisions still needed

- NDEF payload for phone discovery and app handoff versus application-specific memory reads; test both iPhone and Android flows.
- Binary structure version, offsets, endianness, length, counter range, timestamp epoch, validity marker, and checksum/CRC if used.
- Memory allocation, write endurance, atomic snapshot/commit method, and behavior if phone reads during an MCU update.
- Whether extended history is on tag, in phone storage, or transferred over multiple reads. A generic C `struct` may include padding and must not define the wire format by itself.
- Authentication/access control for mutable commands. A public NFC Device ID should not be treated as a secret or proof of device ownership.
- Time-sync command sequence, stale-command detection, confirmation, and host/RTC clock tolerance.

## Candidate phone flow

1. Read a small, versioned status record with device ID, battery reading age/value, detected drink count, and last event.
2. Optionally read history using a documented layout.
3. If supported, write a versioned time/configuration request and read an acknowledgement after the MCU has processed it.

Avoid promising immediate fresh battery readings from a passive NFC read: the phone can read the latest MCU-published snapshot even while the MCU sleeps. The age field makes this visible.

## Validation

Use PN532 for repeatable reads, writes, corrupt input, interrupted writes, and version compatibility tests. Separately verify actual iPhone and Android apps and RF placement. Reader behavior and phone permissions differ; PN532 success alone does not establish mobile compatibility.
