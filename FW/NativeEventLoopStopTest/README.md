# NativeEventLoopStopTest

Native HAL feasibility experiment, not a decision to migrate all firmware.
Independent project; existing Arduino tests are preserved. Variant B retains
stage4 commands, diagnostic strings, counters and application TX buffer. No
reduced-diagnostic variant C was needed. No RTOS, effects, drinking recognition,
schedules, NFC application protocol, simulation or power measurements.

## Build and dependencies

Python3 standard library drives existing arm-none-eabi GCC; no GUI/CMake/Ninja
installation required. GCC is selected by PATH, ARM_GCC_BIN, or explicit
--toolchain-bin; no Arduino installation/path lookup is performed by this build.
Vendor files are local, unchanged, license-preserved and SHA256 checked before
compilation. See vendor/README.md and vendor/manifest.json for exact acquisition
source. Shared pure EventLogic.h and TimeLogic.h are repository source only;
neither includes or links Arduino. Full repository checkout is required.

Validated: xPack GCC14.2.1-1.1 (compiler14.2.1 20241119), newlib-nano4.4.0,
STM32L0 HAL1.10.7, CMSIS DeviceL0 1.9.4, CMSIS distribution6.3.0/Core(M)6.1.0.
HAL is BSD-3-Clause, Device/Core Apache-2.0; all original copyright headers and
component license files retained. The local package provided acquisition files;
it is not an implicit build dependency or a claimed upstream Git commit.
GCC tool binaries are not vendored: xPack GNU Arm Embedded GCC distribution,
GPLv3 with GCC Runtime Library Exception3.1 for covered runtime components.
Newlib has per-component licenses, not one blanket GCC license. Installed
distribution includes distro-info/licenses/gcc-14.2.1/COPYING3 and COPYING.RUNTIME,
and newlib-4.4.0/COPYING.NEWLIB; retain applicable notices for redistribution.

```powershell
# Set to your installed GCC bin directory, or put arm-none-eabi tools on PATH.
$env:ARM_GCC_BIN = '<installed-gcc-bin>'
python FW/NativeEventLoopStopTest/build.py --build-dir "$env:TEMP/Pebo-NativeEventLoopStopTest"
```

Optional comparison: append --baseline-elf pointing at a separately rebuilt
EventLoopStopTest.ino.elf (its matching .map/.bin must be beside it). Existing A
build: STM32duino3.0.0 FQBN
`STMicroelectronics:stm32:GenL0:pnum=GENERIC_L031G6UX,xserial=generic,usb=none,opt=oslto,dbg=none,rtlib=nano,upload_method=swdMethod`.
Do not change that configuration for the comparison.

Build outputs ELF/bin/hex/map, GCC stack-usage metadata when emitted, and
build-report.json with all commands, section/load addresses, SHA256, largest
symbols, dependencies and savings. Vector contents/strong IRQ handlers and
shared compile-time boundary tests are checked; these are not physical race
tests. Debug sections in ELF do not occupy target Flash. No downloads needed.

## Board and native replacements

STM32L031G6U6TR: Flash32KB@08000000, SRAM8KB@20000000. Same user supply2.8V,
LSE PC14/15 32768Hz and HSI16/PLL x4 /2 =32MHz as successful stage4. Actual ST
startup_stm32l031xx.s/system template, vector table atFlash, linker data copy,
bss clear, constructor arrays and1024B minimum stack; no startup backup reset.

| Arduino dependency | Native replacement |
| --- | --- |
| setup/loop/core startup | ST startup + main/initialize/service |
| Generic variant clock | Explicit HAL RCC/PWR HSI/PLL, same clock afterSTOP |
| pinMode/digitalRead/Write | HAL GPIO and documented pin masks |
| attachInterrupt/std::function table | HAL EXTI + explicit IRQ callbacks |
| Uart/Serial | HAL USART2 IT, static64-byte RX/TX rings |
| Wire | HAL I2C1 Mem_Read/Write, single-byte operations,100ms bounds |
| millis/core timer | HAL SysTick tick, excludesSTOP; RTC for actual deadline |
| core allocation/syscalls | No application allocation; heap0, _sbrk rejects |
| Arduino package HAL wrappers | Vendored ST HAL/CMSIS source, no core link |

USART2 uses PA9TX/PA10RX AF4,1152008N1/PCLK1, same pinmap pull-ups. L031 has
USART2 here, not USART1. I2C1 PB6/PB7 AF1/open-drain/no internal pull, external
10kohm. Fixed TIMINGR10E0474A uses baseline Wire timing-search inputs at32MHz:
analog filter50..260ns, digital0, rise640ns/fall20ns,100kHz target (integer
model period9998ns). No runtime timing-search table or generic bus recovery.
Actual SCL frequency/rise waveform not independently measured. This explicit
fixed-board timing choice contributes savings; not all savings are Arduino-only.

