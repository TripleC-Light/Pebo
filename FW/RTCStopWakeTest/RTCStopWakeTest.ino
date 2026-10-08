// Pebo v0.1 LSE + RTC + STOP wake test.
// Target: STM32L031G6U6TR using STM32duino.
//
// Pin source: docs/hardware_pinmap.md.
// This sketch does not use LIS2DW12, NFC data, drinking logic, or reminders.
//
// PEBO_RTC_STOP_WAKE=0: Phase 1, keep CPU awake and print RTC every second.
// PEBO_RTC_STOP_WAKE=1: Phase 2, RTC wake-up timer wakes STOP every 10 seconds.

#include <Arduino.h>

#define PEBO_RTC_DEBUG 1
#define PEBO_RTC_STOP_WAKE 1

extern "C" void SystemClock_Config(void);

static constexpr uint32_t PIN_NFC_LPD = PA4;
static constexpr uint32_t PIN_UART_TX = PA9;
static constexpr uint32_t PIN_UART_RX = PA10;

static constexpr uint32_t PIN_PIEZO_A = PA0;
static constexpr uint32_t PIN_PIEZO_B = PA1;
static constexpr uint32_t PIN_TEST_PA2 = PA2;
static constexpr uint32_t PIN_NFC_GPO = PA3;
static constexpr uint32_t PIN_LED_BLUE_1 = PA5;
static constexpr uint32_t PIN_LED_BLUE_2 = PA6;
static constexpr uint32_t PIN_LED_BLUE_3 = PA7;
static constexpr uint32_t PIN_LED_BLUE_4 = PA8;
static constexpr uint32_t PIN_LED_RED = PA15;
static constexpr uint32_t PIN_ACC_INT1 = PB0;
static constexpr uint32_t PIN_ACC_INT2 = PB1;
static constexpr uint32_t PIN_TEST_PB3 = PB3;
static constexpr uint32_t PIN_I2C_SCL = PB6;
static constexpr uint32_t PIN_I2C_SDA = PB7;

static constexpr uint32_t UART_BAUD = 115200;
static constexpr uint32_t RTC_WAKE_SECONDS = 10;
static constexpr uint32_t LSE_STARTUP_TIMEOUT_MS = 5000;

static RTC_HandleTypeDef rtcHandle;
static volatile bool rtcWakeObserved = false;
static uint32_t wakeCount = 0;

#if PEBO_RTC_DEBUG
static Uart DebugUart(PIN_UART_RX, PIN_UART_TX);
#endif

static void keepSt25InLowPowerDown()
{
  pinMode(PIN_NFC_LPD, OUTPUT);
  digitalWrite(PIN_NFC_LPD, HIGH);
}

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

  keepSt25InLowPowerDown();
}

#if PEBO_RTC_DEBUG
static void beginDebugUart()
{
  DebugUart.begin(UART_BAUD);
}

static void endDebugUartForStop()
{
  DebugUart.flush();
  DebugUart.end();
  pinMode(PIN_UART_TX, INPUT_ANALOG);
  pinMode(PIN_UART_RX, INPUT_ANALOG);
#if defined(__HAL_RCC_USART2_CLK_DISABLE)
  __HAL_RCC_USART2_CLK_DISABLE();
#endif
}

static void printTwoDigits(uint8_t value)
{
  if (value < 10) {
    DebugUart.print('0');
  }
  DebugUart.print(value);
}
#else
static void beginDebugUart() {}
static void endDebugUartForStop() {}
#endif

static bool waitForLseReady()
{
  const uint32_t start = millis();
  while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) == RESET) {
    if ((millis() - start) >= LSE_STARTUP_TIMEOUT_MS) {
      return false;
    }
  }
  return true;
}

