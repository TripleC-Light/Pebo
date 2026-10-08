#pragma once

#include <Arduino.h>
#include <Wire.h>

struct PeboLis2dw12RawSample {
  int16_t x;
  int16_t y;
  int16_t z;
};

class PeboLIS2DW12 {
public:
  static constexpr uint8_t ADDRESS_LOW = 0x18;
  static constexpr uint8_t ADDRESS_HIGH = 0x19;
  static constexpr uint8_t ADDRESS_AUTO = 0xFF;
  static constexpr uint8_t WHO_AM_I_EXPECTED = 0x44;

  enum Odr : uint8_t {
    ODR_POWER_DOWN = 0x00,
    ODR_1_6_HZ = 0x10,
    ODR_12_5_HZ = 0x20,
    ODR_25_HZ = 0x30,
    ODR_50_HZ = 0x40,
    ODR_100_HZ = 0x50,
    ODR_200_HZ = 0x60
  };

  enum LowPowerMode : uint8_t {
    LOW_POWER_MODE_1 = 0x00,
    LOW_POWER_MODE_2 = 0x01,
    LOW_POWER_MODE_3 = 0x02,
    LOW_POWER_MODE_4 = 0x03
  };

  explicit PeboLIS2DW12(TwoWire &wire = Wire);

  bool begin(uint8_t address = ADDRESS_AUTO);
  bool isPresent();
  uint8_t address() const;

  bool readWhoAmI(uint8_t &whoAmI);
  bool configureLowPower(Odr odr = ODR_25_HZ,
                         LowPowerMode mode = LOW_POWER_MODE_1);
  bool powerDown();
  bool readRaw(PeboLis2dw12RawSample &sample);

  bool readRegister(uint8_t reg, uint8_t &value);
  bool readRegisters(uint8_t startReg, uint8_t *data, uint8_t length);
  bool writeRegister(uint8_t reg, uint8_t value);

private:
  static constexpr uint8_t REG_WHO_AM_I = 0x0F;
  static constexpr uint8_t REG_CTRL1 = 0x20;
  static constexpr uint8_t REG_CTRL2 = 0x21;
  static constexpr uint8_t REG_CTRL6 = 0x25;
  static constexpr uint8_t REG_OUT_X_L = 0x28;

  static constexpr uint8_t CTRL2_BDU_AND_AUTO_INCREMENT = 0x0C;
  static constexpr uint8_t CTRL6_FS_2G = 0x00;

  TwoWire *_wire;
  uint8_t _address;

  bool probeAddress(uint8_t address);
  bool writeCtrl1(Odr odr, LowPowerMode mode);
};
