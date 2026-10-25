#pragma once
#include <stdint.h>
struct ElapsedClock {
  static constexpr uint32_t period=86400000u;
  uint32_t last=0,elapsed=0,rejected=0;
  bool seeded=false;
  constexpr void accept(uint32_t value){
    if(seeded){
      uint32_t delta=(value+period-last)%period;
      // Reject short backward snapshots instead of converting them into a day.
      // Service must occur within half a day (run wakes every second).
      if(delta>period/2){if(rejected!=UINT32_MAX)++rejected;return;}
      elapsed+=delta;
    }
    last=value;seeded=true;
  }
};
