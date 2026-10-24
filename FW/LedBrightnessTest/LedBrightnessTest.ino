// Pebo v0.1 blue LED brightness test.
// Target: STM32L031G6U6TR using STM32duino.
//
// Pin source: docs/hardware_pinmap.md.
// This sketch verifies only: four blue LEDs OFF/DIM/BRIGHT and a simple
// progress-display pattern. It does not implement RTC, LIS2DW12, ST25DV NFC
// behavior, drinking logic, or product firmware state.

#include <Arduino.h>
#include <HardwareTimer.h>

#ifndef PEBO_LED_DEBUG
#define PEBO_LED_DEBUG 1
#endif
#ifndef PEBO_LED_DIM_DUTY_PERCENT
#define PEBO_LED_DIM_DUTY_PERCENT 25
#endif
#ifndef PEBO_LED_BRIGHT_DUTY_PERCENT
#define PEBO_LED_BRIGHT_DUTY_PERCENT 100
#endif
#ifndef PEBO_LED_PWM_FREQUENCY_HZ
#define PEBO_LED_PWM_FREQUENCY_HZ 1000
#endif
#ifndef PEBO_LED_PWM_STEPS
#define PEBO_LED_PWM_STEPS 20
#endif

extern "C" void SystemClock_Config(void);

enum class LedLevel : uint8_t {
  OFF,
  DIM,
  BRIGHT
};

static constexpr uint32_t PIN_LED_BLUE_1 = PA5;
static constexpr uint32_t PIN_LED_BLUE_2 = PA6;
static constexpr uint32_t PIN_LED_BLUE_3 = PA7;
static constexpr uint32_t PIN_LED_BLUE_4 = PA8;
static constexpr uint32_t PIN_NFC_LPD = PA4;
static constexpr uint32_t PIN_UART_TX = PA9;
static constexpr uint32_t PIN_UART_RX = PA10;

static constexpr uint32_t UART_BAUD = 115200;
static constexpr uint32_t LED_GPIO_MASK = GPIO_PIN_5 | GPIO_PIN_6 |
                                          GPIO_PIN_7 | GPIO_PIN_8;
static constexpr uint16_t LED_GPIO_PINS[4] = {
  GPIO_PIN_5,
  GPIO_PIN_6,
  GPIO_PIN_7,
  GPIO_PIN_8
};

static volatile uint8_t ledDutyTicks[4] = {0, 0, 0, 0};
static volatile uint8_t pwmPhase = 0;
static HardwareTimer PwmTimer(TIM21);

#if PEBO_LED_DEBUG
static Uart DebugUart(PIN_UART_RX, PIN_UART_TX);
#endif

static uint8_t ticksForDutyPercent(uint8_t percent)
{
  if (percent >= 100) {
    return PEBO_LED_PWM_STEPS;
  }
  const uint16_t scaled = static_cast<uint16_t>(percent) * PEBO_LED_PWM_STEPS;
  return static_cast<uint8_t>((scaled + 99U) / 100U);
}

static uint8_t dutyTicksForLevel(LedLevel level)
{
  switch (level) {
    case LedLevel::DIM:
      return ticksForDutyPercent(PEBO_LED_DIM_DUTY_PERCENT);
    case LedLevel::BRIGHT:
      return ticksForDutyPercent(PEBO_LED_BRIGHT_DUTY_PERCENT);
    case LedLevel::OFF:
    default:
      return 0;
  }
}

void setBlueLed(uint8_t index, LedLevel level)
{
  if (index >= 4) {
    return;
  }
  noInterrupts();
  ledDutyTicks[index] = dutyTicksForLevel(level);
  interrupts();
}

void setAllBlueLeds(LedLevel level)
{
  const uint8_t duty = dutyTicksForLevel(level);
  noInterrupts();
  for (uint8_t i = 0; i < 4; ++i) {
    ledDutyTicks[i] = duty;
  }
  interrupts();
}

static void keepSt25LowPowerDown()
{
  pinMode(PIN_NFC_LPD, OUTPUT);
  digitalWrite(PIN_NFC_LPD, HIGH);
}

static void configureLedPins()
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  pinMode(PIN_LED_BLUE_1, OUTPUT);
  pinMode(PIN_LED_BLUE_2, OUTPUT);
  pinMode(PIN_LED_BLUE_3, OUTPUT);
  pinMode(PIN_LED_BLUE_4, OUTPUT);
  GPIOA->BSRR = static_cast<uint32_t>(LED_GPIO_MASK) << 16;
}

static void beginDebugUart()
{
#if PEBO_LED_DEBUG
  DebugUart.begin(UART_BAUD);
#endif
}

static void endDebugUartForStop()
{
#if PEBO_LED_DEBUG
  DebugUart.flush();
  DebugUart.end();
  pinMode(PIN_UART_TX, INPUT_ANALOG);
  pinMode(PIN_UART_RX, INPUT_ANALOG);
#if defined(__HAL_RCC_USART2_CLK_DISABLE)
  __HAL_RCC_USART2_CLK_DISABLE();
#endif
#endif
}

static void logLine(const char *message)
{
#if PEBO_LED_DEBUG
  DebugUart.println(message);
#else
  (void)message;
#endif
}

