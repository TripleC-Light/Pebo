#include "../src/ShakeLogic.h"
#include "../../BlueLedProgressTest/ProgressLogic.h"
#include "../src/AppParser.h"
#include "../src/ElapsedLogic.h"
#include "../src/DrinkLogic.h"
constexpr bool calibrationChecks(){
  Drink::Reference ref{{0,0,1024},true};Drink::Calibration c;
  for(unsigned i=0;i<200;++i)c.feed({0,-1024,0});
  if(!c.finish(ref)||ref.value.y!=-1024)return false;
  Drink::Calibration moving;
  for(unsigned i=0;i<200;++i)moving.feed({int16_t(i%2?200:-200),-1024,0});
  if(moving.finish(ref)||ref.value.y!=-1024)return false;
  Drink::Calibration freefall;
  for(unsigned i=0;i<200;++i)freefall.feed({0,0,0});
  return !freefall.finish(ref)&&ref.valid;
}
constexpr bool mergeChecks(){
  Drink::Merge m;m.add(100,1000);
  if(m.due(10999)||!m.due(11000)||m.add(10000,11000))return false;
  if(!m.confirm(11000)||m.candidates!=1||m.first!=100||m.confirmedAt!=11000)return false;
  if(!m.add(11000,11000)||!m.add(19000,20999)||m.candidates!=2)return false;
  if(m.due(30998)||!m.due(30999))return false;
  m.cancel();if(m.pending||m.cancelled!=1||m.confirm(40000))return false;
  m.add(UINT32_MAX-100,UINT32_MAX-10);
  return !m.due(9988)&&m.due(9989);
}
constexpr bool drinkChecks(){
  Drink::Reference ref{{0,0,1024},true};Drink::Detector d;d.begin(ref,0);
  bool hit=false;
  for(unsigned t=0;t<6000;t+=10){
    auto a=t<1000||t>=4000?Shake::Sample{0,0,1024}:Shake::Sample{1024,0,0};
    if(d.feed(a,t)){if(hit)return false;hit=true;}
  }
  if(!hit)return false;
  d.begin(ref,0);
  for(unsigned t=0;t<4000;t+=10){
    auto a=t<1000||t>=1400?Shake::Sample{0,0,1024}:Shake::Sample{1024,0,0};
    if(d.feed(a,t))return false;
  }
  d.begin(ref,0);d.shake(0);
  for(unsigned t=0;t<2500;t+=10)if(d.feed({1024,0,0},t))return false;
  for(unsigned t=2500;t<4000;t+=10)if(d.feed({0,0,1024},t))return false;
  if(d.phase!=Drink::Phase::Idle)return false;
  hit=false;
  for(unsigned t=4000;t<9000;t+=10)if(d.feed(t<7000?Shake::Sample{1024,0,0}:Shake::Sample{0,0,1024},t))hit=true;
  if(!hit)return false;
  d.begin(ref,0);
  for(unsigned t=0;t<14500;t+=10)if(d.feed({1024,0,0},t))return false;
  if(!d.timeouts)return false;
  d.abort();if(d.phase!=Drink::Phase::Idle)return false;
  d.begin({},0);for(unsigned t=0;t<3000;t+=10)if(d.feed({1024,0,0},t))return false;
  return Drink::aboveTilt({1024,0,0},ref.value,Drink::kEnterCos2)&&
    !Drink::aboveTilt({0,0,1024},ref.value,Drink::kEnterCos2)&&
    Drink::upright({0,0,1024},ref.value)&&!Drink::upright({0,0,-1024},ref.value);
}
static_assert(calibrationChecks(),"Stationary calibration, magnitude/motion reject and preservation");
static_assert(mergeChecks(),"Before/equal/after merge, cancellation and uint32 rollover");
static_assert(drinkChecks(),"Hold/return hysteresis, short tilt, shake exclusion/rearm, timeout, invalid calibration");
constexpr bool incompleteChecks(){
  Drink::Detector d;d.begin({{0,0,1024},true},UINT32_MAX-2000);
  for(uint32_t t=0;t<2000;t+=10)d.feed({1024,0,0},uint32_t(UINT32_MAX-2000+t));
  if(d.phase!=Drink::Phase::Held)return false;
  if(d.feed({0,0,1024},200)||d.phase==Drink::Phase::Held)return false;
  d.abort();if(d.phase!=Drink::Phase::Idle)return false;
  Drink::Merge m;m.add(100,1000);m.cancel();if(m.confirm(11000))return false;
  AppParser p;AppCommand c{EventLoop::Kind::None,0};
  for(char v:"capture 15\n")if(v)c=p.push(v);
#if PEBO_TRACE
  if(c.extra!=Extra::Capture||c.value!=15)return false;
#else
  if(c.extra!=Extra::None||c.kind!=EventLoop::Kind::Invalid)return false;
#endif
  for(char v:"capture 31\n")if(v)c=p.push(v);
  if(c.extra!=Extra::None)return false;
  for(char v:"calibration start\n")if(v)c=p.push(v);
  return c.extra==Extra::CalibrationStart;
}
static_assert(incompleteChecks(),"Missing samples cannot bridge hold/return; cancellation and new command bounds");
constexpr bool interruptedReturnChecks(){
  Drink::Detector d;d.begin({{0,0,1024},true},0);
  for(unsigned t=0;t<2000;t+=10)d.feed({1024,0,0},t);
  for(unsigned t=2000;t<2500;t+=10)if(d.feed({0,0,1024},t))return false;
  if(d.phase!=Drink::Phase::Return)return false;
  if(d.feed({2047,2047,2047},2500)||d.phase!=Drink::Phase::Held)return false;
  // A reliable return must establish a new stable interval after the shock.
  for(unsigned t=2510;t<2800;t+=10)if(d.feed({0,0,1024},t))return false;
  bool hit=false;for(unsigned t=2800;t<3400;t+=10)if(d.feed({0,0,1024},t))hit=true;
  return hit;
}
static_assert(interruptedReturnChecks(),"Unreliable dynamics cannot count toward stable return duration");
constexpr bool elapsedChecks(){
  ElapsedClock c;c.accept(86399900);c.accept(100);if(c.elapsed!=200)return false;
  c.accept(99);if(c.elapsed!=200||c.rejected!=1)return false;
  c.accept(100);if(c.elapsed!=200)return false;
  c.accept(60100);if(c.elapsed!=60200)return false;
  c.elapsed=UINT32_MAX-50;c.accept(60200);if(c.elapsed!=49)return false;
  c.seeded=false;c.accept(123456);return c.elapsed==49;
}
static_assert(elapsedChecks(),"Midnight, backward RTC glitch, STOP, uint32 wrap and adjustment rebase");
constexpr bool parserChecks(){
  AppParser p;AppCommand c{EventLoop::Kind::None,0};
  for(char ch:"progress 8\r\n"){if(ch)c=p.push(ch);}
  if(c.extra!=Extra::Progress||c.value!=8)return false;
  for(char ch:"progress 9\n"){if(ch)c=p.push(ch);}
  if(c.extra!=Extra::None||c.kind!=EventLoop::Kind::Invalid)return false;
  p.push('s');p.push('h');p.push(0);p.push('o');p.push('w');c=p.push('\n');
  if(c.extra!=Extra::None||c.kind!=EventLoop::Kind::Invalid)return false;
  for(unsigned i=0;i<80;++i)p.push('x');
  if(p.push('\n').kind!=EventLoop::Kind::TooLong)return false;
  for(char ch:"setutc 2024-02-29 23:59:59\n"){if(ch)c=p.push(ch);}
  if(c.extra!=Extra::SetUtc)return false;
  for(char ch:"setutc 2023-02-29 23:59:59\n"){if(ch)c=p.push(ch);}
  return c.extra==Extra::None&&c.kind==EventLoop::Kind::Invalid;
}
static_assert(parserChecks(),"Input bounds, CRLF, NUL, calendar and parser recovery");
constexpr bool displayChecks(){
  BlueProgress::Display d;
  const unsigned expected[]={0,1,2,6,10,26,42,106,170};
  for(unsigned n=0;n<9;++n)if(BlueProgress::patternFor(n)!=expected[n])return false;
  d.show(5,UINT32_MAX-100);d.update(2898);if(!d.pattern)return false;
  d.update(2899);if(d.pattern)return false;
  d.show(8,0);d.show(3,2000);d.update(3001);if(d.pattern!=6)return false;
  if(d.show(9,0)||d.level!=3)return false;
  d.off();if(d.level!=3||d.pattern)return false;
  d.show(0,0);d.update(120);if(d.pattern)return false;
  d.update(240);if(d.pattern!=1)return false;
  d.update(360);return d.pattern==0;
}
constexpr bool detectorChecks(){
  Shake::Detector d;d.begin(UINT32_MAX-100);
  if(d.expired(1398)||!d.expired(1399))return false;
  d.begin(0);
  for(unsigned i=0;i<140;++i)if(d.feed({0,0,1024},i*10))return false;
  d.begin(0);
  for(unsigned i=0;i<140;++i)if(d.feed({int16_t(i*3),0,1024},i*10))return false;
  d.begin(0);bool hit=false;
  for(unsigned i=0;i<140;++i){
    int16_t x=i<40?int16_t((i/8)%2?900:-900):0;
    if(d.feed({x,0,1024},i*10))return false;
  }
  d.begin(0);
  for(unsigned i=0;i<140;++i){
    bool next=d.feed({int16_t((i/12)%2?900:-900),0,1024},i*10);
    if(next){if(hit)return false;hit=true;}
  }
  return hit && d.confirmed && !d.feed({0,0,0},1490);
}
constexpr bool gateChecks(){
  Shake::Gate g;g.success(UINT32_MAX-1000);
  if(g.allow(998))return false;
  g.motion(999);if(g.allow(1200))return false;
  return g.allow(1500);
}
static_assert(displayChecks(),"LED boundary/restart/overflow");
static_assert(detectorChecks(),"repetition/tilt/timeout/single result");
static_assert(gateChecks(),"STOP-compatible external monotonic cooldown/quiet");