LIS2@19: PB0/EXTI0 rising, push-pull latched High; exactly baseline config
CTRL2=0C/CTRL3=10/CTRL4=20/CTRL5=0/CTRL6=0/THS2/DUR0/CTRL7=20/CTRL1=20.
Read38/3B in foreground; motion is not a drink. KC-JF6D3 GPO->100ohm->PA3,
no external pulls, Input/NOPULL rising. PA4/LPD OutputHigh in STOP; each ST
access lowers it for bounded HAL_Delay(1) startup then restoresHigh. This is
1..2ms tick-granularity vs baseline1ms microsecond delay, not busy GPIO polling.
GPO1 static11/field-change already required; dynamicgate2000=1; IT2005 read
clears; EH2002 b2 field sample. No EEPROM/password/RF management writes. No
GPO2 setting change. CMOS requires poweredVDCG; no voltage/waveform claim.
LPDHigh field-removal wake still not guaranteed; prior RF failure cause OPEN.

## IRQ, timer and backup ownership

| Source | Handler/priority | Resource |
| --- | --- | --- |
| SysTick | SysTick_Handler/3 | HAL awake tick only, no TIM2/TIM22 |
| RTC | IRQ2/0 | LSE,WUT1Hz reload0, EXTI20 |
| Motion | EXTI0_1 IRQ5/3 | PB0, no PB1 route |
| GPO | EXTI2_3 IRQ6/3 | PA3, independent ofPB0 |
| UART | USART2 IRQ28/3 | RX/TX IT rings, no DMA |
| I2C | foreground HAL polling | no enabled I2C IRQ or DMA channel |

RTC uses127/255 prescalers and the same owned WUT/flag handling. LSE startup
timeout5s, no silentLSI fallback. No date/backup-domain reset or default valid
date, no new BKP allocation; read stage3 BKP2schema544D0101/BKP3floor only,
core BKP1/HID4 left untouched. Invalid calendar remains timeValid0. Deadline
uses validated seconds-of-day, works without calibrated UTC. No wall-time
calibration command or time-retention claim from this test.

ISR records only source flags/saturating counts; UART IRQ moves bytes via HAL.
PRIMASK save/disable/DMB/restore protects ISR and foreground RMW. All sources
dispatch from a snapshot; new events survive. Before STOP: app TX drained,
UART bounded100ms flush/end, I2C ended, under PRIMASK disableSysTick and clear
only its owned pending tick, check mailbox/activepins/ownedEXTI/RTC/NVIC pending,
thenHAL WFI. No blanket pending clear. Wake restores clocks/tick; peripheral
ISRs do not require I2C/UART. No ordinary UART RX STOP wake assumed. Same100ms
retry,3 consecutive actual failures/128 service cap; quarantine does not stop
other sources. WUT callback + clock advance preflight required before run.
An LSE failure after sleeping has no independent watchdog rescue, as in A.

## UART and HIL

Commands/important fields are unchanged: help/status/probe/counters/counters
clear/mode debug/run1..60/off, LF/CRLF, fixed32-byte parser, oversized line
discard, NUL/range rejection. Bootdebug, no autoSTOP. Status field/lastSt are
last samples, not fresh RF evidence; source counters distinguish observations.
Off stops owned resources, preserves RTC; next run reinitializes. Run expiry
uses RTC notSysTick; replies batched, no continuous wake logs. RX flooding past
64-byte ring capacity is unsupported, as in A. No UART script adaptation needed.

Use existing `FW/EventLoopStopTest/tests/bench_verify.py --help` with the native
bin and a new result path. Confirm fresh Prolific UART and the same STLINK/ID425
before flashing. Example PC-02 values are COM5 and56FF6C066688555210351387;
these are dated observations, not universal port identities. Normal supported
flash: CubeProgrammerCLI2.11.0, SWD4MHz, under-reset, write image08000000,
verify/reset/run. Existing bench tool supplies those flags; no mass erase,
option-byte/RDP/power operations. It validates target/image and logs actual RX.

Baseline: --stimulus none --require-field-absent. Keep still/phone away for
run10/60 thenoff/restart and parser. Motion: --stimulus motion --window-seconds 30.
After RUN_READY/operator prompt, move1..2s then put down. Awake RF:
--stimulus awake-rf --window-seconds 45 --require-field-absent. STOP RF:
--stimulus rf --window-seconds 60 --require-field-absent. Keep field absent before
arming, then approach5s/remove; do not read IT for diagnostics mid-window.
Mixed: --stimulus mixed --window-seconds 60 --require-field-absent, motion first
then phone on separate prompts. Full flags require spaces, e.g. --window-seconds 60.
Each session ends off/debug and closes UART; no instrument control. Before/after
backup inspector is tests/inspect_state.py (--reference for equality check),
only while debug/off. Raw evidence is tests/results, not synthetic IRQ injection.

## Resource comparison (A vs equivalent B)

| ELF/map metric | A Arduino | B native HAL | Saving |
| --- | ---: | ---: | ---: |
| .text |28716|16656|12060|
| .rodata |2220|1208|1012|
| .data RAM and Flash initializer |324|100|224|
| .bss |2536|1252|1284|
| All loaded Flash incl vectors/data/other sections |31480|18164|13316 (42.30%)|
| Static RAM (.data+.bss) |2860|1352|1508 (52.73%)|
| Flash remaining |1288|14604| |
| RAM after static |5332|6840| |
| Minimum stack / heap reservation |1024 /512|1024 /0|not runtime usage|
| Unreserved gap after minimum/alignment |3792|5816|not safe stack proof|

