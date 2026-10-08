"""Pebo hardware confirmation GUI.

Python + Tkinter UART tool for the Firmware/HardwareTest sketch.
Requires pyserial:
    python -m pip install pyserial
"""

import math
import queue
import threading
import time
import tkinter as tk
from tkinter import messagebox, ttk

try:
    import serial
    from serial.tools import list_ports
except ImportError:  # Keep the GUI importable so it can explain the fix.
    serial = None
    list_ports = None


BAUD_RATES = ("115200", "57600", "38400", "19200", "9600")
LED_NAMES = ("BLUE1", "BLUE2", "BLUE3", "BLUE4", "RED", "ALL")


class SerialWorker:
    def __init__(self, rx_queue):
        self.rx_queue = rx_queue
        self.port = None
        self.thread = None
        self.running = False

    def connect(self, port_name, baud):
        if serial is None:
            raise RuntimeError("pyserial is not installed")

        self.disconnect()
        self.port = serial.Serial(port_name, baudrate=baud, timeout=0.1)
        self.running = True
        self.thread = threading.Thread(target=self._read_loop, daemon=True)
        self.thread.start()

    def disconnect(self):
        self.running = False
        if self.thread is not None:
            self.thread.join(timeout=0.5)
            self.thread = None
        if self.port is not None:
            self.port.close()
            self.port = None

    def send_line(self, text):
        if self.port is None or not self.port.is_open:
            raise RuntimeError("UART is not connected")
        self.port.write((text.strip() + "\n").encode("ascii"))

    def _read_loop(self):
        buffer = bytearray()
        while self.running and self.port is not None and self.port.is_open:
            try:
                data = self.port.read(64)
            except serial.SerialException as exc:
                self.rx_queue.put(("error", str(exc)))
                break

            if not data:
                continue

            for value in data:
                if value in (10, 13):
                    if buffer:
                        line = buffer.decode("utf-8", errors="replace")
                        self.rx_queue.put(("line", line))
                        buffer.clear()
                else:
                    buffer.append(value)

        self.running = False


class HardwareTestApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Pebo Hardware Test")
        self.geometry("1040x760")
        self.minsize(900, 640)

        self.rx_queue = queue.Queue()
        self.serial_worker = SerialWorker(self.rx_queue)
        self.status_vars = {}

        self.port_var = tk.StringVar()
        self.baud_var = tk.StringVar(value=BAUD_RATES[0])
        self.connection_var = tk.StringVar(value="Disconnected")
        self.command_var = tk.StringVar()
        self.led_var = tk.StringVar(value=LED_NAMES[0])
        self.buzz_freq_var = tk.StringVar(value="2000")
        self.buzz_ms_var = tk.StringVar(value="120")
        self.lis2_reg_var = tk.StringVar(value="0x0F")
        self.st25_addr_var = tk.StringVar(value="0x0000")
        self.read_len_var = tk.StringVar(value="16")
        self.stream_interval_var = tk.StringVar(value="200")
        self.lis2_streaming = False
        self.lis2_raw = (0, 0, 0)
        self.lis2_last_update_var = tk.StringVar(value="-")

        self._build_ui()
        self.refresh_ports()
        self.after(100, self._poll_rx_queue)
        self.protocol("WM_DELETE_WINDOW", self._on_close)

    def _build_ui(self):
        root = ttk.Frame(self, padding=12)
        root.pack(fill=tk.BOTH, expand=True)
        root.columnconfigure(0, weight=0)
        root.columnconfigure(1, weight=1)
        root.rowconfigure(3, weight=1)

        conn = ttk.LabelFrame(root, text="UART")
        conn.grid(row=0, column=0, columnspan=2, sticky="ew", pady=(0, 10))
        conn.columnconfigure(1, weight=1)

        ttk.Label(conn, text="Port").grid(row=0, column=0, padx=6, pady=8, sticky="w")
        self.port_combo = ttk.Combobox(conn, textvariable=self.port_var, width=24, state="readonly")
        self.port_combo.grid(row=0, column=1, padx=6, pady=8, sticky="ew")
        ttk.Button(conn, text="Refresh", command=self.refresh_ports).grid(row=0, column=2, padx=6, pady=8)

        ttk.Label(conn, text="Baud").grid(row=0, column=3, padx=6, pady=8, sticky="w")
        ttk.Combobox(conn, textvariable=self.baud_var, values=BAUD_RATES, width=10, state="readonly").grid(
            row=0, column=4, padx=6, pady=8
        )
        self.connect_button = ttk.Button(conn, text="Connect", command=self.toggle_connection)
        self.connect_button.grid(row=0, column=5, padx=6, pady=8)
        ttk.Label(conn, textvariable=self.connection_var).grid(row=0, column=6, padx=6, pady=8, sticky="w")

        actions = ttk.LabelFrame(root, text="Controls")
        actions.grid(row=1, column=0, sticky="nsew", padx=(0, 10), pady=(0, 10))

        ttk.Label(actions, text="LED").grid(row=0, column=0, padx=6, pady=6, sticky="w")
        ttk.Combobox(actions, textvariable=self.led_var, values=LED_NAMES, width=12, state="readonly").grid(
            row=0, column=1, padx=6, pady=6, sticky="ew"
        )
        ttk.Button(actions, text="On", command=lambda: self.send_led("ON")).grid(row=1, column=0, padx=6, pady=4, sticky="ew")
        ttk.Button(actions, text="Off", command=lambda: self.send_led("OFF")).grid(row=1, column=1, padx=6, pady=4, sticky="ew")
        ttk.Button(actions, text="Toggle", command=lambda: self.send_led("TOGGLE")).grid(
            row=2, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )
        ttk.Button(actions, text="Blink", command=self.send_blink).grid(
            row=3, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )

        ttk.Separator(actions).grid(row=4, column=0, columnspan=2, sticky="ew", pady=8)
        ttk.Label(actions, text="Buzzer Hz").grid(row=5, column=0, padx=6, pady=4, sticky="w")
        ttk.Entry(actions, textvariable=self.buzz_freq_var, width=10).grid(row=5, column=1, padx=6, pady=4, sticky="ew")
        ttk.Label(actions, text="Duration ms").grid(row=6, column=0, padx=6, pady=4, sticky="w")
        ttk.Entry(actions, textvariable=self.buzz_ms_var, width=10).grid(row=6, column=1, padx=6, pady=4, sticky="ew")
        ttk.Button(actions, text="Buzz", command=self.send_buzz).grid(
            row=7, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )

        ttk.Separator(actions).grid(row=8, column=0, columnspan=2, sticky="ew", pady=8)
        ttk.Button(actions, text="I2C Scan", command=lambda: self.send_command("I2CSCAN")).grid(
            row=9, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )
        ttk.Button(actions, text="Read LIS2DW12", command=self.send_lis2).grid(
            row=10, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )
        ttk.Label(actions, text="LIS2 reg").grid(row=11, column=0, padx=6, pady=4, sticky="w")
        ttk.Entry(actions, textvariable=self.lis2_reg_var, width=10).grid(row=11, column=1, padx=6, pady=4, sticky="ew")
        ttk.Button(actions, text="Read LIS2 Reg", command=self.send_lis2_read).grid(
            row=12, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )
        ttk.Label(actions, text="Stream ms").grid(row=13, column=0, padx=6, pady=4, sticky="w")
        ttk.Entry(actions, textvariable=self.stream_interval_var, width=10).grid(
            row=13, column=1, padx=6, pady=4, sticky="ew"
        )
        self.stream_button = ttk.Button(actions, text="Start LIS2 Stream", command=self.toggle_lis2_stream)
        self.stream_button.grid(row=14, column=0, columnspan=2, padx=6, pady=4, sticky="ew")
        ttk.Button(actions, text="Read ST25DV", command=self.send_st25).grid(
            row=15, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )
        ttk.Label(actions, text="ST25 addr").grid(row=16, column=0, padx=6, pady=4, sticky="w")
        ttk.Entry(actions, textvariable=self.st25_addr_var, width=10).grid(row=16, column=1, padx=6, pady=4, sticky="ew")
        ttk.Label(actions, text="Length").grid(row=17, column=0, padx=6, pady=4, sticky="w")
        ttk.Entry(actions, textvariable=self.read_len_var, width=10).grid(row=17, column=1, padx=6, pady=4, sticky="ew")
        ttk.Button(actions, text="Read ST25 Mem", command=self.send_st25_read).grid(
            row=18, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )

        ttk.Separator(actions).grid(row=19, column=0, columnspan=2, sticky="ew", pady=8)
        ttk.Button(actions, text="Read Status", command=lambda: self.send_command("STATUS")).grid(
            row=20, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )
        ttk.Button(actions, text="Info", command=lambda: self.send_command("INFO")).grid(
            row=21, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )
        ttk.Button(actions, text="Help", command=lambda: self.send_command("HELP")).grid(
            row=22, column=0, columnspan=2, padx=6, pady=4, sticky="ew"
        )

        right = ttk.Frame(root)
        right.grid(row=1, column=1, sticky="nsew", pady=(0, 10))
        right.columnconfigure(0, weight=1)
        right.rowconfigure(1, weight=1)

        status = ttk.LabelFrame(right, text="Hardware Status")
        status.grid(row=0, column=0, sticky="ew", pady=(0, 10))
        status.columnconfigure(1, weight=1)

        for row, key in enumerate(
            (
                "uptime_ms",
                "commands",
                "errors",
                "led_BLUE1",
                "led_BLUE2",
                "led_BLUE3",
                "led_BLUE4",
                "led_RED",
                "buzzer",
                "i2c_devices",
                "lis2",
                "st25_user",
                "st25_system",
                "nfc_gpo",
                "lis2_int1",
                "lis2_int2",
                "lis2_whoami",
                "lis2_x",
                "lis2_y",
                "lis2_z",
                "roll_deg",
                "pitch_deg",
                "tilt_deg",
                "st25_user0",
            )
        ):
            ttk.Label(status, text=key).grid(row=row, column=0, padx=8, pady=4, sticky="w")
            var = tk.StringVar(value="-")
            self.status_vars[key] = var
            ttk.Label(status, textvariable=var).grid(row=row, column=1, padx=8, pady=4, sticky="w")

        plot = ttk.LabelFrame(right, text="PCBA Attitude")
        plot.grid(row=1, column=0, sticky="nsew")
        plot.rowconfigure(0, weight=1)
        plot.columnconfigure(0, weight=1)
        self.lis2_canvas = tk.Canvas(plot, bg="#101418", highlightthickness=0, height=260)
        self.lis2_canvas.grid(row=0, column=0, sticky="nsew")
        ttk.Label(plot, textvariable=self.lis2_last_update_var).grid(row=1, column=0, padx=8, pady=4, sticky="w")
        self.lis2_canvas.bind("<Configure>", lambda _event: self._draw_lis2_vector())
        self._draw_lis2_vector()

        command = ttk.LabelFrame(root, text="Manual Command")
        command.grid(row=2, column=0, columnspan=2, sticky="ew", pady=(0, 10))
        command.columnconfigure(0, weight=1)
        entry = ttk.Entry(command, textvariable=self.command_var)
        entry.grid(row=0, column=0, padx=6, pady=8, sticky="ew")
        entry.bind("<Return>", lambda _event: self.send_manual_command())
        ttk.Button(command, text="Send", command=self.send_manual_command).grid(row=0, column=1, padx=6, pady=8)

        log_frame = ttk.LabelFrame(root, text="UART Log")
        log_frame.grid(row=3, column=0, columnspan=2, sticky="nsew")
        log_frame.rowconfigure(0, weight=1)
        log_frame.columnconfigure(0, weight=1)
        self.log_text = tk.Text(log_frame, wrap="word", height=12)
        self.log_text.grid(row=0, column=0, sticky="nsew")
        scrollbar = ttk.Scrollbar(log_frame, orient="vertical", command=self.log_text.yview)
        scrollbar.grid(row=0, column=1, sticky="ns")
        self.log_text.configure(yscrollcommand=scrollbar.set)

    def refresh_ports(self):
        if list_ports is None:
            self.port_combo.configure(values=())
            self._log("pyserial is missing. Install with: python -m pip install pyserial")
            return

        ports = [port.device for port in list_ports.comports()]
        self.port_combo.configure(values=ports)
        if ports and self.port_var.get() not in ports:
            self.port_var.set(ports[0])

    def toggle_connection(self):
        if self.serial_worker.port is not None:
            self.serial_worker.disconnect()
            self.connection_var.set("Disconnected")
            self.connect_button.configure(text="Connect")
            self._log("Disconnected")
            return

        port_name = self.port_var.get().strip()
        if not port_name:
            messagebox.showwarning("No port", "Select a UART port first.")
            return

        try:
            self.serial_worker.connect(port_name, int(self.baud_var.get()))
        except Exception as exc:
            messagebox.showerror("Connection failed", str(exc))
            return

        self.connection_var.set(f"Connected to {port_name}")
        self.connect_button.configure(text="Disconnect")
        self._log(f"Connected to {port_name} @ {self.baud_var.get()}")
        time.sleep(0.05)
        self.send_command("STATUS")

    def send_command(self, command):
        try:
            self.serial_worker.send_line(command)
            self._log(f"> {command}")
        except Exception as exc:
            messagebox.showerror("UART error", str(exc))

    def send_led(self, action):
        self.send_command(f"LED {self.led_var.get()} {action}")
        self.send_command("STATUS")

    def send_blink(self):
        self.send_command(f"BLINK {self.led_var.get()} 3 120")
        self.send_command("STATUS")

    def send_buzz(self):
        self.send_command(f"BUZZ {self.buzz_freq_var.get()} {self.buzz_ms_var.get()}")
        self.send_command("STATUS")

    def send_lis2(self):
        self.send_command("LIS2")
        self.send_command("STATUS")

    def send_lis2_read(self):
        self.send_command(f"LIS2READ {self.lis2_reg_var.get()} {self.read_len_var.get()}")

    def toggle_lis2_stream(self):
        if self.serial_worker.port is None or not self.serial_worker.port.is_open:
            messagebox.showwarning("UART disconnected", "Connect to the UART port first.")
            return

        self.lis2_streaming = not self.lis2_streaming
        self.stream_button.configure(text="Stop LIS2 Stream" if self.lis2_streaming else "Start LIS2 Stream")
        if self.lis2_streaming:
            self._poll_lis2_stream()

    def _poll_lis2_stream(self):
        if not self.lis2_streaming:
            return

        try:
            self.serial_worker.send_line("LIS2")
            self._log("> LIS2")
        except Exception as exc:
            self.lis2_streaming = False
            self.stream_button.configure(text="Start LIS2 Stream")
            messagebox.showerror("UART error", str(exc))
            return

        try:
            interval = max(100, min(5000, int(self.stream_interval_var.get())))
        except ValueError:
            interval = 200
            self.stream_interval_var.set(str(interval))

        self.after(interval, self._poll_lis2_stream)

    def send_st25(self):
        self.send_command("ST25")
        self.send_command("STATUS")

    def send_st25_read(self):
        self.send_command(f"ST25READ {self.st25_addr_var.get()} {self.read_len_var.get()}")

    def send_manual_command(self):
        command = self.command_var.get().strip()
        if command:
            self.send_command(command)
            self.command_var.set("")

    def _poll_rx_queue(self):
        while True:
            try:
                kind, payload = self.rx_queue.get_nowait()
            except queue.Empty:
                break

            if kind == "line":
                self._log(f"< {payload}")
                self._parse_status(payload)
                self._parse_probe(payload)
            else:
                self._log(f"! {payload}")
                self.connection_var.set("Serial error")

        self.after(100, self._poll_rx_queue)

    def _parse_status(self, line):
        if not line.startswith("STATUS "):
            return

        for field in line.split()[1:]:
            if "=" not in field:
                continue
            key, value = field.split("=", 1)
            if key in self.status_vars:
                self.status_vars[key].set(value)

    def _parse_probe(self, line):
        if line.startswith("I2C_SCAN "):
            for field in line.split():
                if field.startswith("count="):
                    self.status_vars["i2c_devices"].set(field.split("=", 1)[1])
            return

        if line.startswith("LIS2 "):
            fields = self._fields_from_line(line)
            if fields.get("match") == "YES":
                self.status_vars["lis2"].set("PRESENT")
            if "whoami" in fields and fields["whoami"] != "NA":
                self.status_vars["lis2_whoami"].set(fields["whoami"])
            return

        if line.startswith("LIS2_DATA "):
            fields = self._fields_from_line(line)
            raw_text = fields.get("xyz_raw")
            values = self._parse_hex_bytes(raw_text)
            if len(values) >= 6:
                x = self._int16_le(values[0], values[1])
                y = self._int16_le(values[2], values[3])
                z = self._int16_le(values[4], values[5])
                self.lis2_raw = (x, y, z)
                self.status_vars["lis2_x"].set(str(x))
                self.status_vars["lis2_y"].set(str(y))
                self.status_vars["lis2_z"].set(str(z))
                roll, pitch, tilt = self._angles_from_accel(x, y, z)
                self.status_vars["roll_deg"].set(f"{roll:.1f}")
                self.status_vars["pitch_deg"].set(f"{pitch:.1f}")
                self.status_vars["tilt_deg"].set(f"{tilt:.1f}")
                self.lis2_last_update_var.set(
                    f"raw x={x} y={y} z={z}  roll={roll:.1f} pitch={pitch:.1f} tilt={tilt:.1f}  "
                    f"{time.strftime('%H:%M:%S')}"
                )
                self._draw_lis2_vector()
            return

        if line.startswith("ST25 "):
            fields = self._fields_from_line(line)
            if "ack_user" in fields:
                self.status_vars["st25_user"].set("PRESENT" if fields["ack_user"] == "YES" else "NO")
            if "ack_system" in fields:
                self.status_vars["st25_system"].set("PRESENT" if fields["ack_system"] == "YES" else "NO")
            if "user0" in fields:
                self.status_vars["st25_user0"].set(fields["user0"])

    @staticmethod
    def _fields_from_line(line):
        fields = {}
        for token in line.split()[1:]:
            if "=" in token:
                key, value = token.split("=", 1)
                fields[key] = value
        return fields

    @staticmethod
    def _parse_hex_bytes(text):
        if not text or text == "NA":
            return []

        values = []
        for item in text.split(","):
            try:
                values.append(int(item, 0) & 0xFF)
            except ValueError:
                return []
        return values

    @staticmethod
    def _int16_le(low, high):
        value = low | (high << 8)
        if value & 0x8000:
            value -= 0x10000
        return value

    @staticmethod
    def _angles_from_accel(x, y, z):
        magnitude = math.sqrt((x * x) + (y * y) + (z * z))
        if magnitude <= 0:
            return 0.0, 0.0, 0.0

        roll = math.degrees(math.atan2(y, z))
        pitch = math.degrees(math.atan2(-x, math.sqrt((y * y) + (z * z))))
        z_ratio = max(-1.0, min(1.0, z / magnitude))
        tilt = math.degrees(math.acos(z_ratio))
        return roll, pitch, tilt

    def _project_3d(self, x, y, z, scale, cx, cy):
        px = cx + x * scale
        py = cy - z * scale + y * 0.22 * scale
        return px, py

    @staticmethod
    def _rotate_point(point, roll_deg, pitch_deg, yaw_deg=-34.0):
        x, y, z = point
        roll = math.radians(roll_deg)
        pitch = math.radians(pitch_deg)
        yaw = math.radians(yaw_deg)

        cos_r = math.cos(roll)
        sin_r = math.sin(roll)
        y, z = (y * cos_r) - (z * sin_r), (y * sin_r) + (z * cos_r)

        cos_p = math.cos(pitch)
        sin_p = math.sin(pitch)
        x, z = (x * cos_p) + (z * sin_p), (-x * sin_p) + (z * cos_p)

        cos_y = math.cos(yaw)
        sin_y = math.sin(yaw)
        x, y = (x * cos_y) - (y * sin_y), (x * sin_y) + (y * cos_y)
        return x, y, z

    def _draw_pcba_block(self, canvas, cx, cy, scale, roll, pitch):
        length = 1.8
        width = 1.05
        thickness = 0.16
        corners = (
            (-length, -width, -thickness),
            (length, -width, -thickness),
            (length, width, -thickness),
            (-length, width, -thickness),
            (-length, -width, thickness),
            (length, -width, thickness),
            (length, width, thickness),
            (-length, width, thickness),
        )
        rotated = [self._rotate_point(point, roll, pitch) for point in corners]
        projected = [self._project_3d(x, y, z, scale, cx, cy) for x, y, z in rotated]
        faces = (
            ((0, 1, 2, 3), "#27313d"),
            ((0, 1, 5, 4), "#384858"),
            ((1, 2, 6, 5), "#46586a"),
            ((2, 3, 7, 6), "#32404f"),
            ((3, 0, 4, 7), "#415164"),
            ((4, 5, 6, 7), "#5f7ea0"),
        )

        face_depths = []
        for indexes, color in faces:
            avg_y = sum(rotated[i][1] for i in indexes) / len(indexes)
            face_depths.append((avg_y, indexes, color))

        for _depth, indexes, color in sorted(face_depths):
            points = []
            for index in indexes:
                points.extend(projected[index])
            canvas.create_polygon(points, fill=color, outline="#dbeafe", width=1)

        top_center = self._rotate_point((0, 0, thickness + 0.42), roll, pitch)
        top_end = self._project_3d(*top_center, scale, cx, cy)
        top_origin = self._project_3d(*self._rotate_point((0, 0, thickness), roll, pitch), scale, cx, cy)
        canvas.create_line(*top_origin, *top_end, fill="#60a5fa", width=3, arrow=tk.LAST)
        canvas.create_text(top_end[0], top_end[1] - 8, text="Z", fill="#60a5fa", font=("Segoe UI", 10, "bold"))

        x_end = self._project_3d(*self._rotate_point((length + 0.45, 0, thickness), roll, pitch), scale, cx, cy)
        center = self._project_3d(*self._rotate_point((0, 0, thickness), roll, pitch), scale, cx, cy)
        canvas.create_line(*center, *x_end, fill="#f87171", width=3, arrow=tk.LAST)
        canvas.create_text(x_end[0] + 8, x_end[1], text="X", fill="#f87171", font=("Segoe UI", 10, "bold"))

        y_end = self._project_3d(*self._rotate_point((0, width + 0.45, thickness), roll, pitch), scale, cx, cy)
        canvas.create_line(*center, *y_end, fill="#34d399", width=3, arrow=tk.LAST)
        canvas.create_text(y_end[0] + 8, y_end[1], text="Y", fill="#34d399", font=("Segoe UI", 10, "bold"))

    def _draw_lis2_vector(self):
        canvas = self.lis2_canvas
        if not hasattr(self, "lis2_canvas"):
            return

        width = max(1, canvas.winfo_width())
        height = max(1, canvas.winfo_height())
        cx = width * 0.5
        cy = height * 0.6
        raw_x, raw_y, raw_z = self.lis2_raw
        roll, pitch, tilt = self._angles_from_accel(raw_x, raw_y, raw_z)
        block_scale = min(width, height) * 0.15

        canvas.delete("all")
        canvas.create_text(12, 12, anchor="nw", fill="#cbd5e1", text="PCBA attitude from LIS2DW12")
        canvas.create_text(
            12,
            34,
            anchor="nw",
            fill="#e5e7eb",
            text=f"roll {roll:.1f} deg   pitch {pitch:.1f} deg   tilt {tilt:.1f} deg",
        )
        canvas.create_text(
            12,
            56,
            anchor="nw",
            fill="#94a3b8",
            text="Yaw is fixed in this view; accelerometer-only data cannot determine yaw.",
        )
        self._draw_pcba_block(canvas, cx, cy, block_scale, roll, pitch)

    def _log(self, text):
        timestamp = time.strftime("%H:%M:%S")
        self.log_text.insert(tk.END, f"[{timestamp}] {text}\n")
        self.log_text.see(tk.END)

    def _on_close(self):
        self.serial_worker.disconnect()
        self.destroy()


if __name__ == "__main__":
    app = HardwareTestApp()
    app.mainloop()
