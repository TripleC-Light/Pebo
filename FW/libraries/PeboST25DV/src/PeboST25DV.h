#pragma once

#include <Arduino.h>
#include <Wire.h>

class PeboST25DV {
public:
  static constexpr uint8_t USER_I2C_ADDRESS = 0x53;
  static constexpr uint8_t SYSTEM_I2C_ADDRESS = 0x57;
  static constexpr uint32_t NO_LPD_PIN = 0xFFFFFFFFUL;

  explicit PeboST25DV(TwoWire &wire = Wire);

  bool begin(uint32_t lpdPin = NO_LPD_PIN, bool wakeDevice = true);
  bool userInterfacePresent();
  bool systemInterfacePresent();

  bool readUserMemory(uint16_t address, uint8_t *data, uint8_t length);
  bool writeUserMemory(uint16_t address, const uint8_t *data, uint8_t length);

  bool hasLowPowerDownPin() const;
  bool enterLowPowerDown();
  bool exitLowPowerDown(uint16_t bootDelayUs = 1000);
  bool isLowPowerDownRequested() const;

private:
  TwoWire *_wire;
  uint32_t _lpdPin;
  bool _lowPowerDownRequested;

  bool addressAck(uint8_t i2cAddress);
};