B includes192B vector and8B unwind table; Flash total is computed from ELF load
addresses, not just bin length. A includes other vectors/constructor/runtime
sections; row sums need not equal whole Flash. Complete sections/largest
functions/objects in build-report.json; debug sections excluded from target.

Both: same GCC/HAL/newlib-nano, soft-float M0+,Os/LTO/function+data sections/GC,
integer snprintf/no forced float formatter, same diagnostic strings and640B app
TX+64B UART RX/TX. A has generic startup/variant/wrappers and full generic core
module configuration; B has explicit ST startup/-nostartfiles, limited HAL
configuration, fixed pin/peripheral selection and timing. B uses g3 debug info
(not loaded). C++17/noexceptions/nortti; no alternative A config or slim C.

Big A costs: pinMode702B, TwoWire begin784/endTransmission618/timing566B,
globalconstructor496B, generic pin maps, IRQ callback table320B RAM, DebugUart344B,
unused SerialLP1 global344B, __sf312B, Wire228B, serial_debug188B data. B retains
640B appTX, UART136B, I2C84B, sources84B, RX/TX64B each, RTC36B. B largest main
4220B afterLTO, consolebegin1076B/USART2IRQ840B/status664B, integer printf536B.
LTO merges/inlines/aliases functions: these examples cannot be summed into an
exact per-feature savings attribution. This is a concrete implementation/config
comparison, not proof every saved byte comes solely from removing Arduino.

B has no application allocation and no usable heap; _sbrk rejects all requests.
Newlib printf still links allocator support branches; symbol presence is not
evidence of runtime heap use. A Wire uses dynamic minimum32+32B buffers, outside
static totals. Stack/heap peaks not measured on either, minimum1024B is not a
safety proof. No runtime high-water/power/long-term accuracy claim.

## Validation and recommendation

Build/host/vector checks passed, repeated build binary SHA256 identical:
e7e4e403763de5e1175f6b891f278817262aa5712ac69deb7c38a43426a367c0.
A re-created a5cd6df4... and same31480/2860. Native baseline10/60/restart/parser,
RTC/backup equality and MCU routes passed. Actual stimulus evidence under
tests/results (all native image e7e4e403...):

| Session | Result and actual observations |
| --- | --- |
| native-baseline-20261025-01 | PASS10/60s,STOP10/60, no external ISR; off/re-run1; UART/parser/recovery |
| native-baseline-20261025-02 | PASS repeated10/60s after RF/motion; no external ISR/storm; off/re-run1 |
| native-motion-20261025-01 | PASS30s,STOP53, motion ISR/wake/confirmed26,handled52,errors0 |
| native-awake-rf-20261025-01 | PASS45s, GPO ISR/confirmed1,wake0,errors0 |
| native-stop-rf-20261025-01 | PASS60s,STOP62, GPO ISR/wake/confirmed2,lastSt10,errors0 |
| native-mixed-20261025-01 | PASS60s,STOP89,motion31/GPO1 ISR/wake/confirmed,errors0 |

All timed STOP runs return todebug on RTCdeadline, no unknownWake or source quarantine; script
thenoff and UARTclose. Counters can merge events; motion unknown26/31 are
extra service checks without new source flags, not more physical movements.
Source confirmed/wake/ISR and FIELD samples have the same meanings as A.
The raw scripts log version SHA, actualPC RX, round, fullreadback before each
existing-image session; no software IRQ injection or debug during timedSTOP.
Initial native compile caught wrong USART1 assumption beforeflash; verified
actual USART2 from installed pinmap/CMSIS. First link needed retained_sbrk stub
underLTO; both corrected before any HIL. No failed HIL hidden/replaced.

Before/after native inspection: all five backup words were0 and remain0,
PRER007F00FF unchanged, clock continues. Nonzero valid-schema retention has
NOT been physically tested this round; code performs no backup/date writes.
Current timeValid0 is correct, not an RTC validity or retention PASS.
Final-state-20261025.json again confirms all backup words/prescalers unchanged,
RTC WUT off, PA4High. Board remains native e7e4e403... off/debug, UART closed;
user supply unchanged. No instruments, option bytes, RDP or mass erase used.
GCC emits .su (e.g. main304B single frame); this is not a complete call-chain,
interrupt nesting or libc peak-stack bound. Peak not measured. Exact race,
simultaneous stimuli, electrical VDCG/GPO/SCL waveforms, I2C/clock faults,
unlimited motion and long-term stability/power remain unverified.

Native HAL has meaningful measured headroom in this prototype. Recommend it
as the candidate for a staged next decision, not automatic wholesale migration.
Before product migration: broaden HIL/stress, peak stack/clock/I2C fault tests,
review board-specific driver maintenance and low-power measurements. Existing
Arduino tests remain regression references. RF history cause remains OPEN.
