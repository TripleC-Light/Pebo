#pragma once
#include "Board.h"
namespace WakeSources {
constexpr uint8_t kLisAddress=0x19,kStUser=0x53,kStSystem=0x57;
using Board::beginBus;
using Board::endBus;
using Board::read;
using Board::write;
struct StAccess {
  StAccess(){Board::lpd(false);HAL_Delay(1);}
  ~StAccess(){Board::lpd(true);}
};
inline bool configureMotion() {
  uint8_t who=0;
  if (!read(kLisAddress,0x0f,false,who)||who!=0x44) return false;
  // Power down/disable routing first; no sensor reset or unrelated application setup.
  const uint8_t config[][2]={{0x23,0},{0x20,0},{0x21,0x0c},{0x22,0x10},{0x24,0},
    {0x25,0},{0x34,2},{0x35,0},{0x3f,0x20},{0x20,0x20},{0x23,0x20}};
  for (const auto &pair:config) if(!write(kLisAddress,pair[0],false,pair[1]))return false;
  uint8_t ignored=0;
  return read(kLisAddress,0x38,false,ignored)&&read(kLisAddress,0x3b,false,ignored);
}
inline bool stopMotion() {
  // Keep both writes bounded even if the first fails.
  bool route=write(kLisAddress,0x23,false,0);
  bool odr=write(kLisAddress,0x20,false,0);return route&&odr;
}
inline bool configureGpo(uint8_t &staticConfig) {
  StAccess awake;
  if (!read(kStSystem,0,true,staticConfig) || !(staticConfig&0x10)) return false;
  // Static FIELD_CHANGE must already be enabled: never write system EEPROM.
  uint8_t check=0,ignored=0;
  return write(kStUser,0x2000,true,1)&&read(kStUser,0x2000,true,check)&&(check&1)&&
    read(kStUser,0x2005,true,ignored);
}
inline bool stopGpo() { StAccess awake;return write(kStUser,0x2000,true,0); }
inline bool motionCause(uint8_t &wake,uint8_t &all) {
  return read(kLisAddress,0x38,false,wake)&&read(kLisAddress,0x3b,false,all);
}
inline bool gpoCause(uint8_t &cause,int8_t &field) {
  StAccess awake;uint8_t eh=0;
  bool status=read(kStUser,0x2005,true,cause);
  bool state=read(kStUser,0x2002,true,eh);
  field=state?int8_t(bool(eh&4)):-1;
  return status&&state;
}
}
