// Pebo hardware confirmation firmware
// Target: STM32L031G6U6TR using STM32 Arduino core.
//
// This sketch is for PCBA bring-up only. Verify every pin below against the
// current schematic/PCB before flashing to hardware.

#include <Arduino.h>
#include <Wire.h>
#include <strings.h>
#include <stdlib.h>

static const uint32_t UART_BAUD = 115200;
static const uint16_t LINE_MAX = 96;
static const uint32_t I2C_CLOCK_HZ = 100000;

// Pebo PCBA UART pins:
//   USART2: TX=PA9, RX=PA10
// Generic L031G6Ux default Serial pins in STM32 Arduino core are TX=PA2/RX=PA3,
// so this sketch must explicitly bind UART to the PCBA pins below.
// Other variant-supported UART options from PeripheralPins.c:
//   USART2: TX=PA2_ALT1/RX=PA3_ALT1, TX=PA9/RX=PA10,
//           TX=PA14_ALT1/RX=PA15, TX=PB6/RX=PB7
//   LPUART1: TX=PA14/RX=PA13
// Update these two pins to match the verified PCBA USB-UART nets.
static const pin_size_t UART_RX_PIN = PA10;
static const pin_size_t UART_TX_PIN = PA9;
static const char *UART_RX_NAME = "PA10";
static const char *UART_TX_NAME = "PA9";
static Uart DebugUart(UART_RX_PIN, UART_TX_PIN);

// Pebo v0.1 pin map from docs/hardware_pinmap.md.
static const uint32_t LED_BLUE_1_PIN = PA5;
static const uint32_t LED_BLUE_2_PIN = PA6;
static const uint32_t LED_BLUE_3_PIN = PA7;
static const uint32_t LED_BLUE_4_PIN = PA8;
static const uint32_t LED_RED_PIN = PA15;

static const uint32_t PIEZO_A_PIN = PA0;
static const uint32_t PIEZO_B_PIN = PA1;
static const int32_t BUZZER_PIN = PIEZO_A_PIN;
static const uint32_t I2C_SCL_PIN = PB6;
static const uint32_t I2C_SDA_PIN = PB7;
static const uint32_t NFC_GPO_PIN = PA3;
static const uint32_t NFC_LPD_PIN = PA4;
static const uint32_t LIS2_INT1_PIN = PB0;
static const uint32_t LIS2_INT2_PIN = PB1;

static const uint8_t LIS2DW12_ADDR_LOW = 0x18;
static const uint8_t LIS2DW12_ADDR_HIGH = 0x19;
static const uint8_t LIS2DW12_WHO_AM_I_REG = 0x0F;
static const uint8_t LIS2DW12_EXPECTED_ID = 0x44;
static const uint8_t LIS2DW12_CTRL1_REG = 0x20;
static const uint8_t LIS2DW12_CTRL2_REG = 0x21;
static const uint8_t LIS2DW12_CTRL6_REG = 0x25;
static const uint8_t LIS2DW12_STATUS_REG = 0x27;
static const uint8_t LIS2DW12_OUT_X_L_REG = 0x28;

static const uint8_t ST25DV_USER_I2C_ADDR = 0x53;
static const uint8_t ST25DV_SYSTEM_I2C_ADDR = 0x57;

struct OutputChannel {
  const char *name;
  uint32_t pin;
  bool activeHigh;
  bool state;
};

static OutputChannel outputs[] = {
  {"BLUE1", LED_BLUE_1_PIN, true, false},
  {"BLUE2", LED_BLUE_2_PIN, true, false},
  {"BLUE3", LED_BLUE_3_PIN, true, false},
  {"BLUE4", LED_BLUE_4_PIN, true, false},
  {"RED", LED_RED_PIN, true, false},
};

static char lineBuffer[LINE_MAX];
static uint16_t lineLength = 0;
static uint32_t commandCount = 0;
static uint32_t errorCount = 0;
static uint8_t lastI2cDeviceCount = 0;
static bool lastLis2Present = false;
static bool lastSt25UserPresent = false;
static bool lastSt25SystemPresent = false;

static void printHex8(uint8_t value)
{
  if (value < 0x10) {
    DebugUart.print('0');
  }
  DebugUart.print(value, HEX);
}

