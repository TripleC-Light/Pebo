# Power budget and measurement

Target: **at least six months** from a CR2032 under a stated use profile. No battery-life claim is verified yet.

## Measurement worksheet

| State/event | Current or charge | Frequency/duration | Conditions | Status |
| --- | --- | --- | --- | --- |
| Sleep including sensor/NFC/RTC | TBD | Continuous | Voltage, temperature | Pending |
| Motion wake and classification | TBD | TBD per day | Motion trace | Pending |
| Reminder piezo | TBD | TBD per day | Sound pattern, VDD | Pending |
| NFC and snapshot update | TBD | TBD per day | Reader and field | Pending |
| Battery measurement and logging | TBD | TBD per day | ADC, UART mode | Pending |

Estimate average current from measured charge per event plus baseline sleep current, then check cell capacity under actual pulse loads, end voltage, temperature, and storage/self-discharge conditions. A simple capacity divided by MCU sleep current is insufficient.

## Bench method

- Use PPK2 as development supply and current meter. Test startup, sleep, wake, sound, NFC, and voltage points relevant to the selected cell. Document current ranges and sampling configuration.
- Power-cycle by commanding PPK2 output and checking that VDD actually falls sufficiently before a cold-start test. Board capacitance and external signal back-power can invalidate it.
- Check ST-LINK reference/IO, USB-UART TX, PN532 wiring, and scope probe grounds for their electrical effect. A grounded scope probe must never be clipped onto a non-ground node.
- Repeat critical behavior with a real CR2032 and holder; a regulated supply does not reproduce the cell's pulse response.

## Open questions

Battery measurement topology, production debug connection, LPD control, I²C pull-up losses, antenna coupling effects, reminder duty cycle, and real usage frequency.
