// Pebo v0.1 piezo + LIS2DW12 interaction test.
// Target: STM32L031G6U6TR using STM32duino 3.0.0.
//
// Pin source: docs/hardware_pinmap.md.
// Piezo waveform: PA0, TIM2_CH1, 50% duty single-ended square wave.

#include <Arduino.h>
#include <Wire.h>
#include <HardwareTimer.h>

#include "../libraries/PeboLIS2DW12/src/PeboLIS2DW12.h"
#include "../libraries/PeboLIS2DW12/src/PeboLIS2DW12.cpp"

extern "C" void SystemClock_Config(void);

static constexpr uint32_t PIN_PIEZO_A = PA0;
static constexpr uint32_t PIN_PIEZO_B = PA1;
static constexpr uint32_t PIN_DEBUG_MARKER = PA2;
static constexpr uint32_t PIN_ACC_INT1 = PB0;
static constexpr uint32_t PIN_ACC_INT2 = PB1;
static constexpr uint32_t PIN_I2C_SCL = PB6;
static constexpr uint32_t PIN_I2C_SDA = PB7;
static constexpr uint32_t PIN_UART_TX = PA9;
static constexpr uint32_t PIN_UART_RX = PA10;
static constexpr uint32_t PIN_NFC_LPD = PA4;
static constexpr uint32_t PIN_NFC_GPO = PA3;
static constexpr uint32_t PIN_LED_BLUE_1 = PA5;
static constexpr uint32_t PIN_LED_BLUE_2 = PA6;
static constexpr uint32_t PIN_LED_BLUE_3 = PA7;
static constexpr uint32_t PIN_LED_BLUE_4 = PA8;
static constexpr uint32_t PIN_LED_RED = PA15;

static constexpr uint32_t UART_BAUD = 115200;
static constexpr uint32_t I2C_CLOCK_HZ = 100000;
static constexpr uint32_t PIEZO_BEEP_DURATION_MS = 500;
static constexpr uint32_t PIEZO_GAP_MS = 1500;
static constexpr uint32_t AFTER_BEEP_CHECK_MS = 300;
static constexpr uint32_t LIS2_POLL_MS = 10;
static constexpr uint8_t PIEZO_REPEATS_PER_FREQ = 3;
static constexpr uint16_t PIEZO_FREQUENCIES_HZ[] = {2000, 3000, 4000};

static constexpr uint8_t REG_CTRL1 = 0x20;
static constexpr uint8_t REG_CTRL2 = 0x21;
static constexpr uint8_t REG_CTRL3 = 0x22;
static constexpr uint8_t REG_CTRL4_INT1_PAD_CTRL = 0x23;
static constexpr uint8_t REG_CTRL5_INT2_PAD_CTRL = 0x24;
static constexpr uint8_t REG_CTRL6 = 0x25;
static constexpr uint8_t REG_WAKE_UP_THS = 0x34;
static constexpr uint8_t REG_WAKE_UP_DUR = 0x35;
static constexpr uint8_t REG_WAKE_UP_SRC = 0x38;
static constexpr uint8_t REG_ALL_INT_SRC = 0x3B;
static constexpr uint8_t REG_CTRL7 = 0x3F;

static Uart DebugUart(PIN_UART_RX, PIN_UART_TX);
static PeboLIS2DW12 Lis2(Wire);
static HardwareTimer PiezoTimer(TIM2);
static volatile bool lis2Int1Flag = false;

struct Lis2Observation {
  bool triggered;
  uint8_t wakeSrc;
  uint8_t allIntSrc;
};

static void lis2Int1Isr()
{
  lis2Int1Flag = true;
}

static void printHex8(uint8_t value)
{
  DebugUart.print("0x");
  if (value < 0x10) {
    DebugUart.print('0');
  }
  DebugUart.print(value, HEX);
}

