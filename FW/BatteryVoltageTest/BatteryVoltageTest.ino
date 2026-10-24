// Standalone Pebo v0.1 VDD accuracy test; no battery percentage estimate.
#include <Arduino.h>
#include "stm32l0xx_ll_adc.h"

#ifndef PEBO_BATTERY_DEBUG
#define PEBO_BATTERY_DEBUG 1
#endif
#ifndef PEBO_BATTERY_SAMPLES
#define PEBO_BATTERY_SAMPLES 32
#endif
#if !defined(STM32L031xx)
#error "BatteryVoltageTest targets STM32L031 only"
#endif
static_assert(PEBO_BATTERY_SAMPLES > 0 && PEBO_BATTERY_SAMPLES <= 4096,
              "Sample count must be 1..4096");

static Uart BatteryUART(PA10, PA9);
static ADC_HandleTypeDef batteryAdc = {};

// STM32duino owns the ADC MSP hooks. No analogRead call selects an external pin;
// its default NC pin makes core MSP initialization leave GPIOs untouched.

static void quietPins(bool keepUart)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  // Latch before mode change; PA4 never becomes analog.
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
  GPIO_InitTypeDef gpio = {};
  gpio.Pin = GPIO_PIN_4;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &gpio);
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pin = GPIO_PIN_All & ~(GPIO_PIN_4 | GPIO_PIN_13 | GPIO_PIN_14);
  if (keepUart) gpio.Pin &= ~(GPIO_PIN_9 | GPIO_PIN_10);
  HAL_GPIO_Init(GPIOA, &gpio);
  gpio.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOB, &gpio);
  HAL_GPIO_Init(GPIOC, &gpio);
}

static const char *measureBattery(uint32_t &raw, uint32_t &cal, uint32_t &mv)
{
  cal = *VREFINT_CAL_ADDR;
  if (cal == 0 || cal > 4095) return "INVALID_FACTORY_CAL";
  batteryAdc.Instance = ADC1;
  batteryAdc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  batteryAdc.Init.Resolution = ADC_RESOLUTION_12B;
  batteryAdc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  batteryAdc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  batteryAdc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  batteryAdc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  batteryAdc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  batteryAdc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  batteryAdc.Init.SamplingTime = ADC_SAMPLETIME_160CYCLES_5;
  batteryAdc.Init.LowPowerFrequencyMode =
      HAL_RCC_GetPCLK2Freq() / 4 < 2800000 ? ENABLE : DISABLE;
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  if (HAL_ADC_Init(&batteryAdc) != HAL_OK) return "ADC_INIT";
  if (HAL_ADCEx_Calibration_Start(&batteryAdc, ADC_SINGLE_ENDED) != HAL_OK)
    return "ADC_CALIBRATION";
  ADC_ChannelConfTypeDef channel = {};
  channel.Channel = ADC_CHANNEL_VREFINT;
  channel.Rank = ADC_RANK_CHANNEL_NUMBER;
  // HAL enables ADC_CCR_VREFEN (internal buffer/path).
  if (HAL_ADC_ConfigChannel(&batteryAdc, &channel) != HAL_OK) return "ADC_CHANNEL";
  delayMicroseconds(LL_ADC_DELAY_VREFINT_STAB_US);
  uint32_t sum = 0;
  // Discard the first conversion after enabling the internal path.
  for (uint32_t i = 0; i <= PEBO_BATTERY_SAMPLES; ++i) {
    if (HAL_ADC_Start(&batteryAdc) != HAL_OK) return "ADC_START";
    if (HAL_ADC_PollForConversion(&batteryAdc, 20) != HAL_OK) return "ADC_TIMEOUT";
    uint32_t value = HAL_ADC_GetValue(&batteryAdc);
    if (HAL_ADC_Stop(&batteryAdc) != HAL_OK) return "ADC_STOP";
    if (value == 0 || value >= 4095) return "ADC_RANGE";
    if (i != 0) sum += value;
  }
  raw = (sum + PEBO_BATTERY_SAMPLES / 2) / PEBO_BATTERY_SAMPLES;
  // Keep the fractional ADC mean until the final mV rounding.
  mv = (uint64_t(VREFINT_CAL_VREF) * cal * PEBO_BATTERY_SAMPLES + sum / 2) / sum;
  return nullptr;
}

static void shutdownAdc()
{
  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  if (batteryAdc.Instance) {
    HAL_ADC_Stop(&batteryAdc);
    CLEAR_BIT(ADC->CCR, ADC_CCR_VREFEN);
    HAL_ADCEx_DisableVREFINT();
    HAL_ADC_DeInit(&batteryAdc);
  }
  // Reset also handles a partially failed initialization/stop.
  __HAL_RCC_ADC1_FORCE_RESET();
  __HAL_RCC_ADC1_RELEASE_RESET();
  __HAL_RCC_ADC1_CLK_DISABLE();
  __HAL_RCC_SYSCFG_CLK_DISABLE();
}

static void prepareStop()
{
  BatteryUART.flush();
  BatteryUART.end();
  quietPins(false);
  __HAL_RCC_USART2_CLK_DISABLE();
  __HAL_RCC_LPUART1_CLK_DISABLE();
  __HAL_RCC_I2C1_CLK_DISABLE();
  __HAL_RCC_SPI1_CLK_DISABLE();
  __HAL_RCC_TIM2_CLK_DISABLE();
  __HAL_RCC_TIM21_CLK_DISABLE();
  __HAL_RCC_TIM22_CLK_DISABLE();
  __HAL_RCC_LPTIM1_CLK_DISABLE();
  __HAL_RCC_DMA1_CLK_DISABLE();
  __HAL_RCC_CRC_CLK_DISABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_DBGMCU_CLK_ENABLE();
  DBGMCU->CR = 0;
  __HAL_RCC_DBGMCU_CLK_DISABLE();
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  EXTI->IMR = 0;
  EXTI->PR = 0xFFFFFFFFUL;
  for (int irq = 0; irq < 32; ++irq) {
    NVIC_DisableIRQ(static_cast<IRQn_Type>(irq));
    NVIC_ClearPendingIRQ(static_cast<IRQn_Type>(irq));
  }
  HAL_SuspendTick();
  SysTick->CTRL = 0;
  SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
}

void setup()
{
  quietPins(true);
  BatteryUART.begin(115200);
  BatteryUART.println("BOOT BatteryVoltageTest");
  delay(250);
  uint32_t raw = 0, cal = 0, mv = 0;
  const char *error = measureBattery(raw, cal, mv);
  shutdownAdc();
  if (error) {
    BatteryUART.print("ERROR=");
    BatteryUART.println(error);
  } else {
#if PEBO_BATTERY_DEBUG
    BatteryUART.print("VREF_RAW="); BatteryUART.println(raw);
    BatteryUART.print("VREF_CAL="); BatteryUART.println(cal);
#endif
    BatteryUART.print("BATTERY_MV="); BatteryUART.println(mv);
#if PEBO_BATTERY_DEBUG
    BatteryUART.print("SAMPLE_COUNT="); BatteryUART.println(PEBO_BATTERY_SAMPLES);
#endif
  }
  prepareStop();
}

void loop()
{
  // No timed wake or second measurement; reset/power-cycle starts a new test.
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}
