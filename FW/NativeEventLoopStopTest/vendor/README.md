# Vendored manufacturer sources

No Arduino core or wrapper is linked. Sources are unchanged copies extracted
from the locally installed STM32duino3.0.0 distribution only as an acquisition
source, not a runtime/build dependency. Each copied file SHA256 is pinned in
manifest.json; build.py verifies it. No network acquisition needed to build.

- HAL: STM32L0 HAL1.10.7, official component
  https://github.com/STMicroelectronics/stm32l0xx-hal-driver .
  Selected C sources and full matching headers; HAL/LICENSE.md BSD-3-Clause.
- Device: CMSIS Device L0 1.9.4, official component
  https://github.com/STMicroelectronics/cmsis-device-l0 .
  L031 header/startup and ST system template, Device/LICENSE.md Apache-2.0.
- Core: ARM CMSIS distribution6.3.0, Core(M) header version6.1.0,
  https://github.com/ARM-software/CMSIS_6 . GCC/Cortex-M0+ header subset;
  Core/LICENSE Apache-2.0 and copyright headers retained.
- Project HAL config/linker/application are outside vendor and authored here.
  No changes to vendor needed; no RTOS or new third-party binary library.
