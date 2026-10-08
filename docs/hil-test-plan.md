# Bench automation plan

This is a test plan, not a claim that instrument control scripts, drivers, or test fixtures already exist.

| Instrument | Intended role | Setup prerequisite |
| --- | --- | --- |
| ST-LINK | Flash and debug STM32 | Verify model, connections, target Vref, and CLI |
| USB-UART | Structured development logs | Verify voltage levels, port, baud, and power-off isolation |
| Nordic PPK2 | DUT supply, cycle, current capture | Verify source mode, wiring, limits, and control interface |
| Rigol DHO924 | Voltage and timing waveforms | Verify USB/LAN interface, commands, probes, and grounding |
| PN532 | Automated NFC reader/writer | Verify module interface, library, antenna placement, and RF path |

## Suggested first regression set

1. Build and flash a known firmware image; capture version and reset reason.
2. Cold boot at a nominal voltage; verify initialization and no unintended reset.
3. Check STOP/sleep current after all interfaces are idle.
4. Trigger a motion interrupt; verify wake and return to low power state.
5. Replay labeled positive/negative motion traces and compare event counts.
6. Measure I²C clock, rise/fall time, and logic levels with the assembled board.
7. Read NFC identity and a coherent status snapshot with PN532.
8. Exercise valid and malformed NFC time-sync requests; verify RTC and acknowledgement.
9. Interrupt an NFC or status update; verify data recovery/read consistency.
10. Verify piezo frequency, duration, pattern, and supply droop.
11. Check sleep and startup at several source voltages within validated operating limits.
12. Compare PN532 results with manual iPhone and Android reading/writing.

Define numeric acceptance limits from datasheets, the schematic, and product requirements before automating PASS/FAIL. A previous chat's sample thresholds and GPIO names were illustrations, not approved limits or wiring.

## Test safety and reproducibility

- Record firmware commit, schematic/PCB revision, fitted NFC variant, instrument firmware, connections, supply voltage, and raw waveform/current artifacts.
- Ensure powered instruments do not feed an unpowered DUT through signal pins; verify power-off VDD before calling a test “cold boot.”
- Document scope ground reference and any differential measurement method for piezo drive.
- Run software-only checks where possible. Mark HIL as skipped unless actual connected equipment has been exercised.

## Implementation order

Once the board and instrument interfaces are available: discover devices, make a manual known-good measurement, implement a minimal capture/read script, then add assertions and unattended regression. Keep real instrument commands in `tools/` and runnable test cases in `tests/` after their interfaces are confirmed.
