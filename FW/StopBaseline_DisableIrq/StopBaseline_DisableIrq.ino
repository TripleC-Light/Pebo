// Pebo v0.1 lowest-current STOP-mode baseline.
// Target: STM32L031G6U6TR using STM32duino.
//
// Purpose:
// - Verify the STM32L031 MCU branch can enter a true low-current STOP mode.
// - Do not initialize LIS2DW12, ST25DV, I2C, UART, RTC, or application logic.
// - Use this before adding any sensor/NFC/RTC/reminder behavior back in.
//
// Current wake sources:
// - No intentional periodic wake source is enabled.
// - NRST and debugger/SWD can still reset or halt the chip.
// - Any unexpected enabled interrupt would wake STOP; prepareForStopMode()
//   disables SysTick and clears common pending interrupt state before WFI.
// - RTC is intentionally not configured, so there is no timed wake-up path.

#include <Arduino.h>

extern "C" void SystemClock_Config(void);

static constexpr uint32_t PIN_PIEZO_A = PA0;
static constexpr uint32_t PIN_PIEZO_B = PA1;

static constexpr uint32_t PIN_LED_BLUE_1 = PA5;
static constexpr uint32_t PIN_LED_BLUE_2 = PA6;
static constexpr uint32_t PIN_LED_BLUE_3 = PA7;
static constexpr uint32_t PIN_LED_BLUE_4 = PA8;
static constexpr uint32_t PIN_LED_RED = PA15;

static constexpr uint32_t PIN_SWDIO = PA13;
static constexpr uint32_t PIN_SWCLK = PA14;

static void setKnownOutputsOff()
{
  pinMode(PIN_PIEZO_A, OUTPUT);
  pinMode(PIN_PIEZO_B, OUTPUT);
  digitalWrite(PIN_PIEZO_A, LOW);
  digitalWrite(PIN_PIEZO_B, LOW);

  pinMode(PIN_LED_BLUE_1, OUTPUT);
  pinMode(PIN_LED_BLUE_2, OUTPUT);
  pinMode(PIN_LED_BLUE_3, OUTPUT);
  pinMode(PIN_LED_BLUE_4, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_BLUE_1, LOW);
  digitalWrite(PIN_LED_BLUE_2, LOW);
  digitalWrite(PIN_LED_BLUE_3, LOW);
  digitalWrite(PIN_LED_BLUE_4, LOW);
  digitalWrite(PIN_LED_RED, LOW);
}

static void configureGpioAnalogNoPull()
{
  GPIO_InitTypeDef gpio = {};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  // GPIO set to Analog + No Pull:
  // - PA0/PA1 piezo: high-Z analog after being driven LOW, no DC drive.
  // - PA2 test, PA3 ST25_GPO, PA4 ST25_LPD.
  // - PA5/PA6/PA7/PA8/PA15 LEDs: off and high-Z analog.
  // - PA9/PA10 UART: not enabled, high-Z analog.
  // - PA11/PA12 if present on this package/variant: high-Z analog.
  // PA13/PA14 are intentionally excluded to preserve SWDIO/SWCLK.
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
             GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
             GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
             GPIO_PIN_12 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &gpio);

  // PB0/PB1 LIS2DW12 INT, PB3 test, PB6/PB7 I2C, and any unbonded PB pins:
  // Analog + No Pull. I2C is not initialized and internal pulls are disabled.
  gpio.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOB, &gpio);

  // PC14/PC15 LSE crystal pins are not used in this baseline because RTC is
  // intentionally disabled. They are set to Analog + No Pull with all PC pins.
  gpio.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOC, &gpio);
}

