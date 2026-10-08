# Pebo project instructions

Pebo (沛寶) is a small, CR2032-powered drinking reminder mounted on a child's water bottle. Read `README.md` and the relevant files in `docs/` before changing firmware, hardware assumptions, or tests.

## Working rules

- Treat the current schematic, PCB, BOM, and measured board behavior as authoritative for the actual hardware. The pin names in conversation examples are **not** verified pin assignments.
- Keep confirmed decisions separate from proposals and measurements still pending. If documents conflict with the actual board, report the conflict and update the documents after resolving it.
- Target at least six months on a CR2032. Measure sleep, wake, NFC, buzzer, and average current; do not claim battery life from MCU sleep current alone.
- Prefer interrupt or event driven operation and low power states. Check peripheral and GPIO states, pull-ups, and external debug equipment for unwanted current and back-power paths.
- Keep NFC data format versioned and document exact byte order, bounds, validity, and commit behavior before implementing it. Do not assume an NFC reader sees fresh MCU data without a measured update path.
- For firmware changes, run the relevant build and automated checks. Run HIL checks when connected equipment is available; report skipped physical tests plainly.
- Keep UART logs parseable in development builds and avoid leaving costly debug output enabled in the battery-life configuration.
- Never assume example tool scripts or pin mappings exist. Inspect the repository and attached instruments before issuing flash, power, or NFC commands.
- If a relevant manufacturer PDF is inaccessible and has not been verified directly, explicitly say 「未直接核對受限 PDF」 in technical findings.

## Where to read

| Work | Reference |
| --- | --- |
| Components, schematic, pins | `docs/hardware.md` |
| Firmware and timekeeping | `docs/firmware-architecture.md` |
| NFC payload and interactions | `docs/nfc-protocol.md` |
| Drinking event detection | `docs/motion-detection.md` |
| Current and battery-life measurement | `docs/power-budget.md` |
| Instruments and automated checks | `docs/hil-test-plan.md` |

Update the relevant document when a decision is confirmed or changed.