static bool configureLseRtcClock()
{
  RCC_OscInitTypeDef osc = {};
  RCC_PeriphCLKInitTypeDef periph = {};

  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  // Reset backup domain so this firmware can explicitly select LSE as the RTC
  // clock source. Do not fall back to LSI: this test is specifically for LSE.
  __HAL_RCC_BACKUPRESET_FORCE();
  __HAL_RCC_BACKUPRESET_RELEASE();

  osc.OscillatorType = RCC_OSCILLATORTYPE_LSE;
  osc.LSEState = RCC_LSE_ON;
  osc.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    return false;
  }
  if (!waitForLseReady()) {
    return false;
  }

  periph.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  periph.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&periph) != HAL_OK) {
    return false;
  }

  __HAL_RCC_RTC_ENABLE();
  return true;
}

static bool initRtc()
{
  if (!configureLseRtcClock()) {
    return false;
  }

  rtcHandle.Instance = RTC;
  rtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  rtcHandle.Init.AsynchPrediv = 127;
  rtcHandle.Init.SynchPrediv = 255;
  rtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  rtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  rtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&rtcHandle) != HAL_OK) {
    return false;
  }

  RTC_TimeTypeDef time = {};
  time.Hours = 12;
  time.Minutes = 0;
  time.Seconds = 0;
  time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  time.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&rtcHandle, &time, RTC_FORMAT_BIN) != HAL_OK) {
    return false;
  }

  RTC_DateTypeDef date = {};
  date.WeekDay = RTC_WEEKDAY_FRIDAY;
  date.Month = RTC_MONTH_OCTOBER;
  date.Date = 9;
  date.Year = 26;
  if (HAL_RTC_SetDate(&rtcHandle, &date, RTC_FORMAT_BIN) != HAL_OK) {
    return false;
  }

  HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
  HAL_NVIC_ClearPendingIRQ(RTC_IRQn);
  HAL_NVIC_EnableIRQ(RTC_IRQn);
  return true;
}

static void readRtcTime(RTC_TimeTypeDef &time, RTC_DateTypeDef &date)
{
  HAL_RTC_GetTime(&rtcHandle, &time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&rtcHandle, &date, RTC_FORMAT_BIN);
}

#if PEBO_RTC_DEBUG
static void printRtcTime()
{
  RTC_TimeTypeDef time = {};
  RTC_DateTypeDef date = {};
  readRtcTime(time, date);

  DebugUart.print("RTC ");
  printTwoDigits(time.Hours);
  DebugUart.print(':');
  printTwoDigits(time.Minutes);
  DebugUart.print(':');
  printTwoDigits(time.Seconds);
  DebugUart.println();
}
#else
static void printRtcTime() {}
#endif

static void clearRtcWakeFlags()
{
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&rtcHandle, RTC_FLAG_WUTF);
  __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();
  HAL_NVIC_ClearPendingIRQ(RTC_IRQn);
}

static bool armRtcWakeupTimer(uint32_t seconds)
{
  HAL_RTCEx_DeactivateWakeUpTimer(&rtcHandle);
  clearRtcWakeFlags();

  // Wakeup Timer uses RTC_WAKEUPCLOCK_CK_SPRE_16BITS, i.e. ck_spre, the 1 Hz
  // RTC clock produced by LSE 32768 Hz divided by async/sync prescalers
  // 127/255. Reload value N wakes after N+1 seconds, so 10 seconds uses 9.
  const uint32_t reload = (seconds == 0) ? 0 : (seconds - 1);
  const HAL_StatusTypeDef status =
    HAL_RTCEx_SetWakeUpTimer_IT(&rtcHandle, reload, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);

  HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
  HAL_NVIC_ClearPendingIRQ(RTC_IRQn);
  HAL_NVIC_EnableIRQ(RTC_IRQn);
  return status == HAL_OK;
}

