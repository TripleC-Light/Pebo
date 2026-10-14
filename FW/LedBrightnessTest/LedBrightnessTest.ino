// Pebo v0.1 standalone LED brightness test, STM32duino 3.0.0.
#include <Arduino.h>
#include <HardwareTimer.h>

#ifndef PEBO_LED_DEBUG
#define PEBO_LED_DEBUG 1
#endif
#ifndef PEBO_LED_DIM_DUTY_PERCENT
#define PEBO_LED_DIM_DUTY_PERCENT 25
#endif
#ifndef PEBO_LED_PWM_HZ
#define PEBO_LED_PWM_HZ 500
#endif

#if !defined(STM32L031xx)
#error "Select STM32L031G6Ux"
#endif
static_assert(PEBO_LED_DIM_DUTY_PERCENT > 0 &&
              PEBO_LED_DIM_DUTY_PERCENT < 100 &&
              PEBO_LED_DIM_DUTY_PERCENT % 5 == 0,
              "DIM must be 5..95 percent in 5-percent steps");
static_assert(PEBO_LED_PWM_HZ >= 300 && PEBO_LED_PWM_HZ <= 1000,
              "PWM frequency must be 300..1000 Hz");

enum class LedLevel : uint8_t { OFF, DIM, BRIGHT };

// Explicit declarations avoid Arduino auto-prototype ordering with LedLevel.
void setBlueLed(uint8_t index, LedLevel level);
void setAllBlueLeds(LedLevel level);
static uint8_t dutySlots(LedLevel level);

static constexpr uint32_t LED_PINS[] = {PA5, PA6, PA7, PA8};
static constexpr uint32_t LED_BITS[] = {
  GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_8
};
static constexpr uint32_t LED_MASK =
    GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
static constexpr uint8_t PWM_SLOTS = 20;
static volatile uint8_t duties[4] = {};
static uint8_t phase = 0;
static HardwareTimer pwmTimer(TIM21);

#if PEBO_LED_DEBUG
static HardwareSerial debugUart(PA10, PA9);
#define LOG_STATE(text) debugUart.println(text)
#else
#define LOG_STATE(text) do {} while (0)
#endif

static uint8_t dutySlots(LedLevel level)
{
  switch (level) {
    case LedLevel::DIM: return PEBO_LED_DIM_DUTY_PERCENT / 5;
    case LedLevel::BRIGHT: return PWM_SLOTS;
    default: return 0;
  }
}

static void pwmTick()
{
  uint32_t high = 0;
  for (uint8_t i = 0; i < 4; ++i) {
    if (phase < duties[i]) high |= LED_BITS[i];
  }
  // One atomic write updates only the four LEDs, never PA4/ST25_LPD.
  GPIOA->BSRR = high | ((LED_MASK & ~high) << 16);
  if (++phase == PWM_SLOTS) phase = 0;
}

void setBlueLed(uint8_t index, LedLevel level)
{
  if (index >= 4) return;
  const uint8_t duty = dutySlots(level);
  const uint32_t irqState = __get_PRIMASK();
  __disable_irq();
  duties[index] = duty;
  if (duty == 0) GPIOA->BSRR = LED_BITS[index] << 16;
  if (duty == PWM_SLOTS) GPIOA->BSRR = LED_BITS[index];
  __set_PRIMASK(irqState);
}

void setAllBlueLeds(LedLevel level)
{
  const uint8_t duty = dutySlots(level);
  const uint32_t irqState = __get_PRIMASK();
  __disable_irq();
  for (uint8_t i = 0; i < 4; ++i) duties[i] = duty;
  if (duty == 0) GPIOA->BSRR = LED_MASK << 16;
  if (duty == PWM_SLOTS) GPIOA->BSRR = LED_MASK;
  __set_PRIMASK(irqState);
}

static void holdState(uint32_t durationMs)
{
  const uint32_t started = millis();
  // Only test sequencing uses millis; TIM21 independently generates PWM.
  while (static_cast<uint32_t>(millis() - started) < durationMs) {
    yield();
  }
}

