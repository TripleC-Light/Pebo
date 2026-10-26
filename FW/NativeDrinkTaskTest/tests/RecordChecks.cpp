#include "../src/RecordChecks.h"
#include "../src/AppParser.h"
static_assert(Records::checks(),"record semantics, bounds, isolation, saturation");
static_assert(Records::calendarChecks(),"calendar and context");
static_assert(Records::contextChecks(),"pending edit cancellation, no backfill, summary eviction");


constexpr bool parserRecordChecks(){
 AppParser p;AppCommand c{EventLoop::Kind::None,0};
 for(char b: "records list 31\n")if(b)c=p.push(b);
 if(c.extra!=Extra::RecordsList||c.value!=31)return false;
 for(char b:"records list 32\n")if(b)c=p.push(b);
 if(c.extra!=Extra::None||c.kind!=EventLoop::Kind::Invalid)return false;
 for(char b:"records clear\n")if(b)c=p.push(b);
 if(c.extra!=Extra::None)return false;
 for(char b:"records clear confirm\r\n")if(b)c=p.push(b);
 if(c.extra!=Extra::RecordsClear)return false;
 for(unsigned i=0;i<50;++i){p.push('x');}c=p.push('\n');
 if(c.kind!=EventLoop::Kind::TooLong)return false;
 p.push('r');p.push(0);c=p.push('\n');if(c.kind!=EventLoop::Kind::Invalid)return false;
 for(char b:"records status")if(b){c=p.push(b);if(c.kind!=EventLoop::Kind::None)return false;}
 return p.push('\n').extra==Extra::RecordsStatus;
}
static_assert(parserRecordChecks(),"record fixed parser bounds / partial / NUL");

int main(){return Records::checks()&&Records::calendarChecks()&&Records::contextChecks()&&parserRecordChecks()?0:1;}
