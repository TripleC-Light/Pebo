# Development handoff

Updated: 2026-10-14 (Asia/Taipei).
Repository: TripleC-Light/Pebo.
Branch: codex/led-brightness-test.
Draft PR: https://github.com/TripleC-Light/Pebo/pull/1
Implementation commit: b40ae97e792f6d3f4492751225796445414f5a7d.
Use branch HEAD for documentation updates after that commit.

## Current work

Added standalone FW/LedBrightnessTest/LedBrightnessTest.ino.
Four independent OFF/DIM/BRIGHT outputs on PA5/PA6/PA7/PA8.
PA5 and PA8 share TIM2_CH1, so software PWM uses only TIM21.
Default 500 Hz, DIM 25%, BRIGHT 100%, 20 slots, 10 kHz IRQ.
Includes individual/all/mixed tests and six progress steps.
Debug UART 115200 on PA9/PA10; debug OFF initializes no UART.
STM32duino 3.0.0 concrete UART class is Uart (HardwareSerial is abstract).
Shutdown stops PWM, keeps LEDs LOW and PA4 Output High, stops SysTick,
enters STOP with no intentional wake source.

Added docs/led-brightness-test.md and debug ON/OFF build workflow.
AGENTS.md now requires saved handoffs at every session.
No passed bring-up or product firmware was edited.

## Validation and machine state

Local Windows project path: G:/我的雲端硬碟/Application/Project/Pebo.
Local process launch fails: helper_unknown_error: setup refresh had errors.
Local files/Git status/toolchain/instruments could not be inspected or synchronized.
All changes were made through GitHub on the branch above; local checkout is unverified.
GitHub main did not contain FW/BatteryVoltageTest at inspection time;
it may exist on another branch or in an unsynchronized local checkout. It was not created or modified.
ST datasheet fetch timed out: 未直接核對受限 PDF.
PWM mapping verified in STM32duino 3.0.0 G-package PeripheralPins.c.
Initial debug OFF CI passed: Flash 12264 bytes, static RAM 1044 bytes.
Initial debug ON failed due to abstract HardwareSerial; fixed to Uart.
Final debug ON/OFF CI: PASS for implementation commit b40ae97e792f6d3f4492751225796445414f5a7d.
Run: https://github.com/TripleC-Light/Pebo/actions/runs/38491916717
Debug ON: Flash 18700 / 32768 bytes, static RAM 1920 / 8192 bytes.
Debug OFF: Flash 12264 / 32768 bytes, static RAM 1044 / 8192 bytes.
No claim about peak stack/heap RAM. Workflow retains build-log artifacts.
CI report-write permission expansion was rejected by automatic review;
workflow retains contents: read and results were read via job logs.
Physical LED brightness/flicker/mixed/progress/STOP/PA4/current tests: NOT RUN.
No board flashed, no power or NFC commands issued.

## Next computer

1. Fetch and check out codex/led-brightness-test, preserving local changes.
2. Inspect local git status and any firmware missing from GitHub main.
3. Read docs/led-brightness-test.md and check PR build logs for final results.
4. Build with core 3.0.0 and FQBN STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX in both debug modes.
5. Rediscover ST-LINK, UART port and instruments before flashing.
6. Run visual/scope acceptance, check all LEDs LOW and PA4 HIGH in STOP,
   measure whole-board current and update the test document/handoff.

## Session history

- 2026-10-14: independently implemented LED test through GitHub because local
  executor was unavailable; added reproducible CI and cross-computer handoff rules.