static void configureUnusedGpioForStop()
{
  GPIO_InitTypeDef gpio = {};
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  // PA4/ST25_LPD is explicitly excluded and remains GPIO Output High.
  // PA13/PA14 are excluded to preserve SWD. PA9/PA10 UART are analog only
  // after UART has been flushed and ended.
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
             GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 |
             GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
             GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOB, &gpio);

  // Do not configure PC14/PC15. They are OSC32_IN/OSC32_OUT for the active LSE.
}

static void disablePeripheralIrqExceptRtc()
{
  for (uint8_t irq = 0; irq < 32; ++irq) {
    const IRQn_Type irqType = static_cast<IRQn_Type>(irq);
    if (irqType == RTC_IRQn) {
      continue;
    }
    NVIC_DisableIRQ(irqType);
    NVIC_ClearPendingIRQ(irqType);
  }
  HAL_NVIC_ClearPendingIRQ(RTC_IRQn);
  HAL_NVIC_EnableIRQ(RTC_IRQn);
}

static void disableUnusedPeripheralClocksForStop()
{
  // Preserve PWR, RTC, LSE, and GPIO output state for PA4.
  // Disable active debug/peripheral clocks not needed for RTC STOP wake.
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
#if defined(__HAL_RCC_DBGMCU_CLK_DISABLE)
  __HAL_RCC_DBGMCU_CLK_DISABLE();
#endif
}

static void prepareForStopMode()
{
  keepSt25InLowPowerDown();
  endDebugUartForStop();
  configureUnusedGpioForStop();
  disablePeripheralIrqExceptRtc();
  clearRtcWakeFlags();

#if defined(DBGMCU)
  DBGMCU->CR = 0;
#endif

  // Stop SysTick/millis completely so Arduino timing cannot wake STOP.
  SysTick->CTRL = 0;
  HAL_SuspendTick();

  disableUnusedPeripheralClocksForStop();

  // Keep GPIO state latched. RTC/LSE continue from backup domain.
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  // Keep GPIOC untouched; PC14/PC15 are driven by the LSE oscillator.
}

static void enterStopMode()
{
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

static void restoreAfterWakeup()
{
  SystemClock_Config();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  keepSt25InLowPowerDown();

#if PEBO_RTC_DEBUG
  beginDebugUart();
#endif
}

void setup()
{
  setKnownOutputsOff();

#if PEBO_RTC_DEBUG
  beginDebugUart();
  DebugUart.println("BOOT RTCStopWakeTest");
#endif

  if (!initRtc()) {
#if PEBO_RTC_DEBUG
    DebugUart.println("ERR: LSE startup timeout");
    DebugUart.flush();
#endif
    while (true) {
      keepSt25InLowPowerDown();
    }
  }

#if PEBO_RTC_DEBUG
  DebugUart.println("LSE: READY");
  DebugUart.println("RTC: USING LSE");
#endif
}

void loop()
{
#if PEBO_RTC_STOP_WAKE
  if (!armRtcWakeupTimer(RTC_WAKE_SECONDS)) {
#if PEBO_RTC_DEBUG
    DebugUart.println("ERR: RTC wake timer setup failed");
    DebugUart.flush();
#endif
    while (true) {
      keepSt25InLowPowerDown();
    }
  }

#if PEBO_RTC_DEBUG
  DebugUart.println("ENTER STOP");
#endif
  prepareForStopMode();
  enterStopMode();
  restoreAfterWakeup();

#if PEBO_RTC_DEBUG
  if (rtcWakeObserved) {
    DebugUart.println("WAKE: RTC");
  } else {
    DebugUart.println("WAKE: UNKNOWN");
  }
  printRtcTime();
  DebugUart.print("WAKE_COUNT=");
  DebugUart.println(++wakeCount);
#else
  ++wakeCount;
#endif
  rtcWakeObserved = false;
#else
  printRtcTime();
  delay(1000);
#endif
}

extern "C" void RTC_IRQHandler(void)
{
  HAL_RTCEx_WakeUpTimerIRQHandler(&rtcHandle);
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
  (void)hrtc;
  rtcWakeObserved = true;
}

