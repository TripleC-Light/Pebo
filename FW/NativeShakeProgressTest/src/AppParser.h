#pragma once
#include "../../EventLoopStopTest/EventLogic.h"
#include "../../TimeManagementTest/TimeLogic.h"
enum class Extra : uint8_t {None,Progress,Show,Trace,Clock,SetUtc,LedRegs,Observe,LisRegs};
struct AppCommand {
  EventLoop::Kind kind;
  uint8_t seconds;
  Extra extra=Extra::None;
  uint32_t value=0;
};
struct AppParser {
  EventLoop::Parser base;
  constexpr AppCommand push(char c) {
    // Base owns bounds/NUL/CR validation. Its bytes remain available after newline.
    bool discarded=base.discard,nul=base.nul;
    auto result=base.push(c);
    AppCommand out{result.kind,result.seconds};
    if(c!='\n'||discarded||nul||result.kind==EventLoop::Kind::TooLong)return out;
    const char *s=base.line;
    if(TimeTest::equal(s,"show"))out.extra=Extra::Show;
    else if(TimeTest::equal(s,"trace"))out.extra=Extra::Trace;
    else if(TimeTest::equal(s,"clock"))out.extra=Extra::Clock;
    else if(TimeTest::equal(s,"ledregs"))out.extra=Extra::LedRegs;
    else if(TimeTest::equal(s,"observe"))out.extra=Extra::Observe;
    else if(TimeTest::equal(s,"lisregs"))out.extra=Extra::LisRegs;
    else if(TimeTest::prefix(s,"progress ") && s[9]>='0'&&s[9]<='8'&&!s[10]){
      out.extra=Extra::Progress;out.value=s[9]-'0';
    }else if(TimeTest::prefix(s,"setutc ")&&TimeTest::parseDate(s+7,out.value))out.extra=Extra::SetUtc;
    return out;
  }
};