static void printHex16(uint16_t value)
{
  DebugUart.print("0x");
  if (value < 0x1000) {
    DebugUart.print('0');
  }
  if (value < 0x0100) {
    DebugUart.print('0');
  }
  if (value < 0x0010) {
    DebugUart.print('0');
  }
  DebugUart.print(value, HEX);
}

static bool parseUint(const char *text, uint32_t &value)
{
  if ((text == nullptr) || (*text == '\0')) {
    return false;
  }

  char *end = nullptr;
  value = strtoul(text, &end, 0);
  return (end != text) && (*end == '\0');
}

static void writeOutput(OutputChannel &channel, bool on)
{
  channel.state = on;
  const uint8_t level = (on == channel.activeHigh) ? HIGH : LOW;
  digitalWrite(channel.pin, level);
}

static OutputChannel *findOutput(const char *name)
{
  for (size_t i = 0; i < sizeof(outputs) / sizeof(outputs[0]); ++i) {
    if (strcasecmp(outputs[i].name, name) == 0) {
      return &outputs[i];
    }
  }
  return nullptr;
}

static void printOk(const char *message)
{
  DebugUart.print("OK ");
  DebugUart.println(message);
}

static void printError(const char *message)
{
  ++errorCount;
  DebugUart.print("ERR ");
  DebugUart.println(message);
}

static void printHelp()
{
  DebugUart.println("OK COMMANDS");
  DebugUart.println("INFO");
  DebugUart.println("STATUS");
  DebugUart.println("LED <BLUE1|BLUE2|BLUE3|BLUE4|RED|ALL> <ON|OFF|TOGGLE>");
  DebugUart.println("BLINK <BLUE1|BLUE2|BLUE3|BLUE4|RED|ALL> <COUNT> <MS>");
  DebugUart.println("BUZZ <FREQ_HZ> <MS>");
  DebugUart.println("I2CSCAN");
  DebugUart.println("LIS2INIT");
  DebugUart.println("LIS2");
  DebugUart.println("LIS2READ <REG_HEX> [LEN]");
  DebugUart.println("ST25");
  DebugUart.println("ST25READ <ADDR16_HEX> [LEN]");
  DebugUart.println("PING");
  DebugUart.println("HELP");
}

static void printInfo()
{
  DebugUart.println("INFO name=PeboHardwareTest proto=1 baud=115200");
  DebugUart.print("INFO uart_tx=");
  DebugUart.print(UART_TX_NAME);
  DebugUart.print(" uart_rx=");
  DebugUart.println(UART_RX_NAME);
  DebugUart.println("INFO i2c_scl=PB6 i2c_sda=PB7 i2c_hz=100000");
  DebugUart.println("INFO lis2dw12_addr_candidates=0x18,0x19 whoami_expected=0x44");
  DebugUart.println("INFO st25dv_user_addr=0x53 st25dv_system_addr=0x57");
  DebugUart.println("INFO mcu=STM32L031G6U6TR board=Generic_L031G6Ux framework=Arduino");
  DebugUart.println("INFO note=pin_map_requires_schematic_verification");
}

static void printStatus()
{
  DebugUart.print("STATUS uptime_ms=");
  DebugUart.print(millis());
  DebugUart.print(" commands=");
  DebugUart.print(commandCount);
  DebugUart.print(" errors=");
  DebugUart.print(errorCount);

  for (size_t i = 0; i < sizeof(outputs) / sizeof(outputs[0]); ++i) {
    DebugUart.print(" led_");
    DebugUart.print(outputs[i].name);
    DebugUart.print("=");
    DebugUart.print(outputs[i].state ? "ON" : "OFF");
  }

  DebugUart.print(" buzzer=");
  DebugUart.print(BUZZER_PIN >= 0 ? "CONFIGURED" : "DISABLED");
  DebugUart.print(" piezo_a=PA0 piezo_b=PA1");
  DebugUart.print(" i2c_devices=");
  DebugUart.print(lastI2cDeviceCount);
  DebugUart.print(" lis2=");
  DebugUart.print(lastLis2Present ? "PRESENT" : "UNKNOWN");
  DebugUart.print(" st25_user=");
  DebugUart.print(lastSt25UserPresent ? "PRESENT" : "UNKNOWN");
  DebugUart.print(" st25_system=");
  DebugUart.print(lastSt25SystemPresent ? "PRESENT" : "UNKNOWN");
  DebugUart.print(" nfc_gpo=");
  DebugUart.print(digitalRead(NFC_GPO_PIN));
  DebugUart.print(" lis2_int1=");
  DebugUart.print(digitalRead(LIS2_INT1_PIN));
  DebugUart.print(" lis2_int2=");
  DebugUart.print(digitalRead(LIS2_INT2_PIN));
  DebugUart.println();
}

