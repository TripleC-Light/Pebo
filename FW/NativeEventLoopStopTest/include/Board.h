#pragma once
#include "stm32l0xx_hal.h"
#include <stdint.h>
namespace Board {
bool clock();
void initializePins();
void lpd(bool high);
bool motionHigh();
bool gpoHigh();
bool lpdHigh();
void motionInterrupt(bool enabled);
void gpoInterrupt(bool enabled);
bool beginBus();
void endBus();
bool read(uint8_t address,uint16_t reg,bool wide,uint8_t &value);
bool write(uint8_t address,uint16_t reg,bool wide,uint8_t value);
class Console {
public:
  bool begin(uint32_t baud);
  bool flush();
  void end();
  unsigned available() const;
  int read();
  unsigned availableForWrite() const;
  void write(uint8_t value);
};
}
extern "C" void SystemClock_Config();
