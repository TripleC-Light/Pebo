# Pebo

Pebo is a bottle-mounted reminder that detects likely drinking motions and gives a short sound reminder when a configured window passes without a detected drink. The first version uses NFC for phone interaction and time sync; BLE is outside the current v1 plan.

This repository now contains verified Pebo v0.1 board bring-up firmware, measured low-power results, and focused functional tests. Treat the current schematic, `docs/hardware_pinmap.md`, measured board behavior, and the dedicated test sketches under `FW/` as the source of truth for implementation work.

## Current verified status

| Topic | Current direction | Status |
| --- | --- | --- |
| MCU | STM32L031G6U6TR | Programming, STOP mode, UART and GPIO verified |
| Accelerometer | LIS2DW12TR | I2C, WHO_AM_I, sampling and INT1 STOP wake verified |
| NFC | ST25DV04KC-JF6D3 | I2C, user memory read, LPD behavior and GPO STOP wake verified |
| Clock | ABS07-120-32.768KHZ-T external LSE | LSE + RTC + 10 s RTC Wakeup Timer STOP wake verified |
| Power and sound | CR2032 and piezo | Low-power board operation verified; piezo functional test pending |
| Low-power idle | Whole board at 2.8 V | ~4.43 µA measured in current functional wake tests |
| ST25 low-power rule | PA4 / ST25_LPD | Must remain GPIO Output High during STOP |
| Battery life | At least six months | Goal; full event-based budget still pending |
| I²C pull-ups | Initial 10 kΩ; evaluate 22 kΩ later | Experimental |
| NFC antenna | About 34 mm PCB loop, ferrite near battery | Physical tuning pending |
| Drink detection | Motion-based event detector | Wake path verified; algorithm/thresholds pending |
| Mobile data | ID, battery, drinking information | Requirements defined; exact protocol pending |

## Verified wake paths

- LIS2DW12 INT1 -> PB0 / EXTI -> STM32 STOP wake: **PASS**
- RTC Wakeup Timer -> RTC_IRQn -> STM32 STOP wake: **PASS**
- ST25DV GPO -> PA3 / EXTI -> STM32 STOP wake: **PASS**

## Handoff

When moving between development computers, start with `docs/handoff.md`. It
records the current workspace path, confirmed board bring-up status, critical
low-power rules, tool notes, and the next work queue.

Per-computer environment and readiness for the three development computers
are tracked in `docs/development-machines.md`. Identify the current computer
and verify its dated profile at the beginning of each session; update both
records before switching computers.




## Android development handoff

The existing Android engineering App and its protocol/Sample implementation
are saved under `Software/PeboAndroid`. Read that project's `README.md` for
portable setup, `PROTOCOL-v0.1.md` for architecture and provisional decisions,
and `docs/handoff.md` for canonical task status and cross-computer history.
A portable archive and SHA256 sidecar are under `Software/` for transfer.
Source is saved locally; verify cloud replication or file checksums before
using another computer. Git publication status is recorded in `docs/handoff.md`.
## How to use this repository

1. Read `AGENTS.md` and the relevant files under `docs/` before changing firmware.
2. Use `docs/hardware_pinmap.md` for verified Pebo v0.1 pin assignments.
3. Keep focused board-validation sketches under `FW/` separate from future integrated product firmware.
4. Record measured values and test conditions in the corresponding `docs/` file.
5. Preserve passed bring-up tests as regression references instead of overwriting them.

Piezo bring-up and blue LED progress results are recorded in `docs/handoff.md`;
NFC data exchange and multi-source wake integration remain follow-up work.