static bool i2cAddressAck(uint8_t address)
{
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

static bool i2cReadReg8(uint8_t address, uint8_t reg, uint8_t *data, uint8_t length)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  const uint8_t received = Wire.requestFrom(address, length);
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

static bool i2cWriteReg8(uint8_t address, uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

static bool i2cReadMem16(uint8_t address, uint16_t memAddress, uint8_t *data, uint8_t length)
{
  Wire.beginTransmission(address);
  Wire.write(static_cast<uint8_t>(memAddress >> 8));
  Wire.write(static_cast<uint8_t>(memAddress & 0xFF));
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  const uint8_t received = Wire.requestFrom(address, length);
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

static void printBytes(const uint8_t *data, uint8_t length)
{
  for (uint8_t i = 0; i < length; ++i) {
    if (i > 0) {
      DebugUart.print(',');
    }
    DebugUart.print("0x");
    printHex8(data[i]);
  }
}

static void handleI2cScan()
{
  uint8_t found = 0;
  DebugUart.print("I2C_SCAN");

  for (uint8_t address = 0x08; address <= 0x77; ++address) {
    if (i2cAddressAck(address)) {
      ++found;
      DebugUart.print(" 0x");
      printHex8(address);
    }
  }

  lastI2cDeviceCount = found;
  DebugUart.print(" count=");
  DebugUart.println(found);
}

static bool readLis2WhoAmI(uint8_t address, uint8_t &whoAmI)
{
  return i2cReadReg8(address, LIS2DW12_WHO_AM_I_REG, &whoAmI, 1);
}

static void printLis2ProbeForAddress(uint8_t address)
{
  uint8_t whoAmI = 0;
  const bool ack = i2cAddressAck(address);
  const bool readOk = ack && readLis2WhoAmI(address, whoAmI);
  const bool matched = readOk && (whoAmI == LIS2DW12_EXPECTED_ID);

  DebugUart.print("LIS2 addr=0x");
  printHex8(address);
  DebugUart.print(" ack=");
  DebugUart.print(ack ? "YES" : "NO");
  DebugUart.print(" whoami=");
  if (readOk) {
    DebugUart.print("0x");
    printHex8(whoAmI);
  } else {
    DebugUart.print("NA");
  }
  DebugUart.print(" match=");
  DebugUart.println(matched ? "YES" : "NO");

  if (matched) {
    lastLis2Present = true;
  }
}

static uint8_t lis2ActiveAddress()
{
  uint8_t whoAmI = 0;
  if (readLis2WhoAmI(LIS2DW12_ADDR_HIGH, whoAmI) && (whoAmI == LIS2DW12_EXPECTED_ID)) {
    return LIS2DW12_ADDR_HIGH;
  }
  return LIS2DW12_ADDR_LOW;
}

static bool lis2Init(uint8_t address)
{
  // CTRL2: BDU=1, IF_ADD_INC=1. This keeps X/Y/Z reads coherent and enables
  // register auto-increment for multi-byte reads.
  const bool ctrl2Ok = i2cWriteReg8(address, LIS2DW12_CTRL2_REG, 0x0C);

  // CTRL6: +/-2g full scale, low-noise off, default bandwidth.
  const bool ctrl6Ok = i2cWriteReg8(address, LIS2DW12_CTRL6_REG, 0x00);

  // CTRL1: ODR=25 Hz, low-power mode 1. Default CTRL1 is power-down, which
  // leaves output registers at zero.
  const bool ctrl1Ok = i2cWriteReg8(address, LIS2DW12_CTRL1_REG, 0x31);
  delay(80);
  return ctrl1Ok && ctrl2Ok && ctrl6Ok;
}

static void handleLis2Init()
{
  const uint8_t address = lis2ActiveAddress();
  const bool ok = lis2Init(address);
  uint8_t ctrl1 = 0;
  uint8_t ctrl2 = 0;
  uint8_t ctrl6 = 0;
  const bool ctrl1Read = i2cReadReg8(address, LIS2DW12_CTRL1_REG, &ctrl1, 1);
  const bool ctrl2Read = i2cReadReg8(address, LIS2DW12_CTRL2_REG, &ctrl2, 1);
  const bool ctrl6Read = i2cReadReg8(address, LIS2DW12_CTRL6_REG, &ctrl6, 1);

  DebugUart.print("LIS2_INIT addr=0x");
  printHex8(address);
  DebugUart.print(" ok=");
  DebugUart.print(ok ? "YES" : "NO");
  DebugUart.print(" ctrl1=");
  if (ctrl1Read) {
    DebugUart.print("0x");
    printHex8(ctrl1);
  } else {
    DebugUart.print("NA");
  }
  DebugUart.print(" ctrl2=");
  if (ctrl2Read) {
    DebugUart.print("0x");
    printHex8(ctrl2);
  } else {
    DebugUart.print("NA");
  }
  DebugUart.print(" ctrl6=");
  if (ctrl6Read) {
    DebugUart.print("0x");
    printHex8(ctrl6);
  } else {
    DebugUart.print("NA");
  }
  DebugUart.println();
}

static void handleLis2()
{
  lastLis2Present = false;
  printLis2ProbeForAddress(LIS2DW12_ADDR_LOW);
  printLis2ProbeForAddress(LIS2DW12_ADDR_HIGH);

  const uint8_t activeAddress = lis2ActiveAddress();
  uint8_t status = 0;
  uint8_t xyz[6] = {0};
  const bool statusOk = i2cReadReg8(activeAddress, LIS2DW12_STATUS_REG, &status, 1);
  const bool xyzOk = i2cReadReg8(activeAddress, LIS2DW12_OUT_X_L_REG, xyz, sizeof(xyz));

  DebugUart.print("LIS2_DATA addr=0x");
  printHex8(activeAddress);
  DebugUart.print(" status=");
  if (statusOk) {
    DebugUart.print("0x");
    printHex8(status);
  } else {
    DebugUart.print("NA");
  }
  DebugUart.print(" xyz_raw=");
  if (xyzOk) {
    printBytes(xyz, sizeof(xyz));
  } else {
    DebugUart.print("NA");
  }
  DebugUart.println();
}

static void handleLis2Read(char *regText, char *lengthText)
{
  uint32_t regValue = 0;
  uint32_t lengthValue = 1;
  if (!parseUint(regText, regValue) || (regValue > 0xFF)) {
    printError("usage_LIS2READ_reg_hex_len");
    return;
  }
  if ((lengthText != nullptr) && (!parseUint(lengthText, lengthValue))) {
    printError("bad_length");
    return;
  }

  const uint8_t length = constrain(lengthValue, 1, 16);
  uint8_t data[16] = {0};
  const uint8_t activeAddress = lis2ActiveAddress();
  const bool ok = i2cReadReg8(activeAddress, static_cast<uint8_t>(regValue), data, length);

  DebugUart.print("LIS2_READ addr=0x");
  printHex8(activeAddress);
  DebugUart.print(" reg=0x");
  printHex8(static_cast<uint8_t>(regValue));
  DebugUart.print(" ok=");
  DebugUart.print(ok ? "YES" : "NO");
  DebugUart.print(" data=");
  if (ok) {
    printBytes(data, length);
  } else {
    DebugUart.print("NA");
  }
  DebugUart.println();
}

static void handleSt25()
{
  lastSt25UserPresent = i2cAddressAck(ST25DV_USER_I2C_ADDR);
  lastSt25SystemPresent = i2cAddressAck(ST25DV_SYSTEM_I2C_ADDR);

  uint8_t userBytes[16] = {0};
  const bool userReadOk = lastSt25UserPresent &&
                          i2cReadMem16(ST25DV_USER_I2C_ADDR, 0x0000, userBytes, sizeof(userBytes));

  DebugUart.print("ST25 addr_user=0x");
  printHex8(ST25DV_USER_I2C_ADDR);
  DebugUart.print(" ack_user=");
  DebugUart.print(lastSt25UserPresent ? "YES" : "NO");
  DebugUart.print(" addr_system=0x");
  printHex8(ST25DV_SYSTEM_I2C_ADDR);
  DebugUart.print(" ack_system=");
  DebugUart.print(lastSt25SystemPresent ? "YES" : "NO");
  DebugUart.print(" gpo=");
  DebugUart.print(digitalRead(NFC_GPO_PIN));
  DebugUart.print(" user0=");
  if (userReadOk) {
    printBytes(userBytes, sizeof(userBytes));
  } else {
    DebugUart.print("NA");
  }
  DebugUart.println();
}

static void handleSt25Read(char *addrText, char *lengthText)
{
  uint32_t memAddress = 0;
  uint32_t lengthValue = 16;
  if (!parseUint(addrText, memAddress) || (memAddress > 0xFFFF)) {
    printError("usage_ST25READ_addr16_hex_len");
    return;
  }
  if ((lengthText != nullptr) && (!parseUint(lengthText, lengthValue))) {
    printError("bad_length");
    return;
  }

  const uint8_t length = constrain(lengthValue, 1, 32);
  uint8_t data[32] = {0};
  const bool ok = i2cReadMem16(ST25DV_USER_I2C_ADDR, static_cast<uint16_t>(memAddress), data, length);

  DebugUart.print("ST25_READ i2c=0x");
  printHex8(ST25DV_USER_I2C_ADDR);
  DebugUart.print(" addr=");
  printHex16(static_cast<uint16_t>(memAddress));
  DebugUart.print(" ok=");
  DebugUart.print(ok ? "YES" : "NO");
  DebugUart.print(" data=");
  if (ok) {
    printBytes(data, length);
  } else {
    DebugUart.print("NA");
  }
  DebugUart.println();
}

static void setOneLed(OutputChannel &channel, const char *action)
{
  if (strcasecmp(action, "ON") == 0) {
    writeOutput(channel, true);
  } else if (strcasecmp(action, "OFF") == 0) {
    writeOutput(channel, false);
  } else if (strcasecmp(action, "TOGGLE") == 0) {
    writeOutput(channel, !channel.state);
  } else {
    printError("bad_led_action");
    return;
  }

  printOk("led");
}

static void setAllLeds(const char *action)
{
  if ((strcasecmp(action, "ON") != 0) &&
      (strcasecmp(action, "OFF") != 0) &&
      (strcasecmp(action, "TOGGLE") != 0)) {
    printError("bad_led_action");
    return;
  }

  for (size_t i = 0; i < sizeof(outputs) / sizeof(outputs[0]); ++i) {
    if (strcasecmp(action, "ON") == 0) {
      writeOutput(outputs[i], true);
    } else if (strcasecmp(action, "OFF") == 0) {
      writeOutput(outputs[i], false);
    } else {
      writeOutput(outputs[i], !outputs[i].state);
    }
  }
  printOk("led_all");
}

static void handleLed(char *name, char *action)
{
  if ((name == nullptr) || (action == nullptr)) {
    printError("usage_LED_name_action");
    return;
  }

  if (strcasecmp(name, "ALL") == 0) {
    setAllLeds(action);
    return;
  }

  OutputChannel *channel = findOutput(name);
  if (channel == nullptr) {
    printError("unknown_led");
    return;
  }

  setOneLed(*channel, action);
}

static void blinkChannel(OutputChannel &channel, uint16_t count, uint16_t ms)
{
  for (uint16_t i = 0; i < count; ++i) {
    writeOutput(channel, true);
    delay(ms);
    writeOutput(channel, false);
    delay(ms);
  }
}

static void handleBlink(char *name, char *countText, char *msText)
{
  if ((name == nullptr) || (countText == nullptr) || (msText == nullptr)) {
    printError("usage_BLINK_name_count_ms");
    return;
  }

  const uint16_t count = constrain(atoi(countText), 1, 20);
  const uint16_t ms = constrain(atoi(msText), 10, 2000);

  if (strcasecmp(name, "ALL") == 0) {
    for (uint16_t i = 0; i < count; ++i) {
      for (size_t j = 0; j < sizeof(outputs) / sizeof(outputs[0]); ++j) {
        writeOutput(outputs[j], true);
      }
      delay(ms);
      for (size_t j = 0; j < sizeof(outputs) / sizeof(outputs[0]); ++j) {
        writeOutput(outputs[j], false);
      }
      delay(ms);
    }
    printOk("blink_all");
    return;
  }

  OutputChannel *channel = findOutput(name);
  if (channel == nullptr) {
    printError("unknown_led");
    return;
  }

  blinkChannel(*channel, count, ms);
  printOk("blink");
}

static void handleBuzz(char *freqText, char *msText)
{
  if ((freqText == nullptr) || (msText == nullptr)) {
    printError("usage_BUZZ_freq_ms");
    return;
  }

  if (BUZZER_PIN < 0) {
    printError("buzzer_pin_not_configured");
    return;
  }

  const uint16_t freq = constrain(atoi(freqText), 50, 10000);
  const uint16_t ms = constrain(atoi(msText), 10, 5000);
  tone(static_cast<uint32_t>(BUZZER_PIN), freq, ms);
  delay(ms);
  noTone(static_cast<uint32_t>(BUZZER_PIN));
  printOk("buzz");
}

static void handleCommand(char *line)
{
  char *command = strtok(line, " \t");
  if (command == nullptr) {
    return;
  }

  ++commandCount;

  if (strcasecmp(command, "PING") == 0) {
    printOk("PONG");
  } else if (strcasecmp(command, "HELP") == 0) {
    printHelp();
  } else if (strcasecmp(command, "INFO") == 0) {
    printInfo();
  } else if (strcasecmp(command, "STATUS") == 0) {
    printStatus();
  } else if (strcasecmp(command, "LED") == 0) {
    handleLed(strtok(nullptr, " \t"), strtok(nullptr, " \t"));
  } else if (strcasecmp(command, "BLINK") == 0) {
    handleBlink(strtok(nullptr, " \t"), strtok(nullptr, " \t"), strtok(nullptr, " \t"));
  } else if (strcasecmp(command, "BUZZ") == 0) {
    handleBuzz(strtok(nullptr, " \t"), strtok(nullptr, " \t"));
  } else if (strcasecmp(command, "I2CSCAN") == 0) {
    handleI2cScan();
  } else if (strcasecmp(command, "LIS2") == 0) {
    handleLis2();
  } else if (strcasecmp(command, "LIS2INIT") == 0) {
    handleLis2Init();
  } else if (strcasecmp(command, "LIS2READ") == 0) {
    handleLis2Read(strtok(nullptr, " \t"), strtok(nullptr, " \t"));
  } else if (strcasecmp(command, "ST25") == 0) {
    handleSt25();
  } else if (strcasecmp(command, "ST25READ") == 0) {
    handleSt25Read(strtok(nullptr, " \t"), strtok(nullptr, " \t"));
  } else {
    printError("unknown_command");
  }
}

static void readSerial()
{
  while (DebugUart.available() > 0) {
    const char c = static_cast<char>(DebugUart.read());
    if ((c == '\n') || (c == '\r')) {
      if (lineLength > 0) {
        lineBuffer[lineLength] = '\0';
        handleCommand(lineBuffer);
        lineLength = 0;
      }
    } else if (lineLength < (LINE_MAX - 1)) {
      lineBuffer[lineLength++] = c;
    } else {
      lineLength = 0;
      printError("line_too_long");
    }
  }
}

void setup()
{
  DebugUart.begin(UART_BAUD);

  for (size_t i = 0; i < sizeof(outputs) / sizeof(outputs[0]); ++i) {
    pinMode(outputs[i].pin, OUTPUT);
    writeOutput(outputs[i], false);
  }

  if (BUZZER_PIN >= 0) {
    pinMode(static_cast<uint32_t>(BUZZER_PIN), OUTPUT);
    digitalWrite(static_cast<uint32_t>(BUZZER_PIN), LOW);
  }
  pinMode(PIEZO_B_PIN, OUTPUT);
  digitalWrite(PIEZO_B_PIN, LOW);
  pinMode(NFC_GPO_PIN, INPUT);
  pinMode(NFC_LPD_PIN, OUTPUT);
  digitalWrite(NFC_LPD_PIN, LOW);
  pinMode(LIS2_INT1_PIN, INPUT);
  pinMode(LIS2_INT2_PIN, INPUT);

  Wire.setSCL(I2C_SCL_PIN);
  Wire.setSDA(I2C_SDA_PIN);
  Wire.begin();
  Wire.setClock(I2C_CLOCK_HZ);
  lis2Init(lis2ActiveAddress());

  delay(100);
  DebugUart.println("READY PeboHardwareTest proto=1");
  printInfo();
  printStatus();
}

void loop()
{
  readSerial();
}
