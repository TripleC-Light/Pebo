#pragma once
#include "TaskParser.h"
static Tasks::Engine taskEngine;
static Tasks::Monotonic taskTicks;
static Tasks::Capture firstTask;
#if PEBO_TASK_TEST
static Tasks::Engine testTasks;
static bool taskTest=false;
static uint32_t testUtc=0,testSeq=0;
static uint64_t testTicks=0;
#else
static constexpr bool taskTest=false;
#endif
static Tasks::Engine &selectedTasks(){
#if PEBO_TASK_TEST
  if(taskTest)return testTasks;
#endif
  return taskEngine;
}
static const char *taskLabel(){return taskTest?"SYNTHETIC":"REAL";}
static const char *taskReason(Tasks::Reason r){
  switch(r){
    case Tasks::Reason::OnTime:return "on_time";case Tasks::Reason::Catchup:return "catchup";
    case Tasks::Reason::Gap:return "gap";case Tasks::Reason::NoTask:return "no_task";
    case Tasks::Reason::Disabled:return "disabled";case Tasks::Reason::InvalidTime:return "invalid_time";
    case Tasks::Reason::Cutoff:return "cutoff";case Tasks::Reason::Context:return "context";
    case Tasks::Reason::Stale:return "stale_activation";case Tasks::Reason::Duplicate:return "duplicate";
    case Tasks::Reason::Range:return "day_not_retained";
  }return "unknown";
}
static void taskRefresh(){
  taskTicks.update(AppTime::now());
  taskEngine.observe(wallStamp(),realRecords.context,realRecords.zone);
  if(!taskTest)Progress::setProgress(taskEngine.progress());
}
static bool taskIdle(){
  return runState.mode==Mode::Debug&&!Observation::active&&!Observation::merge.pending&&
    !Observation::dumping&&!Progress::active()&&!pageActive&&
    !sources[Rtc].enabled&&!sources[Motion].enabled&&!sources[Gpo].enabled;
}
static void taskResult(const Records::Event &e,Tasks::Outcome o){
  char line[160];
  Text::format(line,sizeof(line),"TASK_RESULT %s seq=%lu gen=%lu reason=%s occurred=%d task=%d level=%u\r\n",
    taskLabel(),(unsigned long)e.seq,(unsigned long)selectedTasks().generation,
    taskReason(o.reason),int(o.occurred),int(o.task),unsigned(selectedTasks().progress()));queue(line);
}
static void taskStatus(){
  auto &t=selectedTasks();char line[224];
  Text::format(line,sizeof(line),"TASKS %s day=%lu ctx=%lu version=%lu target=%u done=%u level=%u paused=%u valid=%u liveDay=%lu gen=%lu lastSeq=%lu reason=%s RAM_ONLY\r\n",
    taskLabel(),(unsigned long)t.today.date,(unsigned long)t.today.context,(unsigned long)t.today.plan.version,
    unsigned(t.today.target),unsigned(t.today.completed()),unsigned(t.progress()),unsigned(t.paused),unsigned(t.valid),
    (unsigned long)t.date,(unsigned long)t.generation,(unsigned long)t.lastEvent,taskReason(t.last.reason));queue(line);
  Text::format(line,sizeof(line),"TASK_PREV %s day=%lu ctx=%lu version=%lu target=%u done=%u\r\n",taskLabel(),
    (unsigned long)t.previous.date,(unsigned long)t.previous.context,(unsigned long)t.previous.plan.version,
    unsigned(t.previous.target),unsigned(t.previous.completed()));queue(line);
}
static void scheduleStatus(){
  auto &t=selectedTasks();char line[144];
  const Tasks::Schedule *plans[]={&t.active,&t.pending,&t.draft};
  for(uint8_t i=0;i<3;++i){const auto &s=*plans[i];
    Text::format(line,sizeof(line),"SCHEDULE %s bank=%u version=%lu count=%u days=%u cutoff=%u gap=%lu editing=%u\r\n",taskLabel(),unsigned(i),
      (unsigned long)s.version,unsigned(s.count),unsigned(s.days),unsigned(s.cutoff),(unsigned long)s.gap,unsigned(t.editing));queue(line);}
}
static bool taskCommand(Tasks::Input in){
  using Tasks::Command;auto k=in.kind;if(k==Command::None)return false;
  auto &t=selectedTasks();char line[160];
  if(k==Command::Invalid){queue("ERR task_invalid\r\n");return true;}
  if(k==Command::Status){taskStatus();return true;}
  if(k==Command::ScheduleStatus){scheduleStatus();return true;}
  if(k==Command::List||k==Command::Previous){
    const auto &d=k==Command::List?t.today:t.previous;
    if(in.a>=d.target){queue("ERR task_index\r\n");return true;}
    Text::format(line,sizeof(line),"TASK %s day=%lu ctx=%lu version=%lu index=%u start=%u end=%u state=%u seq=%lu gen=%lu\r\n",
      taskLabel(),(unsigned long)d.date,(unsigned long)d.context,(unsigned long)d.plan.version,unsigned(in.a),
      unsigned(d.plan.slots[in.a].start),unsigned(d.plan.slots[in.a].end),unsigned(d.state(in.a)),
      (unsigned long)d.seq[in.a],(unsigned long)d.gen[in.a]);queue(line);return true;
  }
  if(k==Command::ScheduleList||k==Command::PendingList){
    const auto &s=k==Command::ScheduleList?t.today.plan:t.pending;
    if(in.a>=s.count){queue("ERR schedule_index\r\n");return true;}
    Text::format(line,sizeof(line),"SLOT %s version=%lu index=%u start=%u end=%u\r\n",taskLabel(),
      (unsigned long)s.version,unsigned(in.a),unsigned(s.slots[in.a].start),unsigned(s.slots[in.a].end));queue(line);return true;
  }
  if(!taskIdle()){queue("ERR tasks_require_off_idle\r\n");return true;}
#if PEBO_TASK_TEST
  if(k==Command::TestEnter){
    if(taskTest){queue("ERR SYNTHETIC already_active\r\n");return true;}
    testTasks={};testUtc=TimeTest::epoch({2024,2,28,0,0,0});testTicks=0;testSeq=0;
    testTasks.observe({testUtc,true},77,480);taskTest=true;queue("TEST_BEGIN SYNTHETIC tasks RTC_unchanged\r\n");return true;
  }
  if(k==Command::TestExit){
    taskTest=false;testTasks={};Progress::stopProgressDisplay();taskRefresh();queue("TEST_END SYNTHETIC REAL_retained\r\n");return true;
  }
  if(k==Command::TestStep||k==Command::TestAdd){
    if(!taskTest){queue("ERR SYNTHETIC not_active\r\n");return true;}
    uint32_t step=k==Command::TestStep?in.a:10;
    if(testUtc>TimeTest::kMaxUtc-step||testSeq==UINT32_MAX){queue("ERR SYNTHETIC exhausted\r\n");return true;}
    auto capture=t.capture({testUtc,true},testTicks);uint32_t first=testUtc;
    testUtc+=step;testTicks+=uint64_t(step)*1000;t.observe({testUtc,true},77,480);
    if(k==Command::TestAdd){Records::Event e{++testSeq,77,first,first,testUtc,1,TimeTest::localEpoch(first,480)/86400,480,3};taskResult(e,t.apply(e,capture));}
    else queue("OK SYNTHETIC step\r\n");return true;
  }
#endif
  bool ok=false;
  if(k==Command::Begin){t.begin();ok=true;}
  else if(k==Command::Add)ok=t.add(in.a,in.b);
  else if(k==Command::Days||k==Command::Cutoff||k==Command::Gap){
    if(t.editing){if(k==Command::Days)t.draft.days=in.a;else if(k==Command::Cutoff)t.draft.cutoff=in.a;else t.draft.gap=in.a;ok=true;}
  }else if(k==Command::Commit||k==Command::Today){ok=t.commit(k==Command::Today);}
  else if(k==Command::Restart||k==Command::Clear)ok=t.restart();
  if(ok){Text::format(line,sizeof(line),"OK %s tasks action=%u effect=%s\r\n",taskLabel(),unsigned(k),
    k==Command::Commit?"next_natural_day":k==Command::Today?"today":"RAM");queue(line);}
  else {Text::format(line,sizeof(line),"ERR %s task_setting\r\n",taskLabel());queue(line);}
  if(!taskTest)Progress::setProgress(taskEngine.progress());return true;
}
