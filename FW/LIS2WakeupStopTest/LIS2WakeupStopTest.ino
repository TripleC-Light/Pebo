// Pebo v0.1 LIS2DW12 INT1 wake from STM32L031 STOP mode test.
// Target: STM32L031G6U6TR using STM32duino.
//
// This is a bring-up firmware only. It verifies:
// STM32 STOP -> LIS2DW12 motion wake-up -> INT1 -> PB0/EXTI0 wake ->
// UART debug -> clear LIS2 source -> STOP again.
//
// Pin source: docs/hardware_pinmap.md.

#include <Arduino.h>
#include <Wire.h>

#include "../libraries/PeboLIS2DW12/src/PeboLIS2DW12.h"
#include "../libraries/PeboLIS2DW12/src/PeboLIS2DW12.cpp"

#define PEBO_WAKE_PULSE_DEBUG 1

extern "C" void SystemClock_Config(void);

static constexpr uint32_t PIN_ACC_INT1 = PB0;
static constexpr uint32_t PIN_ACC_INT2 = PB1;
static constexpr uint32_t PIN_I2C_SCL = PB6;
static constexpr uint32_t PIN_I2C_SDA = PB7;
static constexpr uint32_t PIN_UART_TX = PA9;
static constexpr uint32_t PIN_UART_RX = PA10;
static constexpr uint32_t PIN_NFC_LPD = PA4;
static constexpr uint32_t PIN_WAKE_PULSE = PA2;

static constexpr uint32_t PIN_PIEZO_A = PA0;
static constexpr uint32_t PIN_PIEZO_B = PA1;
static constexpr uint32_t PIN_NFC_GPO = PA3;
static constexpr uint32_t PIN_LED_BLUE_1 = PA5;
static constexpr uint32_t PIN_LED_BLUE_2 = PA6;
static constexpr uint32_t PIN_LED_BLUE_3 = PA7;
static constexpr uint32_t PIN_LED_BLUE_4 = PA8;
static constexpr uint32_t PIN_LED_RED = PA15;

static constexpr uint32_t UART_BAUD = 115200;
static constexpr uint32_t I2C_CLOCK_HZ = 100000;

static constexpr uint8_t REG_CTRL1 = 0x20;
static constexpr uint8_t REG_CTRL2 = 0x21;
static constexpr uint8_t REG_CTRL3 = 0x22;
static constexpr uint8_t REG_CTRL4_INT1_PAD_CTRL = 0x23;
static constexpr uint8_t REG_CTRL5_INT2_PAD_CTRL = 0x24;
static constexpr uint8_t REG_CTRL6 = 0x25;
static constexpr uint8_t REG_STATUS = 0x27;
static constexpr uint8_t REG_OUT_X_L = 0x28;
static constexpr uint8_t REG_WAKE_UP_THS = 0x34;
static constexpr uint8_t REG_WAKE_UP_DUR = 0x35;
static constexpr uint8_t REG_WAKE_UP_SRC = 0x38;
static constexpr uint8_t REG_ALL_INT_SRC = 0x3B;
static constexpr uint8_t REG_CTRL7 = 0x3F;

static Uart DebugUart(PIN_UART_RX, PIN_UART_TX);
static PeboLIS2DW12 Lis2(Wire);
static volatile bool extiInt1Wake = false;
static uint32_t wakeCount = 0;

static void lis2Int1Isr()
{
  extiInt1Wake = true;
}

static void printHex8(uint8_t value)
{
  if (value < 0x10) {
    DebugUart.print('0');
  }
  DebugUart.print(value, HEX);
}

static void printReg(const char *name, uint8_t value)
{
  DebugUart.print(name);
  DebugUart.print("=0x");
  printHex8(value);
  DebugUart.print(' ');
}

