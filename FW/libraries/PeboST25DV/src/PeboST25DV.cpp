#include "PeboST25DV.h"

PeboST25DV::PeboST25DV(TwoWire &wire)
  : _wire(&wire),
    _lpdPin(NO_LPD_PIN),
    _lowPowerDownRequested(false)
{
}

bool PeboST25DV::begin(uint32_t lpdPin, bool wakeDevice)
{
  _lpdPin = lpdPin;
  _lowPowerDownRequested = false;

  if (hasLowPowerDownPin()) {
    pinMode(_lpdPin, OUTPUT);
    digitalWrite(_lpdPin, wakeDevice ? LOW : HIGH);
    _lowPowerDownRequested = !wakeDevice;
    if (wakeDevice) {
      delayMicroseconds(1000);
    }
  }

  return wakeDevice ? userInterfacePresent() : true;
}

bool PeboST25DV::userInterfacePresent()
{
  return addressAck(USER_I2C_ADDRESS);
}

bool PeboST25DV::systemInterfacePresent()
{
  return addressAck(SYSTEM_I2C_ADDRESS);
}

bool PeboST25DV::readUserMemory(uint16_t address, uint8_t *data, uint8_t length)
{
  if ((data == nullptr) || (length == 0) || _lowPowerDownRequested) {
    return false;
  }

  _wire->beginTransmission(USER_I2C_ADDRESS);
  _wire->write(static_cast<uint8_t>(address >> 8));
  _wire->write(static_cast<uint8_t>(address & 0xFF));
  if (_wire->endTransmission(false) != 0) {
    return false;
  }

  const uint8_t received = _wire->requestFrom(USER_I2C_ADDRESS, length);
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

bool PeboST25DV::writeUserMemory(uint16_t address, const uint8_t *data, uint8_t length)
{
  if ((data == nullptr) || (length == 0) || _lowPowerDownRequested) {
    return false;
  }

  _wire->beginTransmission(USER_I2C_ADDRESS);
  _wire->write(static_cast<uint8_t>(address >> 8));
  _wire->write(static_cast<uint8_t>(address & 0xFF));
  for (uint8_t i = 0; i < length; ++i) {
    _wire->write(data[i]);
  }
  return _wire->endTransmission() == 0;
}

bool PeboST25DV::hasLowPowerDownPin() const
{
  return _lpdPin != NO_LPD_PIN;
}

bool PeboST25DV::enterLowPowerDown()
{
  if (!hasLowPowerDownPin()) {
    return false;
  }

  digitalWrite(_lpdPin, HIGH);
  _lowPowerDownRequested = true;
  return true;
}

bool PeboST25DV::exitLowPowerDown(uint16_t bootDelayUs)
{
  if (!hasLowPowerDownPin()) {
    return false;
  }

  digitalWrite(_lpdPin, LOW);
  _lowPowerDownRequested = false;
  if (bootDelayUs > 0) {
    delayMicroseconds(bootDelayUs);
  }
  return true;
}

bool PeboST25DV::isLowPowerDownRequested() const
{
  return _lowPowerDownRequested;
}

bool PeboST25DV::addressAck(uint8_t i2cAddress)
{
  if (_lowPowerDownRequested) {
    return false;
  }

  _wire->beginTransmission(i2cAddress);
  return _wire->endTransmission() == 0;
}
