# NativeShakeProgressTest (stage 5 prototype)

Independent STM32L031G6U6TR (32 KB Flash / 8 KB SRAM) native HAL test.
No Arduino core, RTOS, drink recognition, progress calculation, audio, red LED,
NFC application protocol or automatic reminder scheduling. Progress is a
volatile UART test value, initially zero. Boot is debug, never automatic STOP.

## Recovery baseline and scope

Baseline commit: 820ecb3f2b9fb199ddf5cf95246698d4648b73b1.
Annotated tag: baseline/native-hal-eventloop-v1 (not pushed).
Native B image SHA256:
e7e4e403763de5e1175f6b891f278817262aa5712ac69deb7c38a43426a367c0.
Existing tests and baseline sources are unchanged. Stage5 is uncommitted.
Keep the entire repository: this project references sibling vendor and pure
policy headers; copying just this directory is not a standalone distribution.

## Reproducible build

Python 3 stdlib plus arm-none-eabi GCC 14.2.1-1.1 / newlib-nano 4.4.0.
Set ARM_GCC_BIN to the directory containing arm-none-eabi-gcc, or put it on PATH:

```powershell
python FW/NativeShakeProgressTest/build.py --toolchain-bin $env:ARM_GCC_BIN --build-dir "$env:TEMP/Pebo-NativeShakeProgressTest"
```

ELF/bin/hex/map/build-report.json are produced together. Os, LTO, section GC,
soft float, integer snprintf/nano/nosys; no dynamic application allocation.
Build validates actual ST source hashes, IRQ vectors and compile-time logic.
HAL 1.10.7 BSD-3-Clause, CMSIS Device 1.9.4 Apache-2.0, CMSIS Core(M) 6.1.0
(distribution 6.3) Apache-2.0: sources/licenses are the sibling native baseline
vendor directory. Additional unchanged HAL TIM/TIM_EX sources are pinned by
this directory's vendor/manifest.json and covered by the same HAL license.
Acquired from installed ST distribution; no installed Arduino package paths
are referenced at build time. No new download or software installation.

## Hardware and resource ownership

- USART2 PA9 TX / PA10 RX AF4, COM5 Prolific on PC-02, 115200 8N1.
- I2C1 PB6/7, external 10k pullups, 100 kHz, address 0x19 LIS2DW12.
- PB0 INT1: active High, latched, EXTI0 rising/no pull, IRQ priority3.
- ST25DV04KC-JF6D3: GPO via100ohm PA3, EXTI3 rising/no pull, IRQ priority3.
  No external pull. Existing powered CMOS/VDCG assumption remains conditional;
  actual VDCG voltage/all unpowered pin states have not been measured.
  GPO1=0x11, dynamic gate=1, read-to-clear IT_STATUS. No EEPROM writes.
- PA4 LPD Output High in STOP; scoped Low only for ST I2C, restored High.
- HSI16 PLL x4 /2 ->32MHz, LSE32768, RTC prediv127/255, WUT1Hz IRQ priority0.
  Backup domain is never reset. Existing BKP2 signature544D0101/BKP3 UTC floor
  are used only on explicit setutc; BKP0/1/4 untouched.
- LED1..4 PA5/PA6/PA7/PA8, High-on, cumulative order confirmed by prior user
  visual tests. No claim that this native port has already been visually accepted.
- Low: 1000Hz /25%, TIM2_CH1 AF5 for PA5 OR PA8, TIM22_CH1 AF5 PA6,
  TIM22_CH2 AF5 PA7. Prescaler31, ARR999, CCR250 at32MHz. High is GPIO High.
  At most one low LED; shared PA5/PA8 channel cannot create arbitrary patterns.
  This standalone module exclusively owns TIM2/TIM22, no PWM IRQ. Stop clears
  owned channel/timer, disables both clocks, restores four output-low pins.
  Future PA0 piezo TIM2_CH1 conflicts: explicit ownership/arbitration is required
  before sound integration. No general arbiter or sound is added here.

## UART

Newline terminated, fixed32-byte parser, rejects NUL and overlong lines.

| Command | Behavior |
| --- | --- |
| help / status | Commands; mode/source validity plus APP progress/display/sample/cooldown |
| counters / counters clear | Existing ISR/wake/source/handled counts plus SHAKE summary |
| progress 0..8 | Set volatile test level and immediately display |
| show | Restart current display; does not change progress |
| run 1..60 | RTC preflight, bounded automatic STOP test, returns debug |
| mode debug | Cancel sample/display and automatic STOP; UART remains available |
| off | Cancel effects and event sources; keep RTC/calendar/progress |
| probe | Debug-only RF configuration/field read; does NOT read IT_STATUS |
| clock | RTC validity, UTC epoch, LSE, backup markers, elapsed clock |
| ledregs | Read-only clock enables and GPIOA modes/outputs for cleanup evidence |
| lisregs | Bounded read-only CTRL1..6 diagnostic; no source register reads |
| setutc YYYY-MM-DD HH:MM:SS | Debug-only explicit UTC setting, 2000..2099 |
| trace | Debug-only bounded dump of last observation, at most160 measured XYZ rows |
| observe | Debug-only bounded1500ms diagnostic acquisition; requires LIS2 enabled (run then debug). NOT motion wake evidence |

