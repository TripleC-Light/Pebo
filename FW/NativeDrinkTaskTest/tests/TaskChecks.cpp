#include "../src/TaskLogic.h"
#include <initializer_list>
namespace TaskChecks {
constexpr uint32_t base=TimeTest::epoch({2024,2,28,0,0,0});
constexpr Tasks::Engine setup(unsigned n=4){
  Tasks::Engine t;t.observe({base,true},1,0);t.begin();t.draft.days=127;t.draft.cutoff=1440;
  for(unsigned i=0;i<n;++i)t.add(480+i*60,510+i*60);
  t.commit(true);return t;
}
constexpr Tasks::Outcome event(Tasks::Engine &t,uint32_t at,uint64_t elapsed,uint32_t seq,uint32_t confirm=10){
  t.observe({base+at,true},t.context,t.zone);auto c=t.capture({base+at,true},elapsed);
  Records::Event e{seq,t.context,base+at,base+at,base+at+confirm,1,(base+at)/86400,t.zone,1};
  t.observe({base+at+confirm,true},t.context,t.zone);return t.apply(e,c);
}
constexpr bool scoring(){
  auto t=setup();
  if(event(t,8*3600,0,1).reason!=Tasks::Reason::OnTime)return false;
  if(event(t,8*3600+100,100000,2).reason!=Tasks::Reason::Gap)return false;
  if(event(t,8*3600+900,900000,3).reason!=Tasks::Reason::NoTask)return false;
  if(event(t,9*3600,3600000,4).task!=1)return false;
  if(t.today.completed()!=2||t.progress()!=4)return false;
  auto d=setup();d.active.gap=d.today.plan.gap=0;
  if(event(d,9*3600,0,1).task!=1)return false;
  auto catchup=event(d,9*3600+1,1000,2);
  if(catchup.task!=0||catchup.occurred!=1||catchup.reason!=Tasks::Reason::Catchup)return false;
  if(event(d,9*3600+2,2000,3).reason!=Tasks::Reason::NoTask)return false;
  return d.today.state(0)==Tasks::State::Catchup&&d.today.seq[0]==2;
}
constexpr bool progress(){
  for(unsigned n:{4u,6u,10u,16u}){
    auto t=setup(n);
    for(unsigned i=0;i<n;++i){if(event(t,(8+i)*3600,uint64_t(i)*3600000,i+1).reason!=Tasks::Reason::OnTime)return false;
      uint8_t done=i+1,expected=done==n?8:((done*8+n/2)/n>7?7:(done*8+n/2)/n);
      if(t.progress()!=expected)return false;}
  }return Tasks::level(0,0)==0&&Tasks::level(15,16)==7;
}
constexpr bool gap(){
  auto t=setup();event(t,8*3600,1000,1);
  if(event(t,9*3600,900999,2).reason!=Tasks::Reason::Gap)return false;
  if(event(t,9*3600+1,901000,3).reason!=Tasks::Reason::OnTime)return false;
  Tasks::Monotonic m;m.update(UINT32_MAX-100);m.update(20);return m.value==121;
}
constexpr bool boundaries(){
  auto t=setup(2);t.today.plan.gap=0;t.today.plan.cutoff=600;
  if(event(t,8*3600-1,0,1).reason!=Tasks::Reason::NoTask)return false;
  if(event(t,8*3600,1000,2).reason!=Tasks::Reason::OnTime)return false;
  if(event(t,9*3600+3598,3600000,3,10).task!=1)return false;
  if(event(t,10*3600,3700000,4).reason!=Tasks::Reason::Cutoff)return false;
  auto x=setup(1);x.today.plan.slots[0]={1430,1440};x.today.plan.gap=0;
  x.observe({base+86398,true},1,0);auto c=x.capture({base+86398,true},10);
  Records::Event e{1,1,base+86398,base+86400,base+86408,2,base/86400,0,1};
  x.observe({base+86408,true},1,0);
  if(x.apply(e,c).reason!=Tasks::Reason::OnTime||x.today.completed()!=0||x.previous.completed()!=1)return false;
  x.observe({base+3*86400,true},2,0);return x.paused&&x.apply(e,c).reason==Tasks::Reason::Duplicate;
}
constexpr bool schedules(){
  auto t=setup(2);auto version=t.active.version;t.begin();t.add(600,700);t.add(650,800);t.draft.cutoff=900;t.draft.days=127;
  if(t.commit(false)||t.active.version!=version||t.pending.count)return false;
  t.begin();for(unsigned i=0;i<3;++i)t.add(600+i*60,630+i*60);t.draft.cutoff=800;t.draft.days=127;
  if(!t.commit(false)||t.today.target!=2||t.pending.count!=3)return false;
  if(t.commit(true))return false;
  t.observe({base+86400,true},1,0);
  if(t.today.target!=3||t.previous.target!=2||t.pending.count)return false;
  t.observe({base+86410,true},2,480);if(!t.paused||t.progress())return false;
  if(!t.restart()||t.paused||t.today.context!=2)return false;
  return t.previous.context==1;
}
constexpr bool isolation(){
  auto a=setup(),b=setup();event(a,8*3600,0,1);
  if(b.today.completed()||event(a,9*3600,10000000,1).reason!=Tasks::Reason::Duplicate)return false;
  auto c=a.capture({base+36000,true},11000000);a.recordsCleared();
  Records::Event e{1,1,base+36000,base+36000,base+36010,1,base/86400,0,1};
  if(a.apply(e,c).reason!=Tasks::Reason::Context||a.today.completed()!=1)return false;
  if(!a.clear()||a.today.completed()||a.active.count!=4)return false;
  a.generation=UINT32_MAX;if(a.recordsCleared())return false;
  auto t=setup();if(event(t,8*3600,0,UINT32_MAX).reason!=Tasks::Reason::OnTime)return false;
  if(event(t,9*3600,10000000,UINT32_MAX).reason!=Tasks::Reason::Duplicate)return false;
  auto z=setup();z.observe({0,false},2,0);e.flags=0;e.seq=2;return z.apply(e,{0,0,z.generation}).reason==Tasks::Reason::InvalidTime;
}
constexpr bool dates(){
  auto t=setup();t.today.plan.days=t.active.days=1; // Monday only; current Wed remains original day plan target.
  t.observe({base+86400,true},1,0);if(t.today.target)return false;
  t.observe({TimeTest::epoch({2024,3,1,0,0,0}),true},1,0);if(t.date!=TimeTest::epoch({2024,3,1,0,0,0})/86400)return false;
  auto y=setup();y.observe({TimeTest::epoch({2024,12,31,23,59,59}),true},2,0);if(!y.paused)return false;
  y.restart();y.observe({TimeTest::epoch({2025,1,1,0,0,0}),true},2,0);return !y.paused&&y.previous.date+1==y.today.date;
}
constexpr bool all(){return scoring()&&progress()&&gap()&&boundaries()&&schedules()&&isolation()&&dates();}
}
static_assert(TaskChecks::all(),"task scoring/scheduling/date/isolation regressions");
int main(){return TaskChecks::all()?0:1;}