static void setupAlwaysDefinedPins()
{
  pinMode(PIN_NFC_LPD, OUTPUT);
  digitalWrite(PIN_NFC_LPD, HIGH);

  pinMode(PIN_PIEZO_A, OUTPUT);
  pinMode(PIN_PIEZO_B, OUTPUT);
  digitalWrite(PIN_PIEZO_A, LOW);
  digitalWrite(PIN_PIEZO_B, LOW);

  pinMode(PIN_DEBUG_MARKER, OUTPUT);
  digitalWrite(PIN_DEBUG_MARKER, LOW);

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

static void beginDebugUart()
{
  DebugUart.begin(UART_BAUD);
}

static void beginI2c()
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  Wire.setSCL(PIN_I2C_SCL);
  Wire.setSDA(PIN_I2C_SDA);
  Wire.begin();
  Wire.setClock(I2C_CLOCK_HZ);
}

static void configureExtiInput()
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  pinMode(PIN_ACC_INT1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ACC_INT1), lis2Int1Isr, RISING);
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_ClearPendingIRQ(EXTI0_1_IRQn);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}

static bool lis2Write(uint8_t reg, uint8_t value)
{
  return Lis2.writeRegister(reg, value);
}

static bool configureLis2Wakeup()
{
  uint8_t whoAmI = 0;
  if (!Lis2.begin(PeboLIS2DW12::ADDRESS_HIGH) || !Lis2.readWhoAmI(whoAmI)) {
    DebugUart.println("ERROR LIS2_DETECT_FAIL");
    return false;
  }
  if (whoAmI != PeboLIS2DW12::WHO_AM_I_EXPECTED) {
    DebugUart.print("ERROR LIS2_WHOAMI=");
    printHex8(whoAmI);
    DebugUart.println();
    return false;
  }

  if (!lis2Write(REG_CTRL2, 0x40)) {
    return false;
  }
  delay(5);
  return lis2Write(REG_CTRL2, 0x0C) &&
         lis2Write(REG_CTRL3, 0x10) &&
         lis2Write(REG_CTRL4_INT1_PAD_CTRL, 0x20) &&
         lis2Write(REG_CTRL5_INT2_PAD_CTRL, 0x00) &&
         lis2Write(REG_CTRL6, 0x00) &&
         lis2Write(REG_WAKE_UP_THS, 0x02) &&
         lis2Write(REG_WAKE_UP_DUR, 0x00) &&
         lis2Write(REG_CTRL7, 0x20) &&
         lis2Write(REG_CTRL1, 0x20);
}

static void clearLis2InterruptSource()
{
  uint8_t ignored = 0;
  Lis2.readRegister(REG_WAKE_UP_SRC, ignored);
  Lis2.readRegister(REG_ALL_INT_SRC, ignored);
  lis2Int1Flag = false;
#if defined(EXTI)
  EXTI->PR = GPIO_PIN_0;
#endif
  HAL_NVIC_ClearPendingIRQ(EXTI0_1_IRQn);
}

static bool setLis2MotionHandlingEnabled(bool enabled)
{
  return lis2Write(REG_CTRL4_INT1_PAD_CTRL, enabled ? 0x20 : 0x00);
}

static void startPiezo(uint32_t frequencyHz)
{
  digitalWrite(PIN_DEBUG_MARKER, HIGH);
  // stopPiezo() returns PA0 to GPIO LOW. Force AF/PWM mode every beep so the
  // timer output is restored after the quiet gap.
  PiezoTimer.pause();
  PiezoTimer.setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PIN_PIEZO_A);
  PiezoTimer.setOverflow(frequencyHz, HERTZ_FORMAT);
  PiezoTimer.setCaptureCompare(1, 50, PERCENT_COMPARE_FORMAT);
  PiezoTimer.setCount(0);
  PiezoTimer.resumeChannel(1);
  PiezoTimer.resume();
}

