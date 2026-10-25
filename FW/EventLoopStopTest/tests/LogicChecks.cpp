#include "../EventLogic.h"
using namespace EventLoop;
constexpr bool mailChecks() {
  MailboxModel m;
  m.record(Rtc);m.record(Motion);m.record(Gpo);m.record(Motion);
  uint8_t snapshot=m.take();
  if (snapshot!=7 || m.flags || m.observed[Motion]!=2) return false;
  // ISR arriving after the protected take must survive processing of old snapshot.
  m.record(Motion);m.record(Gpo);
  uint8_t dispatched=0;
  for (uint8_t i=0;i<Count;++i) if (snapshot&sourceBit(Source(i))) dispatched|=sourceBit(Source(i));
  if (dispatched!=7 || m.flags!=6 || m.take()!=6) return false;
  m.observed[Motion]=UINT32_MAX; m.record(Motion);
  if(m.observed[Motion]!=UINT32_MAX || saturated(UINT32_MAX-1)!=UINT32_MAX)return false;
  for(uint8_t old=1;old<8;++old)for(uint8_t arrivals=1;arrivals<8;++arrivals){
    MailboxModel trace;
    for(uint8_t i=0;i<Count;++i)if(old&sourceBit(Source(i)))trace.record(Source(i));
    uint8_t batch=trace.take();
    for(uint8_t i=0;i<Count;++i)if(arrivals&sourceBit(Source(i)))trace.record(Source(i));
    uint8_t handled=0;
    for(uint8_t i=0;i<Count;++i)if(batch&sourceBit(Source(i)))handled|=sourceBit(Source(i));
    if(handled!=old || trace.flags!=arrivals)return false;
  }
  return true;
}
constexpr bool policyChecks() {
  SourceState s; s.enabled=true;
  if (!s.outcome(false,false,UINT32_MAX-9) || s.due(89) || !s.due(90)) return false;
  if (!s.outcome(false,true,90) || s.outcome(false,true,190) || !s.fault || s.enabled) return false;
  s={};s.enabled=true;s.outcome(false,true,1);s.outcome(true,false,21);
  if (s.failures || s.retry || s.fault) return false;
  s={};s.enabled=true;
  for(unsigned i=0;i<20;++i)if(!s.outcome(true,true,100*i)||s.errors)return false;
  if(!s.outcome(true,false,2100)||s.retry)return false;
  s.attempts=kMaxServiceAttempts-1;if(s.outcome(true,true,2200)||!s.fault)return false;
  RunState r;r.begin(60,86399);if (r.expired(0)) return false;
  r.validated(86399);if (r.expired(58)||!r.expired(59)) return false;
  r.stop();r.begin(1,10);r.validated(11);
  if(r.expired(11)||!r.expired(12))return false;
  for(uint8_t n=1;n<=60;++n){
    r.begin(n,86399);if(r.expired((86399+n)%86400))return false;
    r.validated(86399);if(r.expired((86399+n-1)%86400)||!r.expired((86399+n)%86400))return false;
    r.stop();if(r.expired(100))return false;
  }
  return true;
}
constexpr Command feed(Parser &p,const char *s) { Command c;for (;*s;++s)c=p.push(*s);return c; }
constexpr bool parseChecks() {
  Parser p;
  if (feed(p,"ru").kind!=Kind::None || feed(p,"n 60\r\n").seconds!=60) return false;
  if (feed(p,"run 0\n").kind!=Kind::Invalid || feed(p,"run 61\n").kind!=Kind::Invalid || feed(p,"run -1\n").kind!=Kind::Invalid) return false;
  for (unsigned i=0;i<40;++i)p.push('x');
  if (feed(p,"off\n").kind!=Kind::TooLong || feed(p,"off\n").kind!=Kind::Off) return false;
  feed(p,"off");p.push('\0');if(p.push('\n').kind!=Kind::Invalid)return false;
  return feed(p,"counters clear\n").kind==Kind::Clear;
}
static_assert(mailChecks(),"Multi-source coalescing, dispatch, new arrival and saturation");
static_assert(policyChecks(),"Bounded retries, wrap-safe retry and RTC day boundary");
static_assert(parseChecks(),"Partial command, range, overflow/NUL and recovery");
