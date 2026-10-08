# Pebo hardware confirmation software

`hardware_test_gui.py` is a Python + Tkinter UART GUI for the
`Firmware/HardwareTest` sketch.

## Setup

Python 3 with Tkinter is required. Install pyserial:

```powershell
python -m pip install pyserial
```

Run:

```powershell
python Software\hardware_test_gui.py
```

Command-line probe after flashing:

```powershell
python Software\hardware_test_probe.py --port COM5 --baud 115200
```

## Features

- Select and refresh UART ports.
- Connect at 115200 baud by default.
- Send LED, blink, buzzer, info, help, and status commands.
- Run I2C scan.
- Probe and read LIS2DW12 registers.
- Continuously poll LIS2DW12 and draw a PCBA attitude block on a Tkinter canvas.
- Probe ST25DV and read user memory.
- Display parsed `STATUS` fields.
- Show raw UART logs for bring-up debugging.

## LIS2DW12 streaming

Connect to the UART port, then press `Start LIS2 Stream`.

- The GUI sends `LIS2` repeatedly.
- Default interval is `200 ms`; accepted GUI input is clamped from `100 ms` to
  `5000 ms`.
- The 3D view uses the six raw bytes from `LIS2_DATA xyz_raw=...` as signed
  little-endian X/Y/Z values.
- The GUI computes roll, pitch, and tilt-from-Z from the raw acceleration
  vector, then rotates a simple PCBA block to show the estimated attitude.
- Yaw is fixed in the visualization because accelerometer-only data cannot
  determine absolute yaw.
- This is a bring-up visualization, not a calibrated acceleration plot or a
  fused IMU orientation estimate.

## Expected firmware

The GUI expects newline-delimited text responses from the HardwareTest firmware:

```text
READY PeboHardwareTest proto=1
INFO name=PeboHardwareTest proto=1 baud=115200
STATUS uptime_ms=... commands=... errors=... led_BLUE1=...
I2C_SCAN 0x18 0x53 0x57 count=3
LIS2 addr=0x19 ack=YES whoami=0x44 match=YES
ST25 addr_user=0x53 ack_user=YES addr_system=0x57 ack_system=YES ...
RESULT pong=YES uart=YES lis2=YES st25_user=YES st25_system=YES
OK ...
ERR ...
```