static void setupAlwaysDefinedPins()
{
  pinMode(PIN_NFC_LPD, OUTPUT);
  digitalWrite(PIN_NFC_LPD, HIGH);

#if PEBO_WAKE_PULSE_DEBUG
  pinMode(PIN_WAKE_PULSE, OUTPUT);
  digitalWrite(PIN_WAKE_PULSE, LOW);
#endif

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

static void beginI2c()
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  Wire.setSCL(PIN_I2C_SCL);
  Wire.setSDA(PIN_I2C_SDA);
  Wire.begin();
  Wire.setClock(I2C_CLOCK_HZ);
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

static bool lis2WriteChecked(uint8_t reg, uint8_t value)
{
  const bool ok = Lis2.writeRegister(reg, value);
  DebugUart.print(ok ? "LIS2_WR " : "LIS2_WR_FAIL ");
  printReg("REG", reg);
  printReg("VAL", value);
  DebugUart.println();
  return ok;
}

static bool configureLis2Wakeup()
{
  uint8_t whoAmI = 0;
  if (!Lis2.begin(PeboLIS2DW12::ADDRESS_HIGH) || !Lis2.readWhoAmI(whoAmI)) {
    DebugUart.println("ERR LIS2 not detected at 0x19");
    return false;
  }
  if (whoAmI != PeboLIS2DW12::WHO_AM_I_EXPECTED) {
    DebugUart.print("ERR LIS2 WHO_AM_I=0x");
    printHex8(whoAmI);
    DebugUart.println(" expected=0x44");
    return false;
  }

  DebugUart.println("OK LIS2 WHO_AM_I=0x44");

  // CTRL2 (0x21): SOFT_RESET=1. Reset LIS2DW12 before programming wake-up.
  if (!lis2WriteChecked(REG_CTRL2, 0x40)) {
    return false;
  }
  delay(5);

  // CTRL2 (0x21): BDU=1 keeps output registers coherent; IF_ADD_INC=1 enables
  // multi-byte auto-increment reads for OUT_X/Y/Z. I2C remains enabled.
  if (!lis2WriteChecked(REG_CTRL2, 0x0C)) {
    return false;
  }

  // CTRL3 (0x22): LIR=1 latches interrupt request until source is read.
  // H_LACTIVE=0 leaves INT pins active-high. PP_OD=0 keeps push-pull output.
  // This matches STM32 PB0 EXTI rising-edge wake.
  if (!lis2WriteChecked(REG_CTRL3, 0x10)) {
    return false;
  }

  // CTRL4_INT1_PAD_CTRL (0x23): INT1_WU=1 routes wake-up interrupt to INT1.
  // Other INT1 sources are disabled for this motion wake-up bring-up test.
  if (!lis2WriteChecked(REG_CTRL4_INT1_PAD_CTRL, 0x20)) {
    return false;
  }

  // CTRL5_INT2_PAD_CTRL (0x24): 0x00 leaves INT2 unused in this firmware.
  if (!lis2WriteChecked(REG_CTRL5_INT2_PAD_CTRL, 0x00)) {
    return false;
  }

  // CTRL6 (0x25): FS=00 selects +/-2 g; LOW_NOISE=0 to minimize current;
  // FDS=0 uses low-pass path. Bandwidth defaults are kept for bring-up.
  if (!lis2WriteChecked(REG_CTRL6, 0x00)) {
    return false;
  }

  // WAKE_UP_THS (0x34): WK_THS[5:0]=0x02 gives an intentionally low threshold
  // for easy triggering during bring-up. SLEEP_ON=0 and tap mode bits remain 0.
  if (!lis2WriteChecked(REG_WAKE_UP_THS, 0x02)) {
    return false;
  }

  // WAKE_UP_DUR (0x35): WAKE_DUR[1:0]=0 means the shortest wake duration.
  // SLEEP_DUR=0 and stationary/free-fall duration bits are not used here.
  if (!lis2WriteChecked(REG_WAKE_UP_DUR, 0x00)) {
    return false;
  }

  // CTRL7 (0x3F): INTERRUPTS_ENABLE=1 enables embedded interrupt generation.
  // INT2_ON_INT1=0 so only explicitly routed INT1_WU appears on INT1.
  if (!lis2WriteChecked(REG_CTRL7, 0x20)) {
    return false;
  }

  // CTRL1 (0x20): ODR=0010 selects 12.5 Hz; MODE=00 and LP_MODE=00 select
  // continuous low-power 12-bit mode per ST's LIS2DW12 driver definitions.
  if (!lis2WriteChecked(REG_CTRL1, 0x20)) {
    return false;
  }

  uint8_t ignored = 0;
  Lis2.readRegister(REG_WAKE_UP_SRC, ignored);
  Lis2.readRegister(REG_ALL_INT_SRC, ignored);
  return true;
}

static void configureExtiWakeInput()
{
  __HAL_RCC_GPIOB_CLK_ENABLE();

  pinMode(PIN_ACC_INT1, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ACC_INT1), lis2Int1Isr, RISING);
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_ClearPendingIRQ(EXTI0_1_IRQn);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
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

  // Analog + No Pull before STOP:
  // PA0/PA1 piezo, PA3 ST25_GPO, LEDs PA5/PA6/PA7/PA8/PA15, UART PA9/PA10.
  // PA4 ST25_LPD is excluded and remains output HIGH.
  // PA13/PA14 are excluded to preserve SWD.
  // PA2 is excluded when wake-pulse debug is enabled.
#if PEBO_WAKE_PULSE_DEBUG
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 |
             GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 |
             GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
             GPIO_PIN_15;
#else
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
             GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 |
             GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
             GPIO_PIN_15;
#endif
  HAL_GPIO_Init(GPIOA, &gpio);

  // PB0 is EXTI wake input. PB6/PB7 are I2C lines and are kept as inputs with
  // external pull-ups. PB1 INT2 and PB3 test/reserved are analog no-pull.
  gpio.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |
             GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
             GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOB, &gpio);

  // PC14/PC15 LSE pins are not used in this test because RTC is disabled.
  gpio.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOC, &gpio);
}

