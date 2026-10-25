#include "../src/ShakeLogic.h"
#include "../../BlueLedProgressTest/ProgressLogic.h"
#include "../src/AppParser.h"
#include "../src/ElapsedLogic.h"
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
    bool next=d.feed({int16_t((i/12)%2?800:-800),0,1024},i*10);
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
