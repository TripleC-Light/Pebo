#pragma once
#include "WakeSources.h"
#include "DrinkLogic.h"
#include "DrinkRecords.h"
#include <new>
namespace Observation {
constexpr uint8_t kObserveCtrl1=0x50,kDiscardSamples=2;
constexpr uint32_t kPollMs=2,kFreshTimeoutMs=100,kCalibrationMs=2200,kTracePeriodMs=50;
constexpr uint16_t kTraceCapacity=320;
struct Row {uint16_t ms;Shake::Sample value;};
// Mutually exclusive diagnostic owners; placement construction, no heap.
union Diagnostics {Row trace[kTraceCapacity];Records::Store synthetic;constexpr Diagnostics():trace{}{} };
static Diagnostics diagnostic;
static auto &trace=diagnostic.trace;
static Row fresh{};
static uint16_t rows=0,dumpAt=0;
static bool active=false,dumping=false,calibrating=false,capturing=false,freshReady=false;
static bool shakeArmed=true;
static bool accepted=false;
static uint32_t started=0,lastFresh=0,nextPoll=0,awakeStarted=0,limit=0,nextTrace=0;
static uint8_t discarded=0,failures=0;
static Shake::Detector detector;
static Shake::Gate gate;
static Drink::Reference reference;
static Drink::Calibration calibration;
static Drink::Detector drink;
static Drink::Merge merge;
static uint32_t entries=0,windows=0,successes=0,rejected=0,errors=0;
static uint32_t candidates=0,events=0,cancelledWindows=0,traceDropped=0,calFailures=0;
inline uint32_t bump(uint32_t v){return Drink::bump(v);}
inline bool restore(){
  bool ok=false;
  for(uint8_t i=0;i<3&&!ok;++i)ok=WakeSources::configureMotion();
  Board::motionInterrupt(ok);return ok;
}
inline bool cancel(){
  merge.cancel();capturing=freshReady=false;
  if(!active)return true;
  cancelledWindows=bump(cancelledWindows);drink.abort();active=false;return restore();
}
inline bool begin(uint32_t now,bool cal=false,uint32_t duration=Drink::kObserveMs,bool capture=false){
  Board::motionInterrupt(false);rows=0;dumping=false;discarded=failures=0;
  uint8_t stale[6];
  bool ok=WakeSources::write(0x19,0x23,false,0)&&WakeSources::write(0x19,0x20,false,kObserveCtrl1)&&
    Board::readBlock(0x19,0x28,stale,6);
  if(!ok){errors=bump(errors);return false;}
  active=true;calibrating=cal;capturing=capture;freshReady=false;calibration={};accepted=false;
  windows=bump(windows);started=lastFresh=nextPoll=nextTrace=now;awakeStarted=HAL_GetTick();
  limit=cal?kCalibrationMs:duration;detector.begin(now);
  uint32_t oldRejected=drink.rejected,oldTimeouts=drink.timeouts;
  drink.begin(reference,now);drink.rejected=oldRejected;drink.timeouts=oldTimeouts;return true;
}
// 0 ongoing,1 window ended,2 shake pulse,3 error,4 candidate,5 cal OK,6 cal rejected.
inline uint8_t update(uint32_t now){
  if(!active)return 0;
  if(uint32_t(HAL_GetTick()-awakeStarted)>=limit+250){active=false;errors=bump(errors);drink.abort();return 3;}
  if(uint32_t(now-started)>=limit){
    active=false;
    if(calibrating){bool ok=calibration.finish(reference);if(!ok)calFailures=bump(calFailures);return ok?5:6;}
    if(drink.phase!=Drink::Phase::Idle&&drink.phase!=Drink::Phase::ShakeBlocked)drink.timeouts=bump(drink.timeouts);
    drink.abort();if(!accepted)rejected=bump(rejected);return 1;
  }
  if(int32_t(now-nextPoll)<0)return 0;
  nextPoll=now+kPollMs;uint8_t status=0,raw[6];
  bool ok=WakeSources::read(0x19,0x27,false,status);
  if(ok&&(status&1))ok=Board::readBlock(0x19,0x28,raw,6);
  if(!ok){if(++failures<3)return 0;active=false;errors=bump(errors);drink.abort();return 3;}
  failures=0;
  if(!(status&1)){
    if(uint32_t(now-lastFresh)<kFreshTimeoutMs)return 0;
    active=false;errors=bump(errors);drink.abort();return 3;
  }
  lastFresh=now;
  Shake::Sample a{int16_t(int16_t(uint16_t(raw[0])|(uint16_t(raw[1])<<8))/16),
    int16_t(int16_t(uint16_t(raw[2])|(uint16_t(raw[3])<<8))/16),
    int16_t(int16_t(uint16_t(raw[4])|(uint16_t(raw[5])<<8))/16)};
  if(discarded<kDiscardSamples){++discarded;return 0;}
  if(freshReady)traceDropped=bump(traceDropped);
  fresh={uint16_t(now-started),a};freshReady=capturing;
  if(int32_t(now-nextTrace)>=0&&rows<kTraceCapacity){trace[rows++]={uint16_t(now-started),a};nextTrace=now+kTracePeriodMs;}
  if(calibrating){calibration.feed(a);return 0;}
  uint8_t outcome=Drink::coordinate(detector,drink,gate,shakeArmed,a,now);
  if(outcome==2){successes=bump(successes);accepted=true;}
  if(outcome==4){candidates=bump(candidates);accepted=true;}
  return outcome;
}
}
