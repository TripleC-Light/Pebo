#include "../src/AppParser.h"
#include "../src/TaskParser.h"
constexpr AppCommand command(const char *s){AppParser p;while(*s)p.push(*s++);return p.push('\n');}
constexpr bool configCommands(){
  using Tasks::Command;
  return command("run 60").kind==EventLoop::Kind::Run&&command("off").kind==EventLoop::Kind::Off&&
    command("records list 31").value==31&&command("records list 32").kind==EventLoop::Kind::Invalid&&
    command("capture 5").extra==Extra::None&&Tasks::parse("schedule add 08:00 09:00").b==540&&
    Tasks::parse("schedule add 08:00 08:0").kind==Command::Invalid&&
    Tasks::parse("schedule days 128").kind==Command::Invalid&&
    Tasks::parse("schedule gap 86401").kind==Command::Invalid&&
    Tasks::parse("tasks list 16").kind==Command::Invalid&&
    Tasks::parse("schedule commit today").kind==Command::Today&&
    (Tasks::parse("tasks test enter").kind==Command::TestEnter)==bool(PEBO_TASK_TEST);
}
static_assert(configCommands(),"task UART bounds/profile gates");
