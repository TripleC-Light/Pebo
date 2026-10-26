#pragma once
#include <stdint.h>
#include "../../TimeManagementTest/TimeLogic.h"
namespace Records {
constexpr uint8_t Capacity=32, SummaryCapacity=4;
constexpr uint32_t UnknownDay=UINT32_MAX;
constexpr uint32_t sat(uint32_t a,uint32_t b=1){return UINT32_MAX-a<b?UINT32_MAX:a+b;}
struct Stamp {uint32_t value=0; bool valid=false;};
// UTC seconds when valid, otherwise boot elapsed milliseconds. Never backfill.
struct Event {
  uint32_t seq=0,context=0,first=0,last=0,confirmed=0,candidates=0,day=0;
  int16_t zone=0; uint16_t flags=0; // bit0 UTC valid; bit1 SYNTHETIC
};
struct Summary {
  uint32_t context=0,day=0,events=0,candidates=0,last=0;
  int16_t zone=0; uint16_t reserved=0;
};
struct Store {
  Event records[Capacity]{}; Summary days[SummaryCapacity]{};
  uint32_t context=0,day=0,lastSeq=0,overwritten=0,undated=0,summaryLost=0,rejected=0;
  int16_t zone=0; uint8_t head=0,count=0,nextDay=0; bool valid=false,exhausted=false;
  __attribute__((noinline)) constexpr void initialize(){*this=Store{};context=1;day=UnknownDay;zone=480;}
  constexpr const Summary *find(uint32_t c,uint32_t d)const{
    for(const auto &s:days)if(s.context==c&&s.day==d)return &s;
    return nullptr;
  }
  constexpr Summary *find(uint32_t c,uint32_t d){
    for(auto &s:days)if(s.context==c&&s.day==d)return &s;
    return nullptr;
  }
  constexpr void ensure(){
    if(!valid||find(context,day))return;
    days[nextDay]={context,day,0,0,0,zone,0};nextDay=(nextDay+1)%SummaryCapacity;
  }
  __attribute__((noinline)) constexpr bool change(Stamp now,int16_t offset){
    if(offset < -840 || offset > 840)return false;
    if(context==UINT32_MAX){exhausted=true;return false;}
    ++context;zone=offset;valid=now.valid;day=valid?TimeTest::localEpoch(now.value,zone)/86400:UnknownDay;
    ensure();return true;
  }
  // Caller explicitly signals clock edits. observe handles validity and natural dates.
  __attribute__((noinline)) constexpr bool observe(Stamp now){
    if(now.valid!=valid)return change(now,zone);
    uint32_t next=valid?TimeTest::localEpoch(now.value,zone)/86400:UnknownDay;
    if(valid&&day!=UnknownDay&&next!=day&&next!=day+1)return change(now,zone);
    if(next!=day){day=next;ensure();}
    return false;
  }
  constexpr bool append(Event e){
    if(exhausted||!e.seq||e.seq<=lastSeq||e.context!=context||!e.candidates||bool(e.flags&1)!=valid){rejected=sat(rejected);return false;}
    e.zone=zone;
    e.day=(e.flags&1)?TimeTest::localEpoch(e.first,zone)/86400:UnknownDay;
    // Do not synthesize an evicted day's summary. The finite event still survives.
    if(e.flags&1){
      if(auto *s=find(e.context,e.day)){
        s->events=sat(s->events);s->candidates=sat(s->candidates,e.candidates);s->last=e.first;
      }else summaryLost=sat(summaryLost);
    }else undated=sat(undated);
    records[head]=e;head=(head+1)%Capacity;
    if(count==Capacity)overwritten=sat(overwritten);else ++count;
    lastSeq=e.seq;return true;
  }
  constexpr const Event *at(uint8_t oldestIndex)const{
    return oldestIndex<count?&records[(head+Capacity-count+oldestIndex)%Capacity]:nullptr;
  }
  __attribute__((noinline)) constexpr void clear(){
    // Context/time retained; event sequence restarts, explicitly advertised by UART.
    uint32_t c=context,d=day;int16_t z=zone;bool v=valid,x=exhausted;
    *this=Store{};context=c;day=d;zone=z;valid=v;exhausted=x;ensure();
  }
};
struct Pending {
  Stamp first{},last{};uint32_t count=0,seq=0,context=0;bool active=false;
  __attribute__((noinline)) constexpr bool candidate(Stamp stamp,Store &store){
    if(store.exhausted||store.lastSeq==UINT32_MAX)return false;
    if(active&&context!=store.context){cancel();return false;}
    if(!active){active=true;first=stamp;count=0;seq=store.lastSeq+1;context=store.context;}
    last=stamp;count=sat(count);return true;
  }
  constexpr void cancel(){active=false;count=0;}
  __attribute__((noinline)) constexpr bool confirm(Store &store,Stamp output,bool synthetic=false){
    if(!active)return false;
    Event e{seq,context,first.value,last.value,output.value,count,UnknownDay,store.zone,
      uint16_t((first.valid&&last.valid&&output.valid?1:0)|(synthetic?2:0))};
    cancel();return store.append(e);
  }
};
static_assert(sizeof(Event)==32,"aligned 32-byte records, no packing");
static_assert(sizeof(Summary)==24,"aligned summary");
static_assert(sizeof(Store)<=1176,"RAM budget");
}
