# Firmware architecture

## V1 responsibilities

- Detect candidate drinking motions from LIS2DW12 samples and record qualified events.
- Maintain time with the RTC and external LSE. Define an explicit UTC wire format and a local-day policy in the mobile application or configuration.
- Schedule reminders according to configured time windows; drive brief piezo patterns.
- Publish a consistent NFC status snapshot and accept validated configuration/time-sync requests.
- Enter an appropriate low power state between events.

## Proposed event flow

1. Motion interrupt or scheduled wake occurs.
2. MCU reads the source, samples only as needed, and updates the detection state.
3. A qualified drink event updates counters and history.
4. MCU prepares the NFC-visible snapshot safely, then returns to low power state.
5. A phone NFC interaction may request time sync or settings; MCU validates, applies, and acknowledges them via a defined mechanism.

The exact interrupt mapping, STOP mode, NFC update method, and wake timing require schematic review and board measurements. Do not assume the MCU is awake during a phone read or that NFC EEPROM writes are instantaneous or unlimited.

## Time and data integrity

- Store absolute event times in a documented UTC representation; define how an unsynchronized clock is indicated.
- Define local-day boundaries explicitly so “today” is stable across time-zone changes and daylight-saving transitions.
- Use version and integrity checks for persistent settings/history as needed; make interrupted NFC or power operations recoverable.
- Restrict debug logs to development builds, and measure their current impact.

## Open decisions

- Actual clock and low power configuration, watchdog behavior, brownout response, and battery ADC path.
- Reminder schedule, quiet hours, event separation, and user-adjustable target semantics.
- How the ST25DV RF side, I²C side, GPO, and LPD interact in the selected part/board configuration.
