#include "../src/AppParser.h"
constexpr AppCommand command(const char *s){AppParser p;while(*s)p.push(*s++);return p.push('\n');}
constexpr bool configCommands(){
 const char *trace[]={"trace","capture 5","capture 30"};
 for(auto s:trace)if((command(s).extra!=Extra::None)!=bool(PEBO_TRACE))return false;
 const char *tests[]={"records test enter","records test add","records test day","records test exit"};
 for(auto s:tests)if((command(s).extra!=Extra::None)!=bool(PEBO_SYNTHETIC))return false;
 const char *dump[]={"ledregs","lisregs","observe"};
 for(auto s:dump)if((command(s).extra!=Extra::None)!=bool(PEBO_DUMPS))return false;
 if((command("probe").kind!=EventLoop::Kind::Invalid)!=bool(PEBO_DUMPS))return false;
 if((command("counters clear").kind!=EventLoop::Kind::Invalid)!=bool(PEBO_VERBOSE))return false;
 return command("calibration start").extra==Extra::CalibrationStart&&command("records list 31").value==31&&
 command("run 60").kind==EventLoop::Kind::Run&&command("off").kind==EventLoop::Kind::Off&&
 command("counters").kind==EventLoop::Kind::Counters&&command("records list 32").kind==EventLoop::Kind::Invalid;
}
static_assert(configCommands(),"profile command exclusion and retained app operations");
