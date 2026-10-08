# Pebo HardwareTest firmware

This Arduino sketch is a PCBA bring-up firmware for UART-controlled hardware
checks. It is not the product firmware.

## Before flashing

Verify the PCBA schematic and update the pin configuration near the top of
`HardwareTest.ino`.

Current status:

- LED pins follow `docs/hardware_pinmap.md`.
- UART uses `PA9` TX and `PA10` RX.
- I2C uses `PB6` SCL and `PB7` SDA at 100 kHz.
- Piezo test uses `PA0` single-ended output while holding `PA1` low.
- LIS2DW12 and ST25DV read checks are implemented for bring-up.
- Battery ADC, RTC, and low-power checks are not yet implemented.

## Build and upload

Confirmed Arduino CLI path on the development PC:

```powershell
G:\Google Drive\Application\Arduino-cli\arduino-cli.exe
```

Use LTO because STM32L031G6Ux has only 32 KB flash and the I2C/UART bring-up
firmware is close to the limit:

```powershell
& "G:\Google Drive\Application\Arduino-cli\arduino-cli.exe" compile --fqbn "STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX,xserial=generic,usb=none,opt=oslto,dbg=none,rtlib=nano,upload_method=swdMethod" "Firmware\HardwareTest"
```

```powershell
& "G:\Google Drive\Application\Arduino-cli\arduino-cli.exe" upload --fqbn "STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX,xserial=generic,usb=none,opt=oslto,dbg=none,rtlib=nano,upload_method=swdMethod" "Firmware\HardwareTest"
```

The non-LTO default `opt=osstd` build does not currently fit in flash.

## UART

- Baud: `115200`
- Line ending: `\n` or `\r\n`
- Encoding: ASCII commands; text responses
- Current configured pins: `TX=PA9`, `RX=PA10`

The configured pins match the current Pebo PCBA design information. They must
also be verified on the actual assembled board:

- PC USB-UART RX must connect to MCU TX.
- PC USB-UART TX must connect to MCU RX.
- PC USB-UART GND must connect to PCBA GND.
- Logic level must match the PCBA VDD, normally 3.3 V or lower for this board.

Variant-supported UART options for STM32L031G6Ux:

| Peripheral | TX options | RX options |
| --- | --- | --- |
| LPUART1 | `PA2`, `PA14` | `PA3`, `PA13` |
| USART2 | `PA2_ALT1`, `PA9`, `PA14_ALT1`, `PB6` | `PA3_ALT1`, `PA10`, `PA15`, `PB7` |

The STM32 Arduino `Generic L031G6Ux` variant default `Serial` pins are
`TX=PA2`, `RX=PA3`, so `HardwareTest.ino` explicitly creates a UART on
`PA9/PA10`.

If the PCBA routes UART to a different pair, update `UART_TX_PIN`,
`UART_RX_PIN`, `UART_TX_NAME`, and `UART_RX_NAME` in `HardwareTest.ino`.

## Commands

```text
PING
INFO
STATUS
LED <BLUE1|BLUE2|BLUE3|BLUE4|RED|ALL> <ON|OFF|TOGGLE>
BLINK <BLUE1|BLUE2|BLUE3|BLUE4|RED|ALL> <COUNT> <MS>
BUZZ <FREQ_HZ> <MS>
I2CSCAN
LIS2INIT
LIS2
LIS2READ <REG_HEX> [LEN]
ST25
ST25READ <ADDR16_HEX> [LEN]
HELP
```

Responses start with one of:

- `READY`: firmware booted
- `OK`: command accepted
- `ERR`: command rejected
- `INFO`: board/firmware information
- `STATUS`: parseable key-value status fields
- `I2C_SCAN`: detected 7-bit I2C addresses
- `LIS2_INIT`: LIS2DW12 ODR/mode setup and control-register readback
- `LIS2`: LIS2DW12 probe results, including `WHO_AM_I`
- `LIS2_DATA`: LIS2DW12 raw status and output registers
- `ST25`: ST25DV ACK status and user memory bytes from address `0x0000`
- `ST25_READ`: ST25DV user memory read result

Example:

```text
> LED BLUE1 ON
< OK led
> STATUS
< STATUS uptime_ms=1234 commands=2 errors=0 led_BLUE1=ON ...
```

Device checks:

- `LIS2` checks address candidates `0x18` and `0x19`; `WHO_AM_I` is expected
  to be `0x44`.
- The firmware runs LIS2DW12 initialization at boot. `LIS2INIT` can be sent
  manually to set `CTRL1=0x31`, `CTRL2=0x0C`, and `CTRL6=0x00` again.
- `ST25` checks user memory address `0x53` and system address `0x57`; it reads
  the first 16 bytes from user memory address `0x0000`.
- `ST25READ` only reads ST25DV user memory and does not write EEPROM or change
  security/session settings.