static void stopPiezo()
{
  PiezoTimer.pauseChannel(1);
  PiezoTimer.pause();
  pinMode(PIN_PIEZO_A, OUTPUT);
  digitalWrite(PIN_PIEZO_A, LOW);
  digitalWrite(PIN_DEBUG_MARKER, LOW);
}

static Lis2Observation observeLis2(uint32_t windowMs)
{
  Lis2Observation observation = {false, 0, 0};
  const uint32_t deadline = millis() + windowMs;
  while (static_cast<int32_t>(deadline - millis()) > 0) {
    uint8_t wakeSrc = 0;
    uint8_t allIntSrc = 0;
    Lis2.readRegister(REG_WAKE_UP_SRC, wakeSrc);
    Lis2.readRegister(REG_ALL_INT_SRC, allIntSrc);
    observation.wakeSrc |= wakeSrc;
    observation.allIntSrc |= allIntSrc;
    if (lis2Int1Flag || (digitalRead(PIN_ACC_INT1) == HIGH) || wakeSrc || allIntSrc) {
      observation.triggered = true;
    }
    delay(LIS2_POLL_MS);
  }
  return observation;
}

static void printEventBegin(uint8_t seq, uint16_t frequencyHz)
{
  DebugUart.print("PIEZO_EVENT_BEGIN seq=");
  DebugUart.print(seq);
  DebugUart.print(" freq_hz=");
  DebugUart.print(frequencyHz);
  DebugUart.print(" duration_ms=");
  DebugUart.println(PIEZO_BEEP_DURATION_MS);
  DebugUart.flush();
}

static void printEventEnd(uint8_t seq, uint16_t frequencyHz,
                          const Lis2Observation &during,
                          const Lis2Observation &after)
{
  DebugUart.print("PIEZO_EVENT_END seq=");
  DebugUart.print(seq);
  DebugUart.print(" freq_hz=");
  DebugUart.print(frequencyHz);
  DebugUart.print(" lis2_during=");
  DebugUart.print(during.triggered ? 1 : 0);
  DebugUart.print(" lis2_after=");
  DebugUart.print(after.triggered ? 1 : 0);
  DebugUart.print(" wake_src=");
  printHex8(during.wakeSrc | after.wakeSrc);
  DebugUart.print(" all_int_src=");
  printHex8(during.allIntSrc | after.allIntSrc);
  DebugUart.println();
  DebugUart.flush();
}

static void endI2cForStop()
{
  Wire.end();
  pinMode(PIN_I2C_SCL, INPUT);
  pinMode(PIN_I2C_SDA, INPUT);
#if defined(__HAL_RCC_I2C1_CLK_DISABLE)
  __HAL_RCC_I2C1_CLK_DISABLE();
#endif
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

static void configureUnusedGpioForStop()
{
  GPIO_InitTypeDef gpio = {};
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  // PA4/ST25_LPD remains Output High. PA13/PA14 stay available for SWD.
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
             GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 |
             GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
             GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
             GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 |
             GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
             GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOC, &gpio);
}

static void disableUnusedPeripheralClocksForStop()
{
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
#if defined(__HAL_RCC_RTC_DISABLE)
  __HAL_RCC_RTC_DISABLE();
#endif
#if defined(__HAL_RCC_DBGMCU_CLK_DISABLE)
  __HAL_RCC_DBGMCU_CLK_DISABLE();
#endif
}

