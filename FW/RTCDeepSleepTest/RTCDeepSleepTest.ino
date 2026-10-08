// Pebo RTC STOP-mode wake test.
// Target: STM32L031G6U6TR using STM32duino.
//
// Behavior:
// 1. Wake or boot.
// 2. Beep twice.
// 3. Stay awake for 5 seconds.
// 4. Enter STOP mode for about 10 seconds using the RTC wake-up timer.

#include <Arduino.h>

static const uint32_t PIN_PIEZO_A = PA0;
static const uint32_t PIN_PIEZO_B = PA1;
static const uint32_t PIN_ST25_LPD = PA4;

static const uint32_t PIN_LED_BLUE_1 = PA5;
static const uint32_t PIN_LED_BLUE_2 = PA6;
static const uint32_t PIN_LED_BLUE_3 = PA7;
static const uint32_t PIN_LED_BLUE_4 = PA8;
static const uint32_t PIN_LED_RED = PA15;

static const uint32_t PIN_UART_TX = PA9;
static const uint32_t PIN_UART_RX = PA10;
static const uint32_t PIN_I2C_SCL = PB6;
static const uint32_t PIN_I2C_SDA = PB7;
static const uint32_t PIN_ACC_INT1 = PB0;
static const uint32_t PIN_ACC_INT2 = PB1;
static const uint32_t PIN_NFC_GPO = PA3;

static const uint32_t AWAKE_MS = 5000;
static const uint32_t SLEEP_SECONDS = 10;

static RTC_HandleTypeDef rtcHandle;
static volatile bool rtcWakeObserved = false;

extern "C" void SystemClock_Config(void);

static void beep(uint16_t frequencyHz, uint16_t durationMs)
{
  tone(PIN_PIEZO_A, frequencyHz, durationMs);
  delay(durationMs);
  noTone(PIN_PIEZO_A);
}

static void wakeBeep()
{
  digitalWrite(PIN_PIEZO_B, LOW);
  beep(2600, 90);
  delay(80);
  beep(3200, 90);
}

static void errorBeep()
{
  for (uint8_t i = 0; i < 4; ++i) {
    beep(700, 80);
    delay(80);
  }
}

static void setupBoardPins()
{
  pinMode(PIN_PIEZO_A, OUTPUT);
  pinMode(PIN_PIEZO_B, OUTPUT);
  digitalWrite(PIN_PIEZO_A, LOW);
  digitalWrite(PIN_PIEZO_B, LOW);

  pinMode(PIN_ST25_LPD, OUTPUT);
  digitalWrite(PIN_ST25_LPD, HIGH);

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

static bool configureRtcClock()
{
  RCC_OscInitTypeDef osc = {};
  RCC_PeriphCLKInitTypeDef periph = {};

  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();

  osc.OscillatorType = RCC_OSCILLATORTYPE_LSE;
  osc.LSEState = RCC_LSE_ON;
  osc.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&osc) == HAL_OK) {
    periph.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    periph.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&periph) == HAL_OK) {
      __HAL_RCC_RTC_ENABLE();
      return true;
    }
  }

  osc = {};
  periph = {};
  osc.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  osc.LSIState = RCC_LSI_ON;
  osc.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    return false;
  }

  periph.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  periph.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&periph) != HAL_OK) {
    return false;
  }

  __HAL_RCC_RTC_ENABLE();
  return true;
}

static bool initRtc()
{
  if (!configureRtcClock()) {
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

  HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(RTC_IRQn);
  return true;
}

static bool armRtcWakeup(uint32_t seconds)
{
  HAL_RTCEx_DeactivateWakeUpTimer(&rtcHandle);
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&rtcHandle, RTC_FLAG_WUTF);
  __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();

  const uint32_t counter = (seconds == 0) ? 0 : (seconds - 1);
  return HAL_RTCEx_SetWakeUpTimer_IT(&rtcHandle,
                                     counter,
                                     RTC_WAKEUPCLOCK_CK_SPRE_16BITS) == HAL_OK;
}

static void preparePinsForStop()
{
  digitalWrite(PIN_PIEZO_A, LOW);
  digitalWrite(PIN_PIEZO_B, LOW);
  digitalWrite(PIN_LED_BLUE_1, LOW);
  digitalWrite(PIN_LED_BLUE_2, LOW);
  digitalWrite(PIN_LED_BLUE_3, LOW);
  digitalWrite(PIN_LED_BLUE_4, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_ST25_LPD, HIGH);

  pinMode(PIN_UART_TX, INPUT_ANALOG);
  pinMode(PIN_UART_RX, INPUT_ANALOG);
  pinMode(PIN_I2C_SCL, INPUT_ANALOG);
  pinMode(PIN_I2C_SDA, INPUT_ANALOG);
  pinMode(PIN_ACC_INT1, INPUT);
  pinMode(PIN_ACC_INT2, INPUT);
  pinMode(PIN_NFC_GPO, INPUT);
}

static void enterStopForRtcWake()
{
  if (!armRtcWakeup(SLEEP_SECONDS)) {
    errorBeep();
    delay(1000);
    return;
  }

  preparePinsForStop();
  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  HAL_ResumeTick();

  SystemClock_Config();
  setupBoardPins();
  rtcWakeObserved = true;
}

void setup()
{
  setupBoardPins();
  if (!initRtc()) {
    while (true) {
      errorBeep();
      delay(1000);
    }
  }
}

void loop()
{
  wakeBeep();
  delay(AWAKE_MS);
  rtcWakeObserved = false;
  enterStopForRtcWake();
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