static void pwmTick()
{
  ++pwmPhase;
  if (pwmPhase >= PEBO_LED_PWM_STEPS) {
    pwmPhase = 0;
  }

  uint32_t setMask = 0;
  uint32_t resetMask = 0;
  for (uint8_t i = 0; i < 4; ++i) {
    if (pwmPhase < ledDutyTicks[i]) {
      setMask |= LED_GPIO_PINS[i];
    } else {
      resetMask |= LED_GPIO_PINS[i];
    }
  }
  GPIOA->BSRR = setMask | (resetMask << 16);
}

static bool beginSoftwarePwm()
{
  const uint32_t interruptHz = PEBO_LED_PWM_FREQUENCY_HZ * PEBO_LED_PWM_STEPS;

  PwmTimer.pause();
  PwmTimer.setOverflow(interruptHz, HERTZ_FORMAT);
  PwmTimer.attachInterrupt(pwmTick);
  PwmTimer.setInterruptPriority(1, 0);
  PwmTimer.resume();
  return PwmTimer.isRunning();
}

static void stopSoftwarePwm()
{
  PwmTimer.pause();
  PwmTimer.detachInterrupt();
}

static void runIndividualLedTest()
{
  for (uint8_t led = 0; led < 4; ++led) {
    setAllBlueLeds(LedLevel::OFF);
    delay(150);

#if PEBO_LED_DEBUG
    DebugUart.print("LED");
    DebugUart.print(led + 1);
    DebugUart.println(" DIM");
#endif
    setBlueLed(led, LedLevel::DIM);
    delay(2000);

#if PEBO_LED_DEBUG
    DebugUart.print("LED");
    DebugUart.print(led + 1);
    DebugUart.println(" BRIGHT");
#endif
    setBlueLed(led, LedLevel::BRIGHT);
    delay(2000);

    setBlueLed(led, LedLevel::OFF);
  }
}

static void runAllLedTest()
{
  logLine("ALL DIM");
  setAllBlueLeds(LedLevel::DIM);
  delay(3000);

  logLine("ALL BRIGHT");
  setAllBlueLeds(LedLevel::BRIGHT);
  delay(3000);

  logLine("ALL OFF");
  setAllBlueLeds(LedLevel::OFF);
  delay(300);
}

static void runMixedBrightnessTest()
{
  logLine("MIXED LED1=DIM LED2=DIM LED3=BRIGHT LED4=OFF");
  setBlueLed(0, LedLevel::DIM);
  setBlueLed(1, LedLevel::DIM);
  setBlueLed(2, LedLevel::BRIGHT);
  setBlueLed(3, LedLevel::OFF);
  delay(3000);
  setAllBlueLeds(LedLevel::OFF);
}

static void setProgressStep(uint8_t step)
{
  setAllBlueLeds(LedLevel::OFF);

  switch (step) {
    case 1:
      setBlueLed(0, LedLevel::BRIGHT);
      break;
    case 2:
      setBlueLed(0, LedLevel::DIM);
      setBlueLed(1, LedLevel::BRIGHT);
      break;
    case 3:
      setBlueLed(0, LedLevel::DIM);
      setBlueLed(1, LedLevel::DIM);
      setBlueLed(2, LedLevel::BRIGHT);
      break;
    case 4:
      setBlueLed(0, LedLevel::DIM);
      setBlueLed(1, LedLevel::DIM);
      setBlueLed(2, LedLevel::DIM);
      setBlueLed(3, LedLevel::BRIGHT);
      break;
    case 5:
      setAllBlueLeds(LedLevel::BRIGHT);
      break;
    case 0:
    default:
      break;
  }
}

static void runProgressDemo()
{
  static const char *const labels[] = {
    "STEP 0 OFF OFF OFF OFF",
    "STEP 1 BRIGHT OFF OFF OFF",
    "STEP 2 DIM BRIGHT OFF OFF",
    "STEP 3 DIM DIM BRIGHT OFF",
    "STEP 4 DIM DIM DIM BRIGHT",
    "STEP 5 BRIGHT BRIGHT BRIGHT BRIGHT"
  };

  for (uint8_t step = 0; step < 6; ++step) {
    logLine(labels[step]);
    setProgressStep(step);
    delay(1000);
  }
  setAllBlueLeds(LedLevel::OFF);
}

static void enterStopAfterTest()
{
  setAllBlueLeds(LedLevel::OFF);
  delay(10);
  stopSoftwarePwm();
  GPIOA->BSRR = static_cast<uint32_t>(LED_GPIO_MASK) << 16;
  keepSt25LowPowerDown();
  endDebugUartForStop();

  SysTick->CTRL = 0;
  HAL_SuspendTick();

#if defined(DBGMCU)
  DBGMCU->CR = 0;
#endif

  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

void setup()
{
  keepSt25LowPowerDown();
  configureLedPins();
  beginDebugUart();

  logLine("BOOT LedBrightnessTest");
  logLine("PWM=SOFTWARE TIM21 1000Hz DIM=25% BRIGHT=100%");

  if (!beginSoftwarePwm()) {
    logLine("ERR: PWM timer init failed");
    setAllBlueLeds(LedLevel::OFF);
    while (true) {
      keepSt25LowPowerDown();
    }
  }

  runIndividualLedTest();
  runAllLedTest();
  runMixedBrightnessTest();
  runProgressDemo();
  logLine("TEST DONE: ALL OFF, PA4 HIGH, ENTER STOP");
}

void loop()
{
  enterStopAfterTest();

  // Unexpected wake: restore enough state to turn LEDs off and re-enter STOP.
  SystemClock_Config();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  configureLedPins();
  keepSt25LowPowerDown();
}
