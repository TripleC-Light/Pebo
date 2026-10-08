# Pebo v0.1 — Hardware Pin Map / Codex Context

Last updated: 2026-10-09

## Source of truth
- MCU: STM32L031G6U6TR, UFQFPN28
- BOOT0 = Pin 27
- VSS = Pin 16 and Pin 28
- Current firmware environment: Arduino IDE + STM32duino
- ST-LINK / SWD programming verified at 4 MHz
- LED bring-up passed
- UART debug on PA9/PA10 verified at 115200 baud with HardwareTest firmware
- I2C scan verified on PB6/PB7 at 100 kHz
- LIS2DW12 ACK verified at 0x19; WHO_AM_I read as 0x44
- LIS2DW12 output verified after enabling ODR; `STATUS=0x01` and X/Y/Z raw values are non-zero
- ST25DV user/system I2C ACK verified at 0x53/0x57; user memory address 0x0000 read succeeded
- ST-LINK firmware updated; previous USB reconnect issue resolved
- Whole-board deep-sleep baseline measured at 2.8 V: **6.84 µA average** when ST25_LPD is actively driven High

## MCU pin allocation

| Physical Pin | MCU Pin | Pebo Function | Direction | Connected Device / Note |
|---:|---|---|---|---|
| 1 | VDD | STM32_VDD | Power | 3.0 V rail |
| 2 | PC14-OSC32_IN | LSE_IN | Analog | 32.768 kHz crystal |
| 3 | PC15-OSC32_OUT | LSE_OUT | Analog | 32.768 kHz crystal |
| 4 | NRST | RESET | Input | ST-LINK |
| 5 | VDDA | Analog supply | Power | 3.0 V |
| 6 | PA0 | PIEZO_A | Output | Piezo through R20 = 100 ohm |
| 7 | PA1 | PIEZO_B | Output | Optional differential piezo drive |
| 8 | PA2 | TEST / RESERVED | GPIO | Test point |
| 9 | PA3 | ST25_GPO | Input / EXTI | ST25DV GPO through R8 = 100 ohm |
| 10 | PA4 | ST25_LPD | Output | ST25DV LPD through R9 = 0 ohm; **must be driven High during deep sleep** |
| 11 | PA5 | LED_BLUE_1 | Output | Blue LED 1 |
| 12 | PA6 | LED_BLUE_2 | Output | Blue LED 2 |
| 13 | PA7 | LED_BLUE_3 | Output | Blue LED 3 |
| 14 | PB0 | LIS2DW12_INT1 | Input / EXTI | through R1 = 100 ohm |
| 15 | PB1 | LIS2DW12_INT2 | Input / EXTI | through R2 = 100 ohm |
| 16 | VSS | GND | Power | Ground |
| 17 | VDD | STM32_VDD | Power | 3.0 V rail |
| 18 | PA8 | LED_BLUE_4 | Output | Blue LED 4 |
| 19 | PA9 | UART_TX | Output | Debug UART TX |
| 20 | PA10 | UART_RX | Input | Debug UART RX |
| 21 | PA13 | SWDIO | Bidirectional | ST-LINK |
| 22 | PA14 | SWCLK | Input | ST-LINK |
| 23 | PA15 | LED_RED | Output | Red LED |
| 24 | PB3 | TEST / RESERVED | GPIO | Test point |
| 25 | PB6 | I2C_SCL | Open-drain | LIS2DW12 + ST25DV |
| 26 | PB7 | I2C_SDA | Open-drain | LIS2DW12 + ST25DV |
| 27 | BOOT0 | BOOT0 | Input | 47 kOhm pull-down to GND |
| 28 | VSS | GND | Power | Ground |

## LED mapping
GPIO HIGH turns the LED on.

| LED | MCU GPIO | Series resistor |
|---|---|---:|
| Blue 1 | PA5 | 330 ohm |
| Blue 2 | PA6 | 330 ohm |
| Blue 3 | PA7 | 330 ohm |
| Blue 4 | PA8 | 330 ohm |
| Red | PA15 | 1.6 kOhm |

## I2C bus
- PB6 = I2C1_SCL
- PB7 = I2C1_SDA
- Devices: LIS2DW12 + ST25DV04KC-JF6D3
- R6/R7 = 10 kOhm pull-up to VCC_3V0
- Plan: validate 10 kOhm first, then test 22 kOhm for lower power

## LIS2DW12
- SCL -> PB6
- SDA -> PB7
- INT1 -> PB0 through 100 ohm
- INT2 -> PB1 through 100 ohm
- VDD/VDDIO -> LIS2_VDD
- R3/R4 are optional 0 ohm links to LIS2_VDD, not pull-downs

