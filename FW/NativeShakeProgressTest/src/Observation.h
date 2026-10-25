#pragma once
#include "WakeSources.h"
#include "ShakeLogic.h"
namespace Observation {
constexpr uint8_t kObserveCtrl1=0x50,kDiscardSamples=2;
constexpr uint32_t kPollMs=2,kFreshTimeoutMs=100;
constexpr uint16_t kTraceCapacity=160;
struct Row {uint16_t ms;Shake::Sample value;};
static Row trace[kTraceCapacity];
static uint16_t rows=0,dumpAt=0;
static bool active=false,dumping=false;
static uint32_t started=0,lastFresh=0,nextPoll=0,awakeStarted=0;
static uint8_t discarded=0,failures=0;
static Shake::Detector detector;
static Shake::Gate gate;
static uint32_t entries=0,windows=0,successes=0,rejected=0,errors=0;
inline uint32_t bump(uint32_t v){return v==UINT32_MAX?v:v+1;}
inline bool restore(){
  bool ok=false;
  for(uint8_t i=0;i<3&&!ok;++i)ok=WakeSources::configureMotion();
  Board::motionInterrupt(ok);return ok;
}
inline bool cancel(){
  if(!active)return true;
  active=false;return restore();
}
inline bool begin(uint32_t now){
  Board::motionInterrupt(false);rows=0;dumping=false;discarded=failures=0;
  uint8_t stale[6];
  bool ok=WakeSources::write(0x19,0x23,false,0)&&WakeSources::write(0x19,0x20,false,kObserveCtrl1)&&
    Board::readBlock(0x19,0x28,stale,6);
  if(!ok){errors=bump(errors);return false;}
  active=true;windows=bump(windows);started=lastFresh=nextPoll=now;awakeStarted=HAL_GetTick();detector.begin(now);return true;
}
// 0 ongoing, 1 rejected/timeout, 2 confirmed, 3 sensor error.
inline uint8_t update(uint32_t now){
  if(!active)return 0;
  // Sampling never sleeps: independent awake guard also bounds a stalled RTC.
  if(uint32_t(HAL_GetTick()-awakeStarted)>=Shake::kWindowMs+250){
    active=false;errors=bump(errors);return 3;
  }
  if(detector.expired(now)){
    active=false;rejected=bump(rejected);return 1;
  }
  if(int32_t(now-nextPoll)<0)return 0;
  nextPoll=now+kPollMs;uint8_t status=0,raw[6];
  bool ok=WakeSources::read(0x19,0x27,false,status);
  if(ok && (status&1))ok=Board::readBlock(0x19,0x28,raw,6);
  if(!ok){
    if(++failures<3)return 0;
    active=false;errors=bump(errors);return 3;
  }
  failures=0;
  if(!(status&1)){
    if(uint32_t(now-lastFresh)<kFreshTimeoutMs)return 0;
    active=false;errors=bump(errors);return 3;
  }
  lastFresh=now;
  // LP1: signed left-justified 12-bit; division exact for valid samples.
  Shake::Sample a{int16_t(int16_t(uint16_t(raw[0])|(uint16_t(raw[1])<<8))/16),
    int16_t(int16_t(uint16_t(raw[2])|(uint16_t(raw[3])<<8))/16),
    int16_t(int16_t(uint16_t(raw[4])|(uint16_t(raw[5])<<8))/16)};
  if(discarded<kDiscardSamples){++discarded;return 0;}
  if(rows<kTraceCapacity)trace[rows++]={uint16_t(now-started),a};
  if(!detector.feed(a,now))return 0;
  active=false;successes=bump(successes);gate.success(now);return 2;
}
}
