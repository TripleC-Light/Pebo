# Hardware baseline

## Selected parts and architecture

- MCU: STM32L031G6U6TR.
- Motion sensor: LIS2DW12TR on I²C, with an interrupt to wake the MCU.
- NFC: ST25DV04KC-JF6D3 target. A prototype may use ST25DV04K-JFR6D3 due to availability; record which variant was actually fitted and test variant-dependent behavior.
- RTC: external 32.768 kHz LSE, ABS07-120-32.768KHZ-T.
- Power: CR2032 in product; Nordic PPK2 can supply and measure the development board.
- Sound: piezo transducer.
- NFC antenna: roughly 34 mm PCB loop under development, with ferrite considered between coil and battery. Tune on the assembled PCB with battery, ferrite, and enclosure in place.

## Verified pin assignment

Current source of truth: `docs/hardware_pinmap.md`, Pebo v0.1, last updated
2026-10-08. LED bring-up and ST-LINK programming are reported as passed there;
remaining peripheral behavior still needs board-level test.

| Signal | MCU pin/net | External part pin | Schematic rev | Verified by |
| --- | --- | --- | --- | --- |
| I²C SCL/SDA | PB6 SCL, PB7 SDA | LIS2DW12 + ST25DV04KC-JF6D3 | Pebo v0.1 | Pin map; bus test pending |
| LIS2DW12 INT | PB0 INT1, PB1 INT2 | LIS2DW12 through R1/R2 100 ohm | Pebo v0.1 | Pin map; interrupt test pending |
| ST25DV GPO/LPD | PA3 GPO, PA4 LPD | ST25DV through R8 100 ohm / R9 0 ohm | Pebo v0.1 | Pin map; NFC test pending |
| Piezo drive | PA0 PIEZO_A, PA1 PIEZO_B | PA0 through R20 100 ohm; PA1 optional through R18 0 ohm | Pebo v0.1 | Pin map; piezo population/test pending |
| UART TX/RX | TX PA9, RX PA10 | Debug UART | Pebo v0.1 | Pin map; UART test pending |
| SWDIO/SWCLK/NRST | PA13 SWDIO, PA14 SWCLK, NRST | ST-LINK | Pebo v0.1 | ST-LINK programming verified |
| LSE pins | PC14 OSC32_IN, PC15 OSC32_OUT | ABS07-120-32.768KHZ-T, C6/C7 6 pF | Pebo v0.1 | Pin map; RTC test pending |
| Battery measurement | None documented | TBD | Pebo v0.1 | Not present in pin map |

## Items to verify on the board

- I²C currently starts with 10 kΩ pull-ups; measure bus capacitance and rise time before changing to 22 kΩ or altering clock speed. The intended clock is at most 100 kHz and may be lowered.
- Check unused GPIO and peripheral states during STOP, including ST25DV LPD/GPO behavior and LIS2DW12 interrupt polarity and latching.
- Check debug and USB-UART signals while PPK2 output is off. Prevent external High levels from back-powering the DUT.
- Confirm adequate decoupling and CR2032 voltage under buzzer and radio activity.
- Measure NFC antenna tuning and phone read range on the assembled device. Validate the actual ST25DV variant and RF configuration.
- The PPK2 supplies the development board; the production battery and holder still require testing with real cells.

## References to add

Store or link the exact schematic, BOM, PCB revision, manufacturer datasheets, and antenna test records here when available. Do not infer package pinout from the earlier chat example.
