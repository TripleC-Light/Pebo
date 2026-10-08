// Pebo v0.1 ST25DV04KC GPO wake from STM32L031 STOP mode test.
// Target: STM32L031G6U6TR using STM32duino.
//
// Pin source: docs/hardware_pinmap.md.
// This sketch verifies only: ST25DV04KC GPO -> PA3/EXTI -> STOP wake.
// It does not implement RTC, LIS2DW12, NFC payloads, time sync, app logic, or
// reminder behavior.

#include <Arduino.h>
#include <Wire.h>

#include "../libraries/PeboST25DV/src/PeboST25DV.h"
#include "../libraries/PeboST25DV/src/PeboST25DV.cpp"

#define PEBO_ST25_DEBUG 1
#define PEBO_ST25_WAKE_PULSE_DEBUG 1

extern "C" void SystemClock_Config(void);

static constexpr uint32_t PIN_NFC_GPO = PA3;
static constexpr uint32_t PIN_NFC_LPD = PA4;
static constexpr uint32_t PIN_I2C_SCL = PB6;
static constexpr uint32_t PIN_I2C_SDA = PB7;
static constexpr uint32_t PIN_UART_TX = PA9;
static constexpr uint32_t PIN_UART_RX = PA10;
static constexpr uint32_t PIN_WAKE_PULSE = PA2;

static constexpr uint32_t PIN_PIEZO_A = PA0;
static constexpr uint32_t PIN_PIEZO_B = PA1;
static constexpr uint32_t PIN_LED_BLUE_1 = PA5;
static constexpr uint32_t PIN_LED_BLUE_2 = PA6;
static constexpr uint32_t PIN_LED_BLUE_3 = PA7;
static constexpr uint32_t PIN_LED_BLUE_4 = PA8;
static constexpr uint32_t PIN_LED_RED = PA15;
static constexpr uint32_t PIN_ACC_INT1 = PB0;
static constexpr uint32_t PIN_ACC_INT2 = PB1;
static constexpr uint32_t PIN_TEST_PB3 = PB3;

static constexpr uint32_t UART_BAUD = 115200;
static constexpr uint32_t I2C_CLOCK_HZ = 100000;

static constexpr uint8_t ST25_USER_I2C_ADDR = PeboST25DV::USER_I2C_ADDRESS;
static constexpr uint8_t ST25_SYSTEM_I2C_ADDR = PeboST25DV::SYSTEM_I2C_ADDRESS;

static constexpr uint16_t ST25_REG_GPO1 = 0x0000;
static constexpr uint16_t ST25_REG_MEM_SIZE_L = 0x0014;
static constexpr uint16_t ST25_REG_UID = 0x0018;
static constexpr uint16_t ST25_DYN_GPO_CTRL = 0x2000;
static constexpr uint16_t ST25_DYN_EH_CTRL = 0x2002;
static constexpr uint16_t ST25_DYN_RF_MNGT = 0x2003;
static constexpr uint16_t ST25_DYN_IT_STS = 0x2005;

static constexpr uint8_t ST25_GPO1_GPO_EN = 0x01;
static constexpr uint8_t ST25_GPO1_FIELD_CHANGE_EN = 0x10;
static constexpr uint8_t ST25_GPO_CTRL_DYN_GPO_EN = 0x01;

static PeboST25DV St25(Wire);
static volatile bool extiGpoWake = false;
static bool gpoActiveHigh = true;
static uint32_t wakeCount = 0;

#if PEBO_ST25_DEBUG
static Uart DebugUart(PIN_UART_RX, PIN_UART_TX);
#endif

static void st25GpoIsr()
{
  extiGpoWake = true;
}

static void keepSt25LowPowerDown()
{
  pinMode(PIN_NFC_LPD, OUTPUT);
  digitalWrite(PIN_NFC_LPD, HIGH);
}

static void wakeSt25ForI2c()
{
  // PA4/ST25_LPD high is mandatory during STOP. During active debug windows,
  // drive LPD low briefly so VCC-powered I2C registers can be accessed.
  pinMode(PIN_NFC_LPD, OUTPUT);
  digitalWrite(PIN_NFC_LPD, LOW);
  delayMicroseconds(1000);
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

#if PEBO_ST25_WAKE_PULSE_DEBUG
  pinMode(PIN_WAKE_PULSE, OUTPUT);
  digitalWrite(PIN_WAKE_PULSE, LOW);
#endif
}

#if PEBO_ST25_DEBUG
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

static void printHex8(uint8_t value)
{
  if (value < 0x10) {
    DebugUart.print('0');
  }
  DebugUart.print(value, HEX);
}

