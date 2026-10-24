# Pebo

Pebo is a bottle-mounted reminder that detects likely drinking motions and gives a short sound reminder when a configured window passes without a detected drink. The first version uses NFC for phone interaction and time sync; BLE is outside the current v1 plan.

This is a **project context starter**, not firmware or a verified schematic. It captures decisions from design discussions as of 2026-09-29. Add the actual schematic, BOM, pin map, board revision, datasheets, tool connection details, and measured results as they become available. No instrument or physical board has been tested in this package.

## Status

| Topic | Current direction | Status |
| --- | --- | --- |
| MCU | STM32L031G6U6TR | Selected for current design |
| Accelerometer | LIS2DW12TR | Selected for current design |
| NFC | ST25DV04KC-JF6D3 target; ST25DV04K-JFR6D3 prototype substitution | Verify actual assembled BOM and variant differences |
| Clock | ABS07-120-32.768KHZ-T external LSE for RTC | Selected for current design |
| Power and sound | CR2032 and piezo | Selected; electrical behavior to measure |
| Battery life | At least six months | Goal, unverified |
| I²C pull-ups | Initial 10 kΩ; evaluate 22 kΩ after measurement | Experimental |
| NFC antenna | About 34 mm PCB loop, ferrite near battery | Physical tuning pending |
| Drink detection | Motion-based event detector | Algorithm and thresholds pending |
| Mobile data | ID, battery, drinking information | Requirements; exact protocol pending |

## How to use this folder

1. Copy all contents into the root of the Pebo firmware repository. Keep `AGENTS.md` at that root.
2. Add the current schematic/BOM and fill the verified pin table in `docs/hardware.md` before writing pin-specific firmware.
3. Record any confirmed choices in the corresponding `docs/` file. Put measured values and test conditions alongside them.
4. Ask Codex to implement a narrow task against a named document, then review its build and physical test results.

## Handoff

When moving between development computers, start with `docs/handoff.md`. It
records the current workspace path, confirmed board bring-up status, critical
low-power rules, tool notes, and the next work queue.

Per-computer environment and readiness for the three development computers
are tracked in `docs/development-machines.md`. Identify the current computer
and verify its dated profile at the beginning of each session; update both
records before switching computers.

Suggested next task after the actual board information is added: “Compare the schematic and BOM with `docs/hardware.md`; update the verified pin map and flag any discrepancies without changing firmware.”



## Android development handoff

The existing Android engineering App and its protocol/Sample implementation
are saved under `Software/PeboAndroid`. Read that project's `README.md` for
portable setup, `PROTOCOL-v0.1.md` for architecture and provisional decisions,
and `docs/handoff.md` for canonical task status and cross-computer history.
A portable archive and SHA256 sidecar are under `Software/` for transfer.
Source is saved locally; verify cloud replication or file checksums before
using another computer. No automatic Git commit/push was performed.