#pragma once
#include "../../EventLoopStopTest/EventLogic.h"
#include "../../TimeManagementTest/TimeLogic.h"
enum class Extra : uint8_t {None,Progress,Show,Trace,Clock,SetUtc,LedRegs,Observe,LisRegs,CalibrationStart,CalibrationStatus,DrinkStatus,DrinkCounters,Capture,RecordsStatus,RecordsList,RecordsToday,RecordsClear,TestEnter,TestExit,TestAdd,TestDay};
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
    if(TimeTest::equal(s,"records status"))out.extra=Extra::RecordsStatus;
    else if(TimeTest::equal(s,"records today"))out.extra=Extra::RecordsToday;
    else if(TimeTest::equal(s,"records clear confirm"))out.extra=Extra::RecordsClear;
    else if(TimeTest::equal(s,"records test enter"))out.extra=Extra::TestEnter;
    else if(TimeTest::equal(s,"records test exit"))out.extra=Extra::TestExit;
    else if(TimeTest::equal(s,"records test add"))out.extra=Extra::TestAdd;
    else if(TimeTest::equal(s,"records test day"))out.extra=Extra::TestDay;
    else if(TimeTest::equal(s,"records list"))out.extra=Extra::RecordsList;
    else if(TimeTest::prefix(s,"records list ")){uint32_t n=0;if(TimeTest::number(s+13,n)&&n<32){out.extra=Extra::RecordsList;out.value=n;}}
    else if(TimeTest::equal(s,"show"))out.extra=Extra::Show;
    else if(TimeTest::equal(s,"calibration start"))out.extra=Extra::CalibrationStart;
    else if(TimeTest::equal(s,"calibration status"))out.extra=Extra::CalibrationStatus;
    else if(TimeTest::equal(s,"drink status"))out.extra=Extra::DrinkStatus;
    else if(TimeTest::equal(s,"drink counters"))out.extra=Extra::DrinkCounters;
    else if(TimeTest::prefix(s,"capture ")){
      uint32_t seconds=0;uint8_t i=8;
      for(;s[i]>='0'&&s[i]<='9'&&i<11;++i)seconds=seconds*10+unsigned(s[i]-'0');
      if(!s[i]&&i>8&&seconds>=5&&seconds<=30){out.extra=Extra::Capture;out.value=seconds;}
    }
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
