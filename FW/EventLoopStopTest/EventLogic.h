#pragma once
#include <stdint.h>
#include <stddef.h>
namespace EventLoop {
enum Source : uint8_t { Rtc=0, Motion=1, Gpo=2, Count=3 };
constexpr uint32_t kMaxServiceAttempts=128;
constexpr uint8_t sourceBit(Source s) { return uint8_t(1u<<s); }
constexpr uint32_t saturated(uint32_t n) { return n==UINT32_MAX?n:n+1; }
constexpr uint32_t elapsedDay(uint32_t now,uint32_t start) { return (now+86400u-start)%86400u; }
struct MailboxModel {
  uint8_t flags=0; uint32_t observed[Count]={};
  constexpr void record(Source s) { flags|=sourceBit(s); observed[s]=saturated(observed[s]); }
  constexpr uint8_t take() { uint8_t old=flags; flags=0; return old; }
};
struct SourceState {
  bool enabled=false,fault=false,retry=false;
  uint8_t failures=0;
  uint32_t confirmed=0,handled=0,errors=0,unknown=0,attempts=0,next=0;
  constexpr bool due(uint32_t now) const { return retry && int32_t(now-next)>=0; }
  constexpr bool outcome(bool success,bool active,uint32_t now) {
    attempts=saturated(attempts);
    if (success)failures=0;
    else {errors=saturated(errors);++failures;}
    if (failures>=3 || attempts>=kMaxServiceAttempts) { enabled=false; fault=true; retry=false; return false; }
    if (success && !active) { retry=false;return true; }
    // 12.5Hz ODR: allow a complete 80ms sample period before rechecking a latch.
    retry=true; next=now+100; return true;
  }
};
enum class Mode : uint8_t { Debug,Probe,Run };
struct RunState {
  Mode mode=Mode::Debug;
  uint8_t requested=0;
  uint32_t start=0;
  constexpr void begin(uint8_t n,uint32_t now) { requested=n;start=now;mode=Mode::Probe; }
  constexpr void validated(uint32_t now) { start=now;mode=Mode::Run; }
  constexpr bool expired(uint32_t now) const { return mode==Mode::Run && elapsedDay(now,start)>=requested; }
  constexpr void stop() { mode=Mode::Debug; }
};
enum class Kind : uint8_t { None,Help,Status,Probe,Counters,Clear,Debug,Run,Off,Invalid,TooLong };
struct Command { Kind kind=Kind::Invalid; uint8_t seconds=0; };
constexpr bool equal(const char *a,const char *b) {
  while (*a && *a==*b) { ++a;++b; } return *a==*b;
}
struct Parser {
  char line[32]={}; uint8_t used=0; bool discard=false,nul=false;
  constexpr Command push(char c) {
    if (c!='\n') {
      if (!c) nul=true;
      if (used<31 && !discard) line[used++]=c; else discard=true;
      return {Kind::None,0};
    }
    Command out;
    if (discard) out.kind=Kind::TooLong;
    else if (!nul) {
      if (used && line[used-1]=='\r') --used;
      line[used]=0;
      if (equal(line,"help")) out.kind=Kind::Help;
      else if (equal(line,"status")) out.kind=Kind::Status;
      else if (equal(line,"probe")) out.kind=Kind::Probe;
      else if (equal(line,"counters")) out.kind=Kind::Counters;
      else if (equal(line,"counters clear")) out.kind=Kind::Clear;
      else if (equal(line,"mode debug")) out.kind=Kind::Debug;
      else if (equal(line,"off")) out.kind=Kind::Off;
      else if (used>=5 && used<=6 && line[0]=='r' && line[1]=='u' && line[2]=='n' && line[3]==' ') {
        unsigned n=0; bool valid=true;
        for (uint8_t i=4;i<used;++i) { if (line[i]<'0'||line[i]>'9') valid=false; else n=n*10+line[i]-'0'; }
        if (valid && n>=1 && n<=60) out={Kind::Run,uint8_t(n)};
      }
    }
    used=0;discard=nul=false;return out;
  }
};
}
