#include "PeboLIS2DW12.h"

PeboLIS2DW12::PeboLIS2DW12(TwoWire &wire)
  : _wire(&wire), _address(ADDRESS_HIGH)
{
}

bool PeboLIS2DW12::begin(uint8_t address)
{
  if (address == ADDRESS_AUTO) {
    if (probeAddress(ADDRESS_HIGH)) {
      _address = ADDRESS_HIGH;
    } else if (probeAddress(ADDRESS_LOW)) {
      _address = ADDRESS_LOW;
    } else {
      return false;
    }
  } else {
    _address = address;
    if (!probeAddress(_address)) {
      return false;
    }
  }

  return writeRegister(REG_CTRL2, CTRL2_BDU_AND_AUTO_INCREMENT) &&
         writeRegister(REG_CTRL6, CTRL6_FS_2G);
}

bool PeboLIS2DW12::isPresent()
{
  return probeAddress(_address);
}

uint8_t PeboLIS2DW12::address() const
{
  return _address;
}

bool PeboLIS2DW12::readWhoAmI(uint8_t &whoAmI)
{
  return readRegister(REG_WHO_AM_I, whoAmI);
}

bool PeboLIS2DW12::configureLowPower(Odr odr, LowPowerMode mode)
{
  return writeRegister(REG_CTRL2, CTRL2_BDU_AND_AUTO_INCREMENT) &&
         writeRegister(REG_CTRL6, CTRL6_FS_2G) &&
         writeCtrl1(odr, mode);
}

bool PeboLIS2DW12::powerDown()
{
  return writeCtrl1(ODR_POWER_DOWN, LOW_POWER_MODE_1);
}

bool PeboLIS2DW12::readRaw(PeboLis2dw12RawSample &sample)
{
  uint8_t data[6] = {0};
  if (!readRegisters(REG_OUT_X_L, data, sizeof(data))) {
    return false;
  }

  sample.x = static_cast<int16_t>((static_cast<uint16_t>(data[1]) << 8) | data[0]);
  sample.y = static_cast<int16_t>((static_cast<uint16_t>(data[3]) << 8) | data[2]);
  sample.z = static_cast<int16_t>((static_cast<uint16_t>(data[5]) << 8) | data[4]);
  return true;
}

bool PeboLIS2DW12::readRegister(uint8_t reg, uint8_t &value)
{
  return readRegisters(reg, &value, 1);
}

bool PeboLIS2DW12::readRegisters(uint8_t startReg, uint8_t *data, uint8_t length)
{
  if ((data == nullptr) || (length == 0)) {
    return false;
  }

  _wire->beginTransmission(_address);
  _wire->write(startReg);
  if (_wire->endTransmission(false) != 0) {
    return false;
  }

  const uint8_t received = _wire->requestFrom(_address, length);
  if (received != length) {
    while (_wire->available() > 0) {
      _wire->read();
    }
    return false;
  }

  for (uint8_t i = 0; i < length; ++i) {
    data[i] = static_cast<uint8_t>(_wire->read());
  }
  return true;
}

bool PeboLIS2DW12::writeRegister(uint8_t reg, uint8_t value)
{
  _wire->beginTransmission(_address);
  _wire->write(reg);
  _wire->write(value);
  return _wire->endTransmission() == 0;
}

bool PeboLIS2DW12::probeAddress(uint8_t address)
{
  uint8_t previous = _address;
  uint8_t whoAmI = 0;
  _address = address;
  const bool ok = readWhoAmI(whoAmI) && (whoAmI == WHO_AM_I_EXPECTED);
  _address = previous;
  return ok;
}

bool PeboLIS2DW12::writeCtrl1(Odr odr, LowPowerMode mode)
{
  const uint8_t ctrl1 = static_cast<uint8_t>(odr) |
                        static_cast<uint8_t>(mode & 0x03);
  return writeRegister(REG_CTRL1, ctrl1);
}
