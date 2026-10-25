#pragma once
#include <stdint.h>
namespace Shake {
constexpr uint32_t kWindowMs=1500,kCooldownMs=2000,kQuietMs=500,kMinReverseMs=80;
constexpr int32_t kAmplitude=300;
constexpr uint8_t kReversals=3,kLag=5;
struct Sample { int16_t x=0,y=0,z=0; };
constexpr int32_t dot(Sample a,Sample b) {
  return int32_t(a.x)*b.x+int32_t(a.y)*b.y+int32_t(a.z)*b.z;
}
struct Detector {
  Sample lag[kLag]={},previous{};
  uint8_t pos=0,filled=0,reversals=0;
  uint32_t started=0,lastReverse=0,peak2=0,samples=0;
  bool havePeak=false,confirmed=false;
  constexpr void begin(uint32_t now) { *this=Detector{};started=lastReverse=now; }
  constexpr bool expired(uint32_t now)const { return uint32_t(now-started)>=kWindowMs; }
  constexpr bool feed(Sample a,uint32_t now) {
    if(confirmed || expired(now))return false;
    Sample old=lag[pos];lag[pos]=a;pos=(pos+1)%kLag;++samples;
    if(filled<kLag){++filled;return false;}
    Sample d{int16_t(a.x-old.x),int16_t(a.y-old.y),int16_t(a.z-old.z)};
    uint32_t power=uint32_t(dot(d,d));if(power>peak2)peak2=power;
    if(power<uint32_t(kAmplitude*kAmplitude))return false;
    if(!havePeak){previous=d;havePeak=true;lastReverse=now;return false;}
    if(dot(d,previous)<-kAmplitude*kAmplitude && uint32_t(now-lastReverse)>=kMinReverseMs){
      previous=d;lastReverse=now;++reversals;
      if(reversals>=kReversals){confirmed=true;return true;}
    }
    return false;
  }
};
struct Gate {
  bool cooling=false,quietRequired=false;
  uint32_t confirmedAt=0,lastMotion=0;
  constexpr bool cooldown(uint32_t now) const {return cooling && uint32_t(now-confirmedAt)<kCooldownMs;}
  constexpr bool allow(uint32_t now) const {
    return !cooldown(now) && (!quietRequired || uint32_t(now-lastMotion)>=kQuietMs);
  }
  constexpr void motion(uint32_t now){lastMotion=now;}
  constexpr void success(uint32_t now){cooling=quietRequired=true;confirmedAt=lastMotion=now;}
};
}