static void disableUnusedPeripheralClocks()
{
  // Peripheral clocks explicitly disabled for the baseline:
  // - USART/LPUART: no UART debug output.
  // - I2C1: LIS2DW12/ST25DV bus not initialized.
  // - SPI/TIM/LPTIM/ADC/DAC/CRC/DMA/FIREWALL: no application peripherals.
  // - RTC/LSE/LSI: no periodic wake source for lowest-current baseline.
  // - SYSCFG/DBGMCU sleep clock after EXTI cleanup; SWD pins remain untouched.
#if defined(__HAL_RCC_USART1_CLK_DISABLE) && defined(RCC_APB2ENR_USART1EN)
  __HAL_RCC_USART1_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_USART2_CLK_DISABLE)
  __HAL_RCC_USART2_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_LPUART1_CLK_DISABLE)
  __HAL_RCC_LPUART1_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_I2C1_CLK_DISABLE)
  __HAL_RCC_I2C1_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_SPI1_CLK_DISABLE)
  __HAL_RCC_SPI1_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_TIM2_CLK_DISABLE)
  __HAL_RCC_TIM2_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_TIM21_CLK_DISABLE)
  __HAL_RCC_TIM21_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_TIM22_CLK_DISABLE)
  __HAL_RCC_TIM22_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_LPTIM1_CLK_DISABLE)
  __HAL_RCC_LPTIM1_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_ADC1_CLK_DISABLE)
  __HAL_RCC_ADC1_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_DAC_CLK_DISABLE)
  __HAL_RCC_DAC_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_CRC_CLK_DISABLE)
  __HAL_RCC_CRC_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_DMA1_CLK_DISABLE)
  __HAL_RCC_DMA1_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_FIREWALL_CLK_DISABLE)
  __HAL_RCC_FIREWALL_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_RTC_DISABLE)
  __HAL_RCC_RTC_DISABLE();
#endif
#if defined(__HAL_RCC_SYSCFG_CLK_DISABLE)
  __HAL_RCC_SYSCFG_CLK_DISABLE();
#endif
#if defined(__HAL_RCC_DBGMCU_CLK_DISABLE)
  __HAL_RCC_DBGMCU_CLK_DISABLE();
#endif
}

static void disableSleepModeDebug()
{
#if defined(DBGMCU)
  // Do not keep the debug block alive in STOP/STANDBY/SLEEP. For current
  // measurement, disconnect ST-LINK or ensure it is not holding the target.
  DBGMCU->CR = 0;
#endif
}

static void clearWakeAndInterruptState()
{
  __HAL_RCC_PWR_CLK_ENABLE();

  // Clear power wake-up flag before STOP. This matters most for standby/wakeup
  // pin flows, but keeping it clean makes the baseline deterministic.
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

#if defined(EXTI)
  // Clear EXTI pending lines. This baseline does not enable EXTI wake sources.
  EXTI->PR = 0xFFFFFFFFUL;
#endif

  for (IRQn_Type irq = NonMaskableInt_IRQn; irq <= RTC_IRQn; irq = static_cast<IRQn_Type>(irq + 1)) {
    if (irq >= 0) {
      NVIC_ClearPendingIRQ(irq);
    }
  }
}

static void disablePeripheralInterrupts()
{
  // Experiment 2: disable all peripheral NVIC IRQ lines, not only pending bits.
  // SysTick is handled separately by prepareForStopMode() and remains fully off.
  // With RTC, TIM, LPTIM, EXTI, USART, and I2C IRQs disabled here, none of them
  // can be a periodic wake source for STOP mode in this baseline.
  for (uint8_t irq = 0; irq < 32; ++irq) {
    NVIC_DisableIRQ(static_cast<IRQn_Type>(irq));
    NVIC_ClearPendingIRQ(static_cast<IRQn_Type>(irq));
  }
}

void prepareForStopMode()
{
  setKnownOutputsOff();
  configureGpioAnalogNoPull();
  disableSleepModeDebug();
  clearWakeAndInterruptState();
  disablePeripheralInterrupts();
  disableUnusedPeripheralClocks();

  // Stop SysTick/millis periodic interrupt so it cannot wake the MCU.
  SysTick->CTRL = 0;
  HAL_SuspendTick();

  // Keep GPIO states latched in analog/no-pull, then gate GPIO clocks.
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
}

void enterStopMode()
{
  // Enter STM32L031 STOP mode with the low-power regulator using WFI.
  // This is STOP mode, not CPU sleep mode.
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

void restoreAfterWakeup()
{
  // If an unexpected interrupt wakes STOP, restore the system clock enough to
  // run the tiny loop, then immediately prepare for STOP again.
  SystemClock_Config();

  // Experiment 2 keeps SysTick fully disabled even after an unexpected wake.
  // No millis/delay service is restored in this baseline.
  SysTick->CTRL = 0;
  HAL_SuspendTick();
}

void setup()
{
  prepareForStopMode();
}

void loop()
{
  enterStopMode();
  restoreAfterWakeup();
  prepareForStopMode();
}