## ST25DV04KC-JF6D3
- SDA -> PB7
- SCL -> PB6
- GPO -> PA3 through 100 ohm
- LPD -> PA4 through 0 ohm
- VCC/VDCG -> ST25_VDD
- V_EH -> test point
- AC0/AC1 -> NFC antenna matching network

### Deep-sleep requirement for ST25_LPD
Measured behavior on Pebo v0.1 at 2.8 V with ST-LINK disconnected:

- ST25_LPD not actively held High: ~56.68 µA average, ~130 µA peak every ~40 ms
- ST25_LPD actively driven High: **~6.84 µA average, ~16 µA peak every ~40 ms**

This is a confirmed board-level requirement:

- **Keep PA4 configured as GPIO output High before and during system deep sleep.**
- **Do not include PA4 in a blanket Analog/No-Pull conversion for unused GPIOs.**
- Any low-power helper that reconfigures GPIOA must explicitly exclude PA4 or restore PA4 High before entering STOP.

## Piezo
- PA0 -> R20 100 ohm -> Piezo A
- PA1 -> optional R18 0 ohm -> Piezo B
- Single-ended option: Piezo B -> R19 0 ohm -> GND
- R18 and R19 must not both be populated

## LSE / RTC
- Crystal: ABS07-120-32.768KHZ-T
- PC14 = OSC32_IN
- PC15 = OSC32_OUT
- C6 = 6 pF
- C7 = 6 pF

## SWD / UART
SWD:
- PA13 = SWDIO
- PA14 = SWCLK
- NRST
- VCC_3V0 / VTref
- GND

UART:
- PA9 = TX
- PA10 = RX

Bring-up status:
- Arduino IDE / OpenOCD uses 4 MHz SWD
- Arduino upload works and auto-runs firmware
- STM32CubeProgrammer can program successfully
- CubeProgrammer GUI still leaves target halted after programming; manual Software Reset > Run works

## Power-domain links
- R15 = 0 ohm: VCC_3V0 -> STM32_VDD
- R16 = 0 ohm: VCC_3V0 -> LIS2_VDD
- R17 = 0 ohm: VCC_3V0 -> ST25_VDD
- PPK2 will directly power and measure the whole board

## Current bring-up status
Completed:
1. STM32 programming via ST-LINK
2. Arduino IDE + STM32duino build/upload
3. SWD at 4 MHz
4. ST-LINK firmware update
5. LED GPIO test
6. Firmware auto-run after Arduino upload
7. UART debug on PA9/PA10
8. I2C scan on PB6/PB7
9. LIS2DW12 WHO_AM_I at I2C address 0x19
10. LIS2DW12 raw X/Y/Z output after ODR initialization
11. ST25DV I2C user/system address ACK and user memory read
12. STOP-mode whole-board low-power baseline: **6.84 µA average at 2.8 V with ST25_LPD High**

Next:
1. Identify unexpected I2C address 0x2D seen during HardwareTest scan
2. LIS2DW12 INT1 / INT2 behavior
3. Identify source of remaining ~40 ms / ~16 µA sleep peak
4. LSE / RTC
5. Piezo
6. STOP mode wake-up behavior
7. PPK2 subsystem-isolation profiling using R15/R16/R17

## Firmware pin constants suggestion

```cpp
constexpr uint32_t PIN_LED_BLUE_1 = PA5;
constexpr uint32_t PIN_LED_BLUE_2 = PA6;
constexpr uint32_t PIN_LED_BLUE_3 = PA7;
constexpr uint32_t PIN_LED_BLUE_4 = PA8;
constexpr uint32_t PIN_LED_RED    = PA15;

constexpr uint32_t PIN_PIEZO_A = PA0;
constexpr uint32_t PIN_PIEZO_B = PA1;

constexpr uint32_t PIN_ACC_INT1 = PB0;
constexpr uint32_t PIN_ACC_INT2 = PB1;

constexpr uint32_t PIN_NFC_GPO = PA3;
constexpr uint32_t PIN_NFC_LPD = PA4;

constexpr uint32_t PIN_I2C_SCL = PB6;
constexpr uint32_t PIN_I2C_SDA = PB7;

constexpr uint32_t PIN_UART_TX = PA9;
constexpr uint32_t PIN_UART_RX = PA10;

constexpr uint32_t PIN_SWDIO = PA13;
constexpr uint32_t PIN_SWCLK = PA14;

constexpr uint32_t PIN_TEST_PA2 = PA2;
constexpr uint32_t PIN_TEST_PB3 = PB3;
```

## Codex instruction
Treat this document as the current hardware source of truth for Pebo v0.1 firmware development.

Do not reassign GPIO pins unless the schematic is revised.
If firmware behavior conflicts with this document, stop and report the conflict before changing pin assignments.
For low-power firmware, preserve the confirmed ST25_LPD requirement: **PA4 must remain GPIO Output High during deep sleep.**
