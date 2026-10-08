# Firmware architecture

## V1 responsibilities

- Detect candidate drinking motions from LIS2DW12 samples and record qualified events.
- Maintain time with the RTC and external LSE. Define an explicit UTC wire format and a local-day policy in the mobile application or configuration.
- Schedule reminders according to configured time windows; drive brief piezo patterns.
- Publish a consistent NFC status snapshot and accept validated configuration/time-sync requests.
- Enter STM32 STOP mode between events and wake only from defined sources.

## Verified Pebo v0.1 wake paths

The following board-level wake paths have been physically verified:

1. **Motion wake:** LIS2DW12 INT1 -> PB0 / EXTI -> STM32 STOP wake.
2. **Time wake:** external 32.768 kHz LSE + RTC Wakeup Timer -> RTC_IRQn -> STM32 STOP wake.
3. **NFC wake:** ST25DV GPO RF-field change event -> PA3 / EXTI -> STM32 STOP wake.

Focused regression sketches are kept under `FW/LIS2WakeupStopTest`, `FW/RTCStopWakeTest`, and `FW/ST25GpoWakeTest`.

In current functional tests the whole board returns to roughly **4.43 µA average at 2.8 V** after wake handling.

## Confirmed STOP-mode rules

- PA4 / ST25_LPD must remain configured as GPIO Output High while ST25DV is powered.
- SysTick is stopped before STOP in low-power test firmware.
- Debug UART and I2C are shut down when they are not needed in STOP.
- Only the intended wake IRQ/source should remain enabled in each focused test.
- PC14/PC15 must remain assigned to the external LSE when RTC is in use.
- Wake handling must clear the peripheral source and MCU pending state before returning to STOP.

## Proposed integrated event flow

1. A motion interrupt, RTC event, or NFC GPO event wakes the MCU.
2. Firmware identifies the wake source before enabling unnecessary subsystems.
3. For motion wake, MCU samples only as needed and updates the drinking-detection state.
4. For RTC wake, firmware evaluates reminder scheduling.
5. For NFC wake, firmware reads and clears ST25DV event status, then services any pending data/configuration exchange.
6. Any changed state is committed to the NFC-visible snapshot safely.
7. Firmware restores the confirmed low-power pin/peripheral state and returns to STOP.

## Time and data integrity

- Store absolute event times in a documented UTC representation; define how an unsynchronized clock is indicated.
- Define local-day boundaries explicitly so “today” is stable across time-zone changes and daylight-saving transitions.
- Use version and integrity checks for persistent settings/history as needed; make interrupted NFC or power operations recoverable.
- Restrict debug logs to development builds, and measure their current impact.

## Open decisions

- Multi-source wake arbitration and the final integrated state machine.
- Watchdog behavior, brownout response, and production battery ADC policy.
- Reminder schedule, quiet hours, event separation, and user-adjustable target semantics.
- Final ST25DV NFC payload/update method and phone transaction flow.
