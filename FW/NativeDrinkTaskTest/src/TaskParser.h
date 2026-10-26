#pragma once
#include "TaskLogic.h"
namespace Tasks {
enum class Command:uint8_t {None,Invalid,Status,List,Previous,Begin,Add,Days,Cutoff,Gap,Commit,Today,ScheduleStatus,ScheduleList,PendingList,Restart,Clear,TestEnter,TestExit,TestStep,TestAdd};
struct Input {Command kind=Command::None;uint32_t a=0,b=0;};
constexpr bool minute(const char *s,uint16_t &out){
  for(uint8_t i=0;i<5;++i)if(i==2?s[i]!=':':s[i]<'0'||s[i]>'9')return false;
  unsigned h=(s[0]-'0')*10+s[1]-'0',m=(s[3]-'0')*10+s[4]-'0';
  if(h>24||m>59||(h==24&&m))return false;
  out=h*60+m;return true;
}
constexpr Input parse(const char *s){
  using TimeTest::equal;using TimeTest::prefix;using TimeTest::number;
  Input c{};uint32_t n=0;uint16_t a=0,b=0;
  if(prefix(s,"schedule ")){
    c.kind=Command::Invalid;s+=9;
    if(equal(s,"status"))c.kind=Command::ScheduleStatus;
    else if(equal(s,"begin"))c.kind=Command::Begin;
    else if(equal(s,"commit"))c.kind=Command::Commit;
    else if(equal(s,"commit today"))c.kind=Command::Today;
    else if(prefix(s,"add ")){s+=4;unsigned len=0;while(s[len])++len;if(len==11&&s[5]==' '&&minute(s,a)&&minute(s+6,b)){c={Command::Add,a,b};}}
    else if(prefix(s,"days ")&&number(s+5,n)&&n>=1&&n<=127)c={Command::Days,n,0};
    else if(prefix(s,"cutoff ")){s+=7;unsigned len=0;while(s[len])++len;if(len==5&&minute(s,a))c={Command::Cutoff,a,0};}
    else if(prefix(s,"gap ")&&number(s+4,n)&&n<=86400)c={Command::Gap,n,0};
    else if(prefix(s,"list ")&&number(s+5,n)&&n<16)c={Command::ScheduleList,n,0};
    else if(prefix(s,"pending ")&&number(s+8,n)&&n<16)c={Command::PendingList,n,0};
  }else if(prefix(s,"tasks ")){
    c.kind=Command::Invalid;s+=6;
    if(equal(s,"status"))c.kind=Command::Status;
    else if(equal(s,"list"))c.kind=Command::List;
    else if(equal(s,"restart confirm"))c.kind=Command::Restart;
    else if(equal(s,"clear confirm"))c.kind=Command::Clear;
    else if(prefix(s,"list ")&&number(s+5,n)&&n<16)c={Command::List,n,0};
    else if(prefix(s,"prev ")&&number(s+5,n)&&n<16)c={Command::Previous,n,0};
#if PEBO_TASK_TEST
    else if(equal(s,"test enter"))c.kind=Command::TestEnter;
    else if(equal(s,"test exit"))c.kind=Command::TestExit;
    else if(equal(s,"test add"))c.kind=Command::TestAdd;
    else if(prefix(s,"test step ")&&number(s+10,n)&&n<=172800)c={Command::TestStep,n,0};
#endif
  }
  return c;
}
}