Levels1..8 hold3s. Pattern: low/off/off/off, high/off/off/off,
high/low/off/off, high/high/off/off, high/high/low/off,
high/high/high/off, high/high/high/low, all high.
Zero is low LED1 120ms, off120ms, low120ms, then off. Off retains progress.
progress/show restart immediately; malformed commands don't change effects.
Sampling and display stay awake and service UART/RTC/GPO. Only run enters STOP.
Ordinary UART RX is NOT a STOP wake source. Run expiry cancels sample/display.

## Shake prototype (parameters are not product specifications)

Installation axes relative to bottle remain unconfirmed; algorithm uses XYZ
dot products and does not select a particular installed axis.
Idle configuration is exactly the verified 12.5Hz LP1/12-bit +/-2g wake setup.
Motion source confirmation starts a bounded1500ms foreground window, disables
only LIS2 interrupt routing and temporarily sets CTRL1=0x50 (100Hz LP1).
CTRL2 BDU/autoincrement remains0x0c, CTRL6=0. Fresh STATUS27 bit0 is required
before a single6-byte burst from28; signed12-bit = little-endian int16 /16.
Sensitivity0.976mg/count. Two fresh samples are discarded after switching;
no repeated old reading is treated as another sample.

Official references directly checked:
[DS11811](https://www.st.com/resource/en/datasheet/lis2dw12.pdf) and
[AN5038 Rev6](https://www.st.com/resource/en/application_note/dm00401877-lis2dw12-alwayson-3d-accelerometer-stmicroelectronics.pdf).
AN5038 section3.4 table12 describes LP1/CTRL6=00 output settling, with first
power-down conversion available after one sample period. Extra two-sample
discard is a conservative project choice, not a claim of a required ST value.

Detector compares acceleration with five fresh samples earlier (~50ms).
Constant gravity cancels; slow inclination usually has small differences.
Qualified vector magnitude is >=300 counts (~293mg); opposite-vector dot
product must be <-90000 count-squared, separated by >=80ms. Three reversals
within the window confirm once. Integer-only fixed five-sample lag.
After confirmed normal pickup/place false positives on16272d9c..., the prototype
also requires qualified vectors in a consistent hemisphere for20ms and repeated
motion spanning at least600ms from the first qualified lobe. Amplitude, ODR and
minimum reverse interval stay unchanged. Earlier firmware stopped trace at its
confirmation; rejecting the old45-sample pickup prefix proves delayed rejection
of that prefix only, NOT rejection of a complete newly sampled pickup window.
Old accepted prefixes may now be too short to validate a new positive decision;
collect new positive and negative windows before acceptance.
This heuristic is orientation-independent mathematically, not a validated
rejection guarantee for arbitrary drinking/rotation or deliberate cheating.

Cooldown2000ms starts at confirmation. Display-active suppresses new windows;
after success require >=500ms without serviced motion before a new entrance,
so continuous motion cannot keep restarting the same display indefinitely.
RTC SSR/calendar-derived accumulated milliseconds includes STOP, wraps midnight
and uint32 safely under the continuously serviced1Hz deadline. RTC adjustment
rebases its sample without advancing/reversing elapsed time. HAL tick only
controls effects while awake and service retry intervals, never proves sleep.
The elapsed accumulator requires service within half a day; it is not persisted.
Short backward samples are
rejected, not mapped into a full day. At most one read per awake HAL tick plus
bounded SSR/TR/DR/SSR consistency check follows
[ES0322 Rev5 section2.9.2](https://www.st.com/resource/en/errata_sheet/es0322-stm32l031xxl041xx-device-errata-stmicroelectronics.pdf).
The pre-fix auto03 transcript contains a spurious24h elapsed jump; its script
PASS only covered per-run differences, not total elapsed correctness. Do not
use that revision as accepted cooldown timing. A PC elapsed anchor regression
and actual backward-snapshot/midnight/wrap/rebase pure tests cover the fix.
Physical traces also exposed stale pre-STOP elapsed at the observation start
when HAL tick had not advanced yet (first fresh retained sample at386ms).
After wake and before motion observation the RTC elapsed read is forced,
independent of that awake read-rate guard. New trace regression requires the
first retained sample within100ms. Earlier b2ad066f gesture evidence is a
pre-fix reference, not final observation-timing acceptance.

Poll at most every2ms, no-fresh limit100ms, three failed transactions abort;
an independent1750ms awake HAL-tick guard bounds an RTC-stall observation too.
restore verified idle config with at most three attempts, quarantine on failure.
SPI/FIFO/interrupt-per-sample are not used. I2C HAL calls are bounded polling
transactions, not an unbounded acquisition loop. Increased awake current,
false positives, installation effects and battery budget remain unmeasured.

## Event and STOP safety

Same PRIMASK-protected event snapshot, saturating ISR counters, bounded source
retry/fault budgets and pending-checked WFI as native baseline. No I2C/UART in
ISR, all pending sources processed. Active sample/display/TX blocks STOP.
SysTick/UART/I2C shutdown and system clock restoration retain baseline order.
RTC/NVIC/EXTI resources remain unchanged; no software IRQ injection is used.
RF field detection is not NFC data exchange and never requests LED display.
Prior RF failure's historical root cause remains OPEN.

## Validation

Final2026-10-25 results: [VALIDATION.md](VALIDATION.md). Final image2ae93c94...
passed build/readback/UART/RTC-reset/STOP/RF/mixed regressions. User confirmed
all0..8 LED patterns,10/10 intentional shakes,0/10 normal pickup false displays,
0/3 slow tilt false displays and one non-extended continuous-shake display.
This is a single mounted-bottle prototype cohort, not production accuracy.

Requires Python pyserial already present. Use user-supplied board power only.
Never open PPK2/COM6/7 or measurement instruments. Exact target required:

```powershell
python FW/NativeShakeProgressTest/tests/bench_verify.py --programmer "C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe" --stlink 56FF6C066688555210351387 --uart-port COM5 --image "$env:TEMP/Pebo-NativeShakeProgressTest/NativeShakeProgressTest.bin" --results FW/NativeShakeProgressTest/tests/results/NEW-UNUSED-DIRECTORY --flash
```

Default auto: verify/readback, UART/LED/invalid/partial commands, explicit UTC
from PC UTC (NTP accuracy not verified), ordinary MCU reset + valid marker
retention, run10/60/off/re-run1. No backup reset/option-byte/mass-erase actions.
For an existing matching image omit --flash. Stimulus: --test motion|pickup|
tilt|continuous|rf|mixed|awake-rf --seconds30 (RF ideally60). Wait for
WINDOW_READY; motion1..2s then settle; RF reader unlocked, approach5s then
withdraw. RF tests require FIELD0 before arming. Off/debug/close in finally.
Run results distinguish actual counts; physical trial labels need operator
confirmation. Do not treat script completion as LED optics or statistical proof.
Use --test controls for finite awake UART/sample cancellation/off/restart checks.
Prefer5 physical trials per60s window with~8s spacing; human reaction time and
the setup delay made the original10-trial single window inadequate.
For slower physical operation use --test awake-motion or awake-pickup
--seconds90 for5 trials in debug (no STOP in that observation window).
This does NOT replace separate run-mode wake and return-to-STOP tests.

Measured trace replay:

```powershell
python FW/NativeShakeProgressTest/tests/replay.py PATH/trace.json --compiler "$env:ARM_GCC_BIN/arm-none-eabi-g++.exe" --expected confirmed --result PATH/replay.json
```

Compiled actual C++ detector evaluates captured XYZ/time, no Python imitation.
Synthetic compile-time tests verify boundaries only, not real recognition.
Need ten intentional shakes + ten normal pickup/place trials, slow tilt,
stillness and continuous shaking, with actual hits/false positives recorded.

## Memory / limits

Linker reserves minimum1024 stack, heap0 (_sbrk rejects allocation), 8KB total.
Minimum reservation is not proof of peak safety. No stack painting on live stack;
peak stack is NOT_MEASURED. Build report lists .text/.rodata/.data/.bss and
largest objects/functions; trace160*8=1280 bytes is intentional bounded debug
cost. UART appTX1024 plus64RX/64TX. Trace dump is finite and not normal output.
Main power-off retention is not promised: no separately verified VBAT supply.
Reset behavior restores volatile progress0/debug, never resets valid calendar.

Final Flash23996 bytes, static RAM3256 (.data104/.bss3152), remaining after
static4936; minimum stack1024/heap0. Peak stack and power remain unmeasured.
Actual logs: tests/results; current board off/debug, UART closed. Physical
LED optics are user-confirmed; electrical timing/fault/race stress remain open.