static void logIndividual(uint8_t index, bool bright)
{
#if PEBO_LED_DEBUG
  debugUart.print("LED");
  debugUart.print(index + 1);
  debugUart.println(bright ? " BRIGHT" : " DIM");
#else
  (void)index;
  (void)bright;
#endif
}

static void runTests()
{
  for (uint8_t i = 0; i < 4; ++i) {
    setAllBlueLeds(LedLevel::OFF);
    setBlueLed(i, LedLevel::DIM);
    logIndividual(i, false);
    holdState(2000);
    setBlueLed(i, LedLevel::BRIGHT);
    logIndividual(i, true);
    holdState(2000);
    setBlueLed(i, LedLevel::OFF);
  }
  setAllBlueLeds(LedLevel::DIM);
  LOG_STATE("ALL DIM");
  holdState(3000);
  setAllBlueLeds(LedLevel::BRIGHT);
  LOG_STATE("ALL BRIGHT");
  holdState(3000);
  setAllBlueLeds(LedLevel::OFF);
  LOG_STATE("ALL OFF");

  setBlueLed(0, LedLevel::DIM);
  setBlueLed(1, LedLevel::DIM);
  setBlueLed(2, LedLevel::BRIGHT);
  setBlueLed(3, LedLevel::OFF);
  LOG_STATE("MIXED DIM DIM BRIGHT OFF");
  holdState(3000);

  for (uint8_t step = 0; step <= 5; ++step) {
    setAllBlueLeds(LedLevel::OFF);
    if (step == 5) {
      setAllBlueLeds(LedLevel::BRIGHT);
    } else if (step > 0) {
      for (uint8_t i = 0; i + 1 < step; ++i) {
        setBlueLed(i, LedLevel::DIM);
      }
      setBlueLed(step - 1, LedLevel::BRIGHT);
    }
#if PEBO_LED_DEBUG
    debugUart.print("PROGRESS STEP ");
    debugUart.println(step);
#endif
    holdState(1000);
  }
  setAllBlueLeds(LedLevel::OFF);
  LOG_STATE("ALL OFF");
}

static void prepareForStop()
{
  pwmTimer.pause();
  pwmTimer.detachInterrupt();
  TIM21->DIER = 0;
  TIM21->SR = 0;
  NVIC_DisableIRQ(TIM21_IRQn);
  NVIC_ClearPendingIRQ(TIM21_IRQn);
  __HAL_RCC_TIM21_CLK_DISABLE();
  setAllBlueLeds(LedLevel::OFF);
#if PEBO_LED_DEBUG
  LOG_STATE("STOP PA4 HIGH");
  debugUart.flush();
  debugUart.end();
#endif

  // Leave LEDs actively LOW and PA4 actively HIGH throughout STOP.
  pinMode(PA4, OUTPUT);
  digitalWrite(PA4, HIGH);
  GPIO_InitTypeDef gpio = {};
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
             GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
             GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &gpio);
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  gpio.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOB, &gpio);
  HAL_GPIO_Init(GPIOC, &gpio);

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  DBGMCU->CR = 0;
  // This standalone test intentionally has no wake source.
  NVIC->ICER[0] = 0xFFFFFFFFUL;
  NVIC->ICPR[0] = 0xFFFFFFFFUL;
  EXTI->PR = 0xFFFFFFFFUL;
  HAL_SuspendTick();
  SysTick->CTRL = 0;
  SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;
  __DSB();
  __ISB();
}

void setup()
{
  pinMode(PA4, OUTPUT);
  digitalWrite(PA4, HIGH);
  for (uint8_t i = 0; i < 4; ++i) {
    digitalWrite(LED_PINS[i], LOW);
    pinMode(LED_PINS[i], OUTPUT);
  }
  digitalWrite(PA15, LOW);
  pinMode(PA15, OUTPUT);
#if PEBO_LED_DEBUG
  debugUart.begin(115200);
  LOG_STATE("LED TEST START");
#endif
  pwmTimer.setOverflow(PEBO_LED_PWM_HZ * PWM_SLOTS, HERTZ_FORMAT);
  pwmTimer.attachInterrupt(pwmTick);
  pwmTimer.setInterruptPriority(0, 0);
  pwmTimer.resume();
  runTests();
  prepareForStop();
}

void loop()
{
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}