static void printByte(const char *name, uint8_t value)
{
  DebugUart.print(name);
  DebugUart.print("=0x");
  printHex8(value);
  DebugUart.print(' ');
}
#else
static void beginDebugUart() {}
static void endDebugUartForStop() {}
#endif

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

static bool i2cAddressAck(uint8_t address)
{
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

static bool st25ReadMem(uint8_t i2cAddress, uint16_t memAddress, uint8_t *data, uint8_t length)
{
  if ((data == nullptr) || (length == 0)) {
    return false;
  }

  Wire.beginTransmission(i2cAddress);
  Wire.write(static_cast<uint8_t>(memAddress >> 8));
  Wire.write(static_cast<uint8_t>(memAddress & 0xFF));
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  const uint8_t received = Wire.requestFrom(i2cAddress, length);
  if (received != length) {
    while (Wire.available() > 0) {
      Wire.read();
    }
    return false;
  }

  for (uint8_t i = 0; i < length; ++i) {
    data[i] = static_cast<uint8_t>(Wire.read());
  }
  return true;
}

static bool st25WriteMem(uint8_t i2cAddress, uint16_t memAddress, uint8_t value)
{
  Wire.beginTransmission(i2cAddress);
  Wire.write(static_cast<uint8_t>(memAddress >> 8));
  Wire.write(static_cast<uint8_t>(memAddress & 0xFF));
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

static bool st25ReadUserDyn(uint16_t reg, uint8_t &value)
{
  // ST25DVxxKC dynamic registers such as GPO_CTRL_Dyn (0x2000),
  // RF_MNGT_Dyn (0x2003), and IT_STS_Dyn (0x2005) are addressed through the
  // user I2C target address on this board (0x53).
  return st25ReadMem(ST25_USER_I2C_ADDR, reg, &value, 1);
}

static bool st25WriteUserDyn(uint16_t reg, uint8_t value)
{
  return st25WriteMem(ST25_USER_I2C_ADDR, reg, value);
}

static bool st25ReadSystem(uint16_t reg, uint8_t &value)
{
  // ST25DVxxKC system configuration registers such as GPO1 (0x0000), MEM_SIZE,
  // and UID are addressed through the system I2C target address (0x57).
  return st25ReadMem(ST25_SYSTEM_I2C_ADDR, reg, &value, 1);
}

static bool st25WriteSystem(uint16_t reg, uint8_t value)
{
  return st25WriteMem(ST25_SYSTEM_I2C_ADDR, reg, value);
}

static bool configureSt25Gpo()
{
  wakeSt25ForI2c();
  beginI2c();

  const bool userAck = St25.userInterfacePresent();
  const bool systemAck = St25.systemInterfacePresent();

#if PEBO_ST25_DEBUG
  DebugUart.print("ST25 user_ack=");
  DebugUart.print(userAck ? "YES" : "NO");
  DebugUart.print(" system_ack=");
  DebugUart.println(systemAck ? "YES" : "NO");
#endif

  if (!userAck || !systemAck) {
    return false;
  }

  uint8_t gpo1 = 0;
  uint8_t memSizeL = 0;
  uint8_t uid[8] = {0};
  const bool gpo1Ok = st25ReadSystem(ST25_REG_GPO1, gpo1);
  const bool memOk = st25ReadSystem(ST25_REG_MEM_SIZE_L, memSizeL);
  const bool uidOk = st25ReadMem(ST25_SYSTEM_I2C_ADDR, ST25_REG_UID, uid, sizeof(uid));

#if PEBO_ST25_DEBUG
  DebugUart.print("ST25 ");
  if (gpo1Ok) {
    printByte("GPO1", gpo1);
  } else {
    DebugUart.print("GPO1=NA ");
  }
  if (memOk) {
    printByte("MEM_SIZE_L", memSizeL);
  } else {
    DebugUart.print("MEM_SIZE_L=NA ");
  }
  DebugUart.print("UID=");
  if (uidOk) {
    for (uint8_t i = 0; i < sizeof(uid); ++i) {
      printHex8(uid[i]);
    }
  } else {
    DebugUart.print("NA");
  }
  DebugUart.println();
#endif

  // GPO1 (system config 0x0000): GPO_EN bit 0 enables output; FIELD_CHANGE_EN
  // bit 4 enables a pulse when RF field appears or disappears. Factory default
  // is expected to include both bits, but if field-change is disabled we try to
  // write it. This write may fail unless the I2C security session is open.
  if (gpo1Ok && ((gpo1 & (ST25_GPO1_GPO_EN | ST25_GPO1_FIELD_CHANGE_EN)) !=
                 (ST25_GPO1_GPO_EN | ST25_GPO1_FIELD_CHANGE_EN))) {
    const uint8_t requested = gpo1 | ST25_GPO1_GPO_EN | ST25_GPO1_FIELD_CHANGE_EN;
    const bool writeOk = st25WriteSystem(ST25_REG_GPO1, requested);
#if PEBO_ST25_DEBUG
    DebugUart.print("ST25 GPO1_WRITE requested=0x");
    printHex8(requested);
    DebugUart.print(" ok=");
    DebugUart.println(writeOk ? "YES" : "NO_SECURITY_SESSION_MAY_BE_REQUIRED");
#endif
  }

  // GPO_CTRL_Dyn (dynamic user address 0x2000): GPO_EN bit 0 dynamically
  // enables GPO output. This is writable without opening the security session
  // and takes precedence over static GPO1.GPO_EN while powered.
  if (!st25WriteUserDyn(ST25_DYN_GPO_CTRL, ST25_GPO_CTRL_DYN_GPO_EN)) {
    return false;
  }

  uint8_t gpoCtrl = 0;
  uint8_t rfMngt = 0;
  uint8_t ehCtrl = 0;
  uint8_t itStatus = 0;
  st25ReadUserDyn(ST25_DYN_GPO_CTRL, gpoCtrl);
  st25ReadUserDyn(ST25_DYN_RF_MNGT, rfMngt);
  st25ReadUserDyn(ST25_DYN_EH_CTRL, ehCtrl);
  // IT_STS_Dyn (0x2005): reading clears accumulated GPO interrupt causes.
  st25ReadUserDyn(ST25_DYN_IT_STS, itStatus);

#if PEBO_ST25_DEBUG
  DebugUart.print("ST25 ");
  printByte("GPO_CTRL_DYN", gpoCtrl);
  printByte("RF_MNGT_DYN", rfMngt);
  printByte("EH_CTRL_DYN", ehCtrl);
  printByte("IT_STATUS_CLEAR", itStatus);
  DebugUart.println();
#endif

  const int idle = digitalRead(PIN_NFC_GPO);
  // ST25DVxxKC CMOS GPO set state is documented as High. Measure idle and use
  // the opposite edge for wake so the firmware does not assume board polarity.
  gpoActiveHigh = (idle == LOW);
  const PinStatus mode = gpoActiveHigh ? RISING : FALLING;
  attachInterrupt(digitalPinToInterrupt(PIN_NFC_GPO), st25GpoIsr, mode);

#if PEBO_ST25_DEBUG
  DebugUart.print("GPO idle=");
  DebugUart.print(idle);
  DebugUart.print(" active=");
  DebugUart.print(gpoActiveHigh ? "HIGH" : "LOW");
  DebugUart.print(" exti_edge=");
  DebugUart.println(gpoActiveHigh ? "RISING" : "FALLING");
#endif

  return true;
}

static void configureGpoExti()
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  pinMode(PIN_NFC_GPO, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_NFC_GPO),
                  st25GpoIsr,
                  gpoActiveHigh ? RISING : FALLING);
  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
  HAL_NVIC_ClearPendingIRQ(EXTI2_3_IRQn);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
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

  // PA3/ST25_GPO is excluded and remains EXTI input.
  // PA4/ST25_LPD is excluded and remains GPIO Output High.
  // PA13/PA14 are excluded for SWD.
#if PEBO_ST25_WAKE_PULSE_DEBUG
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 |
             GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 |
             GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
             GPIO_PIN_15;
#else
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
             GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 |
             GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
             GPIO_PIN_15;
#endif
  HAL_GPIO_Init(GPIOA, &gpio);

  // PB6/PB7 I2C are left as inputs by endI2cForStop() so the external pullups
  // keep the bus in a safe idle state. PB0/PB1/PB3 and other PB pins analog.
  gpio.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
             GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_9 |
             GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
             GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin = GPIO_PIN_All;
  HAL_GPIO_Init(GPIOC, &gpio);
}

