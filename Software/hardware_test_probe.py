"""Command-line UART probe for Pebo HardwareTest firmware."""

import argparse
import sys
import time

import serial


DEFAULT_COMMANDS = ("PING", "INFO", "STATUS", "I2CSCAN", "LIS2", "ST25", "STATUS")


def read_available(port, timeout_s):
    deadline = time.time() + timeout_s
    lines = []
    buffer = bytearray()

    while time.time() < deadline:
        data = port.read(64)
        if not data:
            continue

        for value in data:
            if value in (10, 13):
                if buffer:
                    lines.append(buffer.decode("utf-8", errors="replace"))
                    buffer.clear()
            else:
                buffer.append(value)

    if buffer:
        lines.append(buffer.decode("utf-8", errors="replace"))

    return lines


def send_command(port, command, timeout_s):
    port.write((command + "\n").encode("ascii"))
    return read_available(port, timeout_s)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", required=True)
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--timeout", type=float, default=1.2)
    args = parser.parse_args()

    all_lines = []
    with serial.Serial(args.port, args.baud, timeout=0.1) as port:
      time.sleep(0.25)
      all_lines.extend(read_available(port, args.timeout))

      for command in DEFAULT_COMMANDS:
          print(f"> {command}")
          lines = send_command(port, command, args.timeout)
          for line in lines:
              print(f"< {line}")
          all_lines.extend(lines)

    has_pong = any(line == "OK PONG" for line in all_lines)
    has_uart_info = any("uart_tx=PA9" in line and "uart_rx=PA10" in line for line in all_lines)
    has_lis2 = any(line.startswith("LIS2 ") and "whoami=0x44" in line and "match=YES" in line for line in all_lines)
    has_st25_user = any(line.startswith("ST25 ") and "ack_user=YES" in line for line in all_lines)
    has_st25_system = any(line.startswith("ST25 ") and "ack_system=YES" in line for line in all_lines)

    print("RESULT "
          f"pong={'YES' if has_pong else 'NO'} "
          f"uart={'YES' if has_uart_info else 'NO'} "
          f"lis2={'YES' if has_lis2 else 'NO'} "
          f"st25_user={'YES' if has_st25_user else 'NO'} "
          f"st25_system={'YES' if has_st25_system else 'NO'}")

    return 0 if has_pong and has_uart_info else 1


if __name__ == "__main__":
    sys.exit(main())
