#pragma once
#include "ShakeLogic.h"
namespace Drink {
constexpr uint32_t kObserveMs=15000,kHoldMs=1200,kReturnMs=500,kCandidateMs=12000;
constexpr uint32_t kMergeMs=10000,kUnreliableMs=150;
constexpr int32_t kMinGravity=820,kMaxGravity=1230,kMaxStep=180,kStableStep=120;
// Engineering seed values, NOT data-tuned/accepted product parameters.
constexpr uint16_t kEnterCos2=2500,kExitCos2=5000,kUprightCos2=8200;
constexpr uint32_t bump(uint32_t n){return n==UINT32_MAX?n:n+1;}
constexpr Shake::Sample delta(Shake::Sample a,Shake::Sample b){return {int16_t(a.x-b.x),int16_t(a.y-b.y),int16_t(a.z-b.z)};}
constexpr bool gravity(Shake::Sample a){int32_t n=Shake::dot(a,a);return n>=kMinGravity*kMinGravity&&n<=kMaxGravity*kMaxGravity;}
struct Reference {
  Shake::Sample value{};bool valid=false;
};
struct Calibration {
  int32_t sum[3]={};Shake::Sample lo{},hi{};uint16_t count=0;bool bad=false;
  constexpr void feed(Shake::Sample a){
    if(!count)lo=hi=a;
    if(!gravity(a))bad=true;
    int16_t v[3]={a.x,a.y,a.z};
    int16_t *l[3]={&lo.x,&lo.y,&lo.z},*h[3]={&hi.x,&hi.y,&hi.z};
    for(uint8_t i=0;i<3;++i){sum[i]+=v[i];if(v[i]<*l[i])*l[i]=v[i];if(v[i]>*h[i])*h[i]=v[i];}
    ++count;
  }
  constexpr bool finish(Reference &ref)const{
    if(bad||count<150||hi.x-lo.x>60||hi.y-lo.y>60||hi.z-lo.z>60)return false;
    Shake::Sample a{int16_t(sum[0]/count),int16_t(sum[1]/count),int16_t(sum[2]/count)};
    if(!gravity(a))return false;
    ref={a,true};return true;
  }
};
constexpr bool aboveTilt(Shake::Sample a,Shake::Sample ref,uint16_t cos2){
  int64_t d=Shake::dot(a,ref);if(d<=0)return true;
  return d*d*10000<=int64_t(Shake::dot(a,a))*Shake::dot(ref,ref)*cos2;
}
constexpr bool upright(Shake::Sample a,Shake::Sample ref){
  int64_t d=Shake::dot(a,ref);
  return d>0&&d*d*10000>=int64_t(Shake::dot(a,a))*Shake::dot(ref,ref)*kUprightCos2;
}
enum class Phase:uint8_t {Idle,Tilt,Held,Return,ShakeBlocked};
struct Detector {
  Phase phase=Phase::Idle;Reference reference{};Shake::Sample filtered{},previous{};
  uint32_t started=0,tiltAt=0,candidateAt=0,returnAt=0,lastReliable=0,lastSample=0,samples=0;
  uint32_t rejected=0,timeouts=0;bool seeded=false,returnTiming=false;
  constexpr void begin(Reference ref,uint32_t now){*this=Detector{};reference=ref;started=lastReliable=now;}
  constexpr void shake(uint32_t now){phase=Phase::ShakeBlocked;returnTiming=false;lastReliable=now;}
  constexpr void abort(){if(phase!=Phase::Idle&&phase!=Phase::ShakeBlocked)rejected=bump(rejected);phase=Phase::Idle;returnTiming=false;}
  constexpr bool feed(Shake::Sample a,uint32_t now){
    ++samples;
    if(!reference.valid)return false;
    if(seeded&&uint32_t(now-lastSample)>100){abort();seeded=false;}
    lastSample=now;
    bool reliable=gravity(a)&&(!seeded||Shake::dot(delta(a,previous),delta(a,previous))<=kMaxStep*kMaxStep);
    previous=a;
    if(!seeded){filtered=a;seeded=true;}
    if(!reliable){
      if(phase==Phase::Return)phase=Phase::Held;
      if(uint32_t(now-lastReliable)>=kUnreliableMs){
        if(phase==Phase::Tilt)abort();
        returnTiming=false;
      }
      return false;
    }
    auto old=filtered;
    filtered={int16_t(filtered.x+(a.x-filtered.x)/8),int16_t(filtered.y+(a.y-filtered.y)/8),int16_t(filtered.z+(a.z-filtered.z)/8)};
    lastReliable=now;
    bool stable=Shake::dot(delta(a,old),delta(a,old))<=kStableStep*kStableStep;
    if((phase==Phase::Tilt||phase==Phase::Held||phase==Phase::Return)&&uint32_t(now-candidateAt)>=kCandidateMs){
      timeouts=bump(timeouts);abort();return false;
    }
    if(phase==Phase::ShakeBlocked){
      if(stable&&upright(filtered,reference.value)){
        if(!returnTiming){returnTiming=true;returnAt=now;}
        if(uint32_t(now-returnAt)>=kReturnMs){phase=Phase::Idle;returnTiming=false;}
      }else returnTiming=false;
      return false;
    }
    if(phase==Phase::Idle){
      if(stable&&aboveTilt(filtered,reference.value,kEnterCos2)){phase=Phase::Tilt;tiltAt=candidateAt=now;}
      return false;
    }
    if(phase==Phase::Tilt){
      if(!aboveTilt(filtered,reference.value,kExitCos2)){abort();return false;}
      if(uint32_t(now-tiltAt)>=kHoldMs)phase=Phase::Held;
      return false;
    }
    if(upright(filtered,reference.value)&&stable){
      if(phase!=Phase::Return){phase=Phase::Return;returnAt=now;}
      if(uint32_t(now-returnAt)>=kReturnMs){phase=Phase::Idle;return true;}
    }else if(phase==Phase::Return)phase=Phase::Held;
    return false;
  }
};
struct Merge {
  bool pending=false;uint32_t first=0,last=0,candidates=0,confirmedAt=0,cancelled=0;
  constexpr bool due(uint32_t now)const{return pending&&uint32_t(now-last)>=kMergeMs;}
  // Caller must flush due() BEFORE add(): exactly10s belongs to a new event.
  constexpr bool add(uint32_t start,uint32_t now){
    if(due(now))return false;
    if(!pending){pending=true;first=start;candidates=0;}
    last=now;candidates=bump(candidates);return true;
  }
  constexpr bool confirm(uint32_t now){if(!due(now))return false;pending=false;confirmedAt=now;return true;}
  constexpr void cancel(){if(pending)cancelled=bump(cancelled);pending=false;}
};
// Shared by the firmware sampler and measured-data replay, not a second model.
constexpr uint8_t coordinate(Shake::Detector &shakeDetector,Detector &drink,
                            Shake::Gate &gate,bool &shakeArmed,Shake::Sample a,uint32_t now){
  if(!shakeArmed&&gate.allow(now))shakeArmed=true;
  auto step=delta(a,drink.previous);
  if(Shake::dot(step,step)>100*100)gate.motion(now);
  if(shakeDetector.expired(now))shakeDetector.begin(now);
  if(shakeDetector.feed(a,now)){
    drink.shake(now);
    if(shakeArmed){gate.success(now);shakeArmed=false;return 2;}
  }
  return drink.feed(a,now)?4:0;
}
}