static void disablePeripheralIrqExceptGpoExti()
{
  for (uint8_t irq = 0; irq < 32; ++irq) {
    const IRQn_Type irqType = static_cast<IRQn_Type>(irq);
    if (irqType == EXTI2_3_IRQn) {
      continue;
    }
    NVIC_DisableIRQ(irqType);
    NVIC_ClearPendingIRQ(irqType);
  }
  HAL_NVIC_ClearPendingIRQ(EXTI2_3_IRQn);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
}

static void disableUnusedPeripheralClocksForStop()
{
  // Preserve GPIO state for PA3 EXTI and PA4 Output High. Disable debug and
  // unused peripheral clocks before STOP. I2C and UART are disabled separately.
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

static void prepareForStopMode()
{
  keepSt25LowPowerDown();
  configureGpoExti();
  endI2cForStop();
  endDebugUartForStop();
  configureUnusedGpioForStop();
  disablePeripheralIrqExceptGpoExti();

#if defined(EXTI)
  EXTI->PR = GPIO_PIN_3;
#endif
  extiGpoWake = false;

#if defined(DBGMCU)
  DBGMCU->CR = 0;
#endif

  // Stop SysTick/millis completely so no Arduino periodic tick wakes STOP.
  SysTick->CTRL = 0;
  HAL_SuspendTick();

  disableUnusedPeripheralClocksForStop();

  // Keep PA3/PA4 GPIO state latched while in STOP.
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
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

#if PEBO_ST25_WAKE_PULSE_DEBUG
  pinMode(PIN_WAKE_PULSE, OUTPUT);
  digitalWrite(PIN_WAKE_PULSE, HIGH);
  delayMicroseconds(80);
  digitalWrite(PIN_WAKE_PULSE, LOW);
#endif
}

static void printWakeDebug()
{
  ++wakeCount;

  const int gpoLevel = digitalRead(PIN_NFC_GPO);
  uint8_t itStatus = 0;
  uint8_t rfStatus = 0;
  uint8_t ehStatus = 0;
  uint8_t gpoCtrl = 0;
  const bool itOk = st25ReadUserDyn(ST25_DYN_IT_STS, itStatus);
  const bool rfOk = st25ReadUserDyn(ST25_DYN_RF_MNGT, rfStatus);
  const bool ehOk = st25ReadUserDyn(ST25_DYN_EH_CTRL, ehStatus);
  const bool gpoOk = st25ReadUserDyn(ST25_DYN_GPO_CTRL, gpoCtrl);

#if PEBO_ST25_DEBUG
  DebugUart.println((extiGpoWake || (gpoLevel == (gpoActiveHigh ? HIGH : LOW))) ?
                    "WAKE: ST25 GPO" : "WAKE: UNKNOWN");
  DebugUart.print("COUNT=");
  DebugUart.print(wakeCount);
  DebugUart.print(" GPO=");
  DebugUart.print(gpoLevel);
  DebugUart.print(' ');
  if (itOk) {
    printByte("IT_STATUS", itStatus);
  } else {
    DebugUart.print("IT_STATUS=NA ");
  }
  if (rfOk) {
    printByte("RF_STATUS", rfStatus);
  } else {
    DebugUart.print("RF_STATUS=NA ");
  }
  if (ehOk) {
    printByte("EH_STATUS", ehStatus);
  } else {
    DebugUart.print("EH_STATUS=NA ");
  }
  if (gpoOk) {
    printByte("GPO_CTRL_DYN", gpoCtrl);
  } else {
    DebugUart.print("GPO_CTRL_DYN=NA ");
  }
  DebugUart.print("GPO_AFTER_CLEAR=");
  DebugUart.println(digitalRead(PIN_NFC_GPO));
#endif

  // IT_STS_Dyn read above clears the accumulated ST25 GPO interrupt cause.
  HAL_NVIC_ClearPendingIRQ(EXTI2_3_IRQn);
#if defined(EXTI)
  EXTI->PR = GPIO_PIN_3;
#endif
  extiGpoWake = false;
}

void setup()
{
  setKnownOutputsOff();
  wakeSt25ForI2c();
  beginDebugUart();
  beginI2c();

#if PEBO_ST25_DEBUG
  DebugUart.println("BOOT ST25GpoWakeTest");
#endif

  if (!configureSt25Gpo()) {
#if PEBO_ST25_DEBUG
    DebugUart.println("ERR: ST25 init/config failed");
    DebugUart.flush();
#endif
    keepSt25LowPowerDown();
    while (true) {
    }
  }

#if PEBO_ST25_DEBUG
  DebugUart.println("ENTER STOP");
#endif
}

void loop()
{
  prepareForStopMode();
  enterStopMode();
  restoreAfterWakeup();

  wakeSt25ForI2c();
  beginDebugUart();
  beginI2c();
  configureGpoExti();
  printWakeDebug();

#if PEBO_ST25_DEBUG
  DebugUart.println("ENTER STOP");
#endif
}
