#pragma once
#include "DrinkRecords.h"
namespace Tasks {
constexpr uint8_t Capacity=16;
struct Slot {uint16_t start=0,end=0;};
struct Schedule {
  Slot slots[Capacity]{};uint32_t version=0,gap=900;
  uint16_t cutoff=0;uint8_t days=0,count=0;
  constexpr bool valid()const{
    if(!count||count>Capacity||!days||days>127||gap>86400||cutoff>1440)return false;
    for(uint8_t i=0;i<count;++i)if(slots[i].start>=slots[i].end||slots[i].end>1440||
       (i&&slots[i-1].end>slots[i].start))return false;
    return cutoff>=slots[count-1].end;
  }
};
enum class Reason:uint8_t {OnTime,Catchup,Gap,NoTask,Disabled,InvalidTime,Cutoff,Context,Stale,Duplicate,Range};
enum class State:uint8_t {Future,Active,Missed,OnTime,Catchup,Expired};
constexpr uint8_t level(uint8_t done,uint8_t target){
  if(!target)return 0;
  if(done>=target)return 8;
  uint8_t n=(uint16_t(done)*8+target/2)/target;return n>7?7:n;
}
struct Capture {uint64_t elapsed=0;uint32_t serial=0,generation=0;};
struct Monotonic {
  uint64_t value=0;uint32_t last=0;bool seeded=false;
  constexpr void update(uint32_t now){if(seeded)value+=uint32_t(now-last);last=now;seeded=true;}
};
struct Day {
  Schedule plan{};uint32_t seq[Capacity]{},gen[Capacity]{};
  uint32_t date=Records::UnknownDay,context=0,serial=0,asOf=0;
  uint16_t onTime=0,catchup=0;int16_t zone=480;uint8_t target=0;
  constexpr uint8_t completed()const{uint8_t n=0;for(uint8_t i=0;i<target;++i)if((onTime|catchup)&(1u<<i))++n;return n;}
  constexpr int slot(uint32_t sec)const{for(uint8_t i=0;i<plan.count;++i)if(sec>=uint32_t(plan.slots[i].start)*60&&sec<uint32_t(plan.slots[i].end)*60)return i;return -1;}
  constexpr State state(uint8_t i)const{
    if(onTime&(1u<<i))return State::OnTime;
    if(catchup&(1u<<i))return State::Catchup;
    if(asOf>=uint32_t(plan.cutoff)*60)return State::Expired;
    if(asOf<uint32_t(plan.slots[i].start)*60)return State::Future;
    return asOf<uint32_t(plan.slots[i].end)*60?State::Active:State::Missed;
  }
};
struct Outcome {Reason reason=Reason::Disabled;int8_t occurred=-1,task=-1;};
struct Engine {
  Schedule active{},pending{},draft{};Day today{},previous{};
  uint64_t lastScore=0;uint32_t generation=1,lastSeq=0,version=0,serial=0;
  uint32_t context=0,date=Records::UnknownDay,utc=0;
  int16_t zone=480;bool valid=false,paused=false,scored=false,editing=false;
  Outcome last{};uint32_t lastEvent=0;
  constexpr uint32_t local(uint32_t t)const{return TimeTest::localEpoch(t,zone);}
  constexpr Day *find(uint32_t ctx,uint32_t d){
    if(today.serial&&today.context==ctx&&today.date==d)return &today;
    if(previous.serial&&previous.context==ctx&&previous.date==d)return &previous;
    return nullptr;
  }
  __attribute__((noinline)) constexpr void start(){
    previous=today;today=Day{};
    if(!valid||!active.count||serial==UINT32_MAX){paused=active.count!=0;return;}
    today.plan=active;today.date=date;today.context=context;today.zone=zone;today.serial=++serial;
    today.asOf=local(utc)%86400;
    today.target=(active.days&(1u<<((date+3)%7)))?active.count:0;paused=false;
  }
  __attribute__((noinline)) constexpr void observe(Records::Stamp stamp,uint32_t ctx,int16_t offset){
    uint32_t d=stamp.valid?TimeTest::localEpoch(stamp.value,offset)/86400:Records::UnknownDay;
    bool edit=context&&(ctx!=context||stamp.valid!=valid||offset!=zone||
      (stamp.valid&&valid&&d!=date&&d!=date+1));
    bool next=!edit&&valid&&stamp.valid&&ctx==context&&d==date+1;
    if(edit&&today.serial)paused=true;
    context=ctx;valid=stamp.valid;zone=offset;date=d;utc=stamp.value;
    if(next){if(pending.count){active=pending;pending=Schedule{};}if(today.serial)today.asOf=86400;start();}
    else if(!paused&&today.serial&&today.context==context&&today.date==date)today.asOf=local(utc)%86400;
  }
  constexpr void begin(){draft=Schedule{};editing=true;}
  constexpr bool add(uint16_t a,uint16_t b){
    if(!editing||draft.count==Capacity||a>=b||b>1440)return false;
    draft.slots[draft.count++]={a,b};return true;
  }
  __attribute__((noinline)) constexpr bool commit(bool immediate){
    if(!editing||!draft.valid()||version==UINT32_MAX||
       (immediate&&(!valid||active.count||pending.count||serial==UINT32_MAX)))return false;
    draft.version=version+1;
    if(immediate){active=draft;++version;start();}else {pending=draft;++version;}
    editing=false;return true;
  }
  __attribute__((noinline)) constexpr bool restart(){if(!valid||!active.count||serial==UINT32_MAX)return false;start();return true;}
  constexpr bool clear(){return restart();} // Explicitly reset tasks, retain schedules/gap/dedupe.
  constexpr bool recordsCleared(){if(generation==UINT32_MAX)return false;++generation;lastSeq=0;return true;}
  constexpr Capture capture(Records::Stamp first,uint64_t ticks){
    auto *d=first.valid?find(context,local(first.value)/86400):nullptr;
    return {ticks,d?d->serial:0,generation};
  }
  constexpr uint8_t progress()const{
    return valid&&!paused&&today.context==context&&today.date==date?level(today.completed(),today.target):0;
  }
  __attribute__((noinline)) constexpr Outcome apply(const Records::Event &e,Capture c){
    Outcome out{};
    if(c.generation!=generation){out.reason=Reason::Context;return out;}
    if(!e.seq||e.seq<=lastSeq){out.reason=Reason::Duplicate;return out;}
    lastSeq=e.seq;lastEvent=e.seq;
    if(!(e.flags&1))out.reason=Reason::InvalidTime;
    else if(paused||e.context!=context||e.zone!=zone||e.day!=TimeTest::localEpoch(e.first,e.zone)/86400)out.reason=Reason::Context;
    else {
      auto *d=find(e.context,e.day);
      if(!d)out.reason=active.count?Reason::Range:Reason::Disabled;
      else if(!c.serial||d->serial!=c.serial)out.reason=Reason::Stale;
      else if(!d->target)out.reason=Reason::Disabled;
      else {
        uint32_t sec=TimeTest::localEpoch(e.first,e.zone)%86400;
        out.occurred=int8_t(d->slot(sec));
        if(sec>=uint32_t(d->plan.cutoff)*60)out.reason=Reason::Cutoff;
        else if(scored&&(c.elapsed<lastScore||c.elapsed-lastScore<uint64_t(d->plan.gap)*1000))out.reason=Reason::Gap;
        else {
          uint16_t done=d->onTime|d->catchup;int task=out.occurred;
          bool normal=task>=0&&!(done&(1u<<task));
          if(!normal){task=-1;for(uint8_t i=0;i<d->target;++i)if(!(done&(1u<<i))&&sec>=uint32_t(d->plan.slots[i].end)*60){task=i;break;}}
          if(task<0)out.reason=Reason::NoTask;
          else {
            out.task=int8_t(task);out.reason=normal?Reason::OnTime:Reason::Catchup;
            if(normal)d->onTime|=1u<<task;else d->catchup|=1u<<task;
            d->seq[task]=e.seq;d->gen[task]=generation;lastScore=c.elapsed;scored=true;
          }
        }
      }
    }
    last=out;return out;
  }
};
static_assert(sizeof(Schedule)==76,"fixed sixteen-slot schedule");
static_assert(sizeof(Engine)<=800,"bounded task RAM");
}