static void disableUnusedPeripheralClocksForStop()
{
  // Clocks disabled before STOP:
  // UART/USART2 after debug flush, I2C1 after LIS2 transaction, ADC, SPI,
  // TIM/LPTIM, RTC, DMA, CRC, DAC, and debug block sleep clock.
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

static void disablePeripheralIrqExceptExti0()
{
  for (uint8_t irq = 0; irq < 32; ++irq) {
    const IRQn_Type irqType = static_cast<IRQn_Type>(irq);
    if (irqType == EXTI0_1_IRQn) {
      continue;
    }
    NVIC_DisableIRQ(irqType);
    NVIC_ClearPendingIRQ(irqType);
  }
  NVIC_ClearPendingIRQ(EXTI0_1_IRQn);
}

void prepareForStopMode()
{
  setupAlwaysDefinedPins();
  configureExtiWakeInput();
  endI2cForStop();
  endDebugUartForStop();
  configureUnusedGpioForStop();
  disablePeripheralIrqExceptExti0();

#if defined(DBGMCU)
  DBGMCU->CR = 0;
#endif

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

#if defined(EXTI)
  EXTI->PR = GPIO_PIN_0;
#endif
  extiInt1Wake = false;

  // Stop SysTick/millis before STOP so no periodic Arduino tick can wake MCU.
  SysTick->CTRL = 0;
  HAL_SuspendTick();

  disableUnusedPeripheralClocksForStop();

  // Keep GPIO state latched; PB0 EXTI and PA4 output state remain configured.
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
}

void enterStopMode()
{
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

void restoreAfterWakeup()
{
  SystemClock_Config();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

#if PEBO_WAKE_PULSE_DEBUG
  pinMode(PIN_WAKE_PULSE, OUTPUT);
  digitalWrite(PIN_WAKE_PULSE, HIGH);
  delayMicroseconds(80);
  digitalWrite(PIN_WAKE_PULSE, LOW);
#endif
}

static void printWakeDebug()
{
  ++wakeCount;

  const bool int1Line = digitalRead(PIN_ACC_INT1) == HIGH;
  if (extiInt1Wake || int1Line) {
    DebugUart.println("WAKE: LIS2DW12 INT1");
  } else {
    DebugUart.println("WAKE: UNKNOWN");
  }

  uint8_t status = 0;
  uint8_t wakeSrc = 0;
  uint8_t allIntSrc = 0;
  PeboLis2dw12RawSample sample = {};

  Lis2.readRegister(REG_STATUS, status);
  // Reading WAKE_UP_SRC clears the latched LIS2DW12 wake-up interrupt source
  // when CTRL3.LIR=1.
  Lis2.readRegister(REG_WAKE_UP_SRC, wakeSrc);
  Lis2.readRegister(REG_ALL_INT_SRC, allIntSrc);
  Lis2.readRaw(sample);

  DebugUart.print("COUNT=");
  DebugUart.print(wakeCount);
  DebugUart.print(' ');
  printReg("STATUS", status);
  printReg("WAKE_SRC", wakeSrc);
  printReg("ALL_INT_SRC", allIntSrc);
  DebugUart.print("INT1=");
  DebugUart.print(digitalRead(PIN_ACC_INT1));
  DebugUart.print(" X=");
  DebugUart.print(sample.x);
  DebugUart.print(" Y=");
  DebugUart.print(sample.y);
  DebugUart.print(" Z=");
  DebugUart.println(sample.z);

  DebugUart.flush();
}

void setup()
{
  setupAlwaysDefinedPins();
  beginDebugUart();
  beginI2c();

  DebugUart.println("BOOT LIS2WakeupStopTest");
  if (!configureLis2Wakeup()) {
    DebugUart.println("HALT LIS2 config failed");
    while (true) {
      delay(1000);
    }
  }

  configureExtiWakeInput();
  DebugUart.println("ENTER STOP: waiting for LIS2DW12 INT1");
  DebugUart.flush();
}

void loop()
{
  prepareForStopMode();
  enterStopMode();
  restoreAfterWakeup();

  beginDebugUart();
  beginI2c();
  configureExtiWakeInput();
  printWakeDebug();

  DebugUart.println("ENTER STOP: waiting for LIS2DW12 INT1");
}
