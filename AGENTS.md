# Pebo project instructions

Pebo (沛寶) is a small, CR2032-powered drinking reminder mounted on a child's water bottle. Read `README.md` and the relevant files in `docs/` before changing firmware, hardware assumptions, or tests.

## Working rules

- Treat the current schematic, PCB, BOM, and measured board behavior as authoritative for the actual hardware. The pin names in conversation examples are **not** verified pin assignments.
- Keep confirmed decisions separate from proposals and measurements still pending. If documents conflict with the actual board, report the conflict and update the documents after resolving it.
- Target at least six months on a CR2032. Measure sleep, wake, NFC, buzzer, and average current; do not claim battery life from MCU sleep current alone.
- Prefer interrupt or event driven operation and low power states. Check peripheral and GPIO states, pull-ups, and external debug equipment for unwanted current and back-power paths.
- **Confirmed low-power rule:** on Pebo v0.1, PA4 / ST25_LPD must remain configured as GPIO Output High during deep sleep while ST25DV is powered. Do not include PA4 in generic Analog/No-Pull GPIO shutdown code. This requirement reduced measured whole-board sleep current at 2.8 V from about 56.68 µA to 6.84 µA.
- Keep NFC data format versioned and document exact byte order, bounds, validity, and commit behavior before implementing it. Do not assume an NFC reader sees fresh MCU data without a measured update path.
- For firmware changes, run the relevant build and automated checks. Run HIL checks when connected equipment is available; report skipped physical tests plainly.
- Keep UART logs parseable in development builds and avoid leaving costly debug output enabled in the battery-life configuration.
- Never assume example tool scripts or pin mappings exist. Inspect the repository and attached instruments before issuing flash, power, or NFC commands.
- If a relevant manufacturer PDF is inaccessible and has not been verified directly, explicitly say 「未直接核對受限 PDF」 in technical findings.

## Where to read

| Work | Reference |
| --- | --- |
| Components, schematic, pins | `docs/hardware.md` and `docs/hardware_pinmap.md` |
| Firmware and timekeeping | `docs/firmware-architecture.md` |
| NFC payload and interactions | `docs/nfc-protocol.md` |
| Drinking event detection | `docs/motion-detection.md` |
| Current and battery-life measurement | `docs/power-budget.md` |
| Instruments and automated checks | `docs/hil-test-plan.md` |

Update the relevant document when a decision is confirmed or changed.

## Cross-computer development

- Ignore MCP Bridge for routine development, environment readiness and handoff checks unless the user explicitly asks to work on it. Do not install, configure, launch or test it as a prerequisite for firmware work.

- The user develops on three computers. At session start, read `docs/handoff.md` and `docs/development-machines.md`, identify the current computer, and verify its Git and tool state.
- At every session end, including blocked sessions, update the handoff and current machine profile with completed work, changed files, branch/commit, validation, blockers and concrete next steps.
- Keep other machines' dated observations intact. Unknown identity, tools, ports or connections must remain unverified until checked; never copy another machine's readiness or COM ports as current facts.
- Preserve existing uncommitted work. Record whether changes were committed/pushed or only saved locally; do not claim synchronization from a matching cloud-folder path.
- Use `docs/handoff.md` as the canonical task handoff. Reconcile alternate handoff files when integrating branches, preserving their history.
- Record instrument/process state and report whether the handoff was saved. Never put credentials or secrets in these files.