static void enterFinalStop()
{
  stopPiezo();
  clearLis2InterruptSource();
  setupAlwaysDefinedPins();
  endI2cForStop();
  endDebugUartForStop();
  detachInterrupt(digitalPinToInterrupt(PIN_ACC_INT1));
  configureUnusedGpioForStop();

#if defined(DBGMCU)
  DBGMCU->CR = 0;
#endif

  SysTick->CTRL = 0;
  HAL_SuspendTick();
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  disableUnusedPeripheralClocksForStop();

  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

void setup()
{
  setupAlwaysDefinedPins();
  beginDebugUart();
  beginI2c();
  configureExtiInput();

  DebugUart.println("BOOT PiezoLIS2InteractionTest");
  DebugUart.println("PIEZO_PWM timer=TIM2 channel=CH1 pin=PA0 duty_percent=50 drive=SINGLE_ENDED");
  if (!configureLis2Wakeup()) {
    DebugUart.println("PIEZO_TEST_SUMMARY total=0 false_triggers=0 result=FAIL");
    enterFinalStop();
  }
  clearLis2InterruptSource();

  uint8_t seq = 0;
  uint8_t rawFalseTriggers = 0;
  for (uint8_t f = 0; f < (sizeof(PIEZO_FREQUENCIES_HZ) / sizeof(PIEZO_FREQUENCIES_HZ[0])); ++f) {
    for (uint8_t repeat = 0; repeat < PIEZO_REPEATS_PER_FREQ; ++repeat) {
      ++seq;
      clearLis2InterruptSource();
      printEventBegin(seq, PIEZO_FREQUENCIES_HZ[f]);
      startPiezo(PIEZO_FREQUENCIES_HZ[f]);
      Lis2Observation during = observeLis2(PIEZO_BEEP_DURATION_MS);
      stopPiezo();
      Lis2Observation after = observeLis2(AFTER_BEEP_CHECK_MS);
      if (during.triggered || after.triggered) {
        ++rawFalseTriggers;
      }
      printEventEnd(seq, PIEZO_FREQUENCIES_HZ[f], during, after);
      delay(PIEZO_GAP_MS);
    }
  }

  uint8_t maskedPostSettleTriggers = 0;
  for (uint8_t f = 0; f < (sizeof(PIEZO_FREQUENCIES_HZ) / sizeof(PIEZO_FREQUENCIES_HZ[0])); ++f) {
    for (uint8_t repeat = 0; repeat < PIEZO_REPEATS_PER_FREQ; ++repeat) {
      clearLis2InterruptSource();
      setLis2MotionHandlingEnabled(false);
      startPiezo(PIEZO_FREQUENCIES_HZ[f]);
      delay(PIEZO_BEEP_DURATION_MS);
      stopPiezo();
      delay(AFTER_BEEP_CHECK_MS);

      clearLis2InterruptSource();
      setLis2MotionHandlingEnabled(true);
      clearLis2InterruptSource();
      delay(20);

      uint8_t wakeSrc = 0;
      uint8_t allIntSrc = 0;
      Lis2.readRegister(REG_WAKE_UP_SRC, wakeSrc);
      Lis2.readRegister(REG_ALL_INT_SRC, allIntSrc);
      const bool immediateTrigger = lis2Int1Flag ||
                                    (digitalRead(PIN_ACC_INT1) == HIGH) ||
                                    wakeSrc || allIntSrc;
      if (immediateTrigger) {
        ++maskedPostSettleTriggers;
      }
      clearLis2InterruptSource();
      delay(PIEZO_GAP_MS);
    }
  }

  DebugUart.print("LIS2_RAW_FALSE_TRIGGERS=");
  DebugUart.println(rawFalseTriggers);
  DebugUart.print("LIS2_MASKED_POST_SETTLE_TRIGGERS=");
  DebugUart.println(maskedPostSettleTriggers);
  DebugUart.print("MASK_POLICY_RESULT=");
  DebugUart.println(maskedPostSettleTriggers == 0 ? "PASS" : "FAIL");
  DebugUart.print("PIEZO_TEST_SUMMARY total=");
  DebugUart.print(seq);
  DebugUart.print(" false_triggers=");
  DebugUart.print(rawFalseTriggers);
  DebugUart.print(" result=");
  DebugUart.println((rawFalseTriggers == 0 && maskedPostSettleTriggers == 0) ? "PASS" : "FAIL");
  DebugUart.flush();
  enterFinalStop();
}

void loop()
{
}
