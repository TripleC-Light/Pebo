# LED brightness validation (Pebo v0.1)

Status: STM32duino 3.0.0 builds PASS in both debug modes; bench acceptance pending.
Sketch: `FW/LedBrightnessTest/LedBrightnessTest.ino`.
Target: STM32L031G6Ux (UFQFPN28), STM32duino 3.0.0.
No product firmware or passed bring-up sketches are changed.

## PWM decision

Verified against the STM32duino **3.0.0** generic G-package variant:
[PeripheralPins.c](https://github.com/stm32duino/Arduino_Core_STM32/blob/3.0.0/variants/STM32L0xx/L031G%284-6%29U_L041G6U/PeripheralPins.c).

| LED | Pin | Hardware PWM mapping |
| --- | --- | --- |
| 1 | PA5 | AF5 TIM2_CH1 |
| 2 | PA6 | AF5 TIM22_CH1 |
| 3 | PA7 | AF5 TIM22_CH2 |
| 4 | PA8 | AF5 TIM2_CH1 |

PA5 and PA8 share the same compare channel and cannot independently display different hardware PWM duties. All LEDs therefore use one TIM21 update interrupt as a shared software PWM time base. No timer output alternate function or analogWrite is configured.
ST datasheet retrieval timed out: 未直接核對受限 PDF. Mapping evidence above is core source, not a claim of direct PDF verification.

Default PWM: 500 Hz, 20 slots per cycle, interrupt rate 10 kHz.
DIM: 25% (5 slots), BRIGHT: 100% constant High, OFF: constant Low.
`PEBO_LED_DIM_DUTY_PERCENT` supports 5..95 in 5% steps (including 20, 25, 30).
`PEBO_LED_PWM_HZ` supports 300..1000 Hz.
GPIOA BSRR writes affect PA5/PA6/PA7/PA8 only.
API indexes are 0..3; invalid indexes are ignored. Invalid levels become OFF.
Single/all updates preserve PRIMASK and are atomic with respect to PWM.
No delay-based PWM; millis only controls the test durations.

TIM21 is reserved for this test. TIM2/TIM22 are unused. UART uses USART2 on PA9/PA10, not timer alternate functions. SysTick supplies millis during the sequence and is stopped before STOP. This sketch must not be combined with another TIM21 owner without review.

## Sequence and shutdown

Individual LEDs: DIM 2 s, BRIGHT 2 s, OFF, in order.
All LEDs: DIM 3 s, BRIGHT 3 s, OFF.
Mixed: DIM DIM BRIGHT OFF for 3 s.
Progress steps 0..5: OFF/OFF/OFF/OFF; BRIGHT/OFF/OFF/OFF;
DIM/BRIGHT/OFF/OFF; DIM/DIM/BRIGHT/OFF;
DIM/DIM/DIM/BRIGHT; BRIGHT/BRIGHT/BRIGHT/BRIGHT. Each lasts 1 s.
Total holds: 31 s, plus small logging/transition overhead.
PEBO_LED_DEBUG=1 initializes PA9/PA10 UART at 115200 and logs states.
PEBO_LED_DEBUG=0 creates no debug UART object and does not initialize UART.

After the sequence: pause/detach TIM21, clear its IRQ and gate its clock,
LED outputs LOW, flush/end debug UART, analog/no-pull unused pins,
retain PA4 Output High and SWD pins, stop SysTick, disable/clear external IRQs,
enter low-power-regulator STOP. No intentional wake source; reset restarts test.
No RTC, LIS2, ST25 protocol or drinking algorithm is initialized.
This is a functional test, not a verified battery-life configuration.

## Reproducible build

Install Arduino CLI and STM32duino 3.0.0 using the official board index:
`https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json`.
FQBN from core 3.0.0 boards.txt:
`STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX`.

```text
arduino-cli compile --fqbn STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX --build-property build.extra_flags=-DPEBO_LED_DEBUG=1 FW/LedBrightnessTest
arduino-cli compile --fqbn STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX --build-property build.extra_flags=-DPEBO_LED_DEBUG=0 FW/LedBrightnessTest
```

Workflow `.github/workflows/led-brightness-build.yml` builds both modes and retains logs.
Verified CI run: https://github.com/TripleC-Light/Pebo/actions/runs/38491916717
Implementation commit: b40ae97e792f6d3f4492751225796445414f5a7d.

| Debug | Build | Flash bytes | Static RAM bytes |
| --- | --- | ---: | ---: |
| ON | PASS | 18700 | 1920 |
| OFF | PASS | 12264 | 1044 |

RAM figures are linked static allocation, not measured peak stack/heap use.
Device limits are 32768 bytes Flash / 8192 bytes RAM, not measured usage.

## Bench acceptance (pending)

Record commit, fitted board, supply voltage, UART/ST-LINK connections, and instruments.
Verify all four LEDs independently; all DIM/BRIGHT; mixed; all six progress steps;
visible brightness difference and no visible flicker. Scope all four signals:
nominal period 2 ms, DIM High 0.5 ms, BRIGHT constant High, OFF constant Low.
Check timing jitter with UART enabled and disabled.
After completion verify all LED pins LOW and PA4 HIGH during STOP.
Measure whole-board current with debug equipment disconnected and a stated supply.
Do not import another sketch's sleep-current measurement as this sketch's result.
