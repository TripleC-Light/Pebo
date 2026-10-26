#pragma once
#include "DrinkRecords.h"
#include <initializer_list>
namespace Records {
constexpr Stamp stamp(uint32_t v){return {v,true};}
constexpr bool checks(){
 Store s;s.initialize();uint32_t t=TimeTest::epoch({2024,2,28,15,59,55});s.observe(stamp(t));
 Pending p;p.candidate(stamp(t),s);p.candidate(stamp(t+2),s);
 s.observe(stamp(t+10));if(!p.confirm(s,stamp(t+12))||p.confirm(s,stamp(t+13)))return false;
 if(s.count!=1||s.at(0)->first!=t||s.at(0)->last!=t+2||s.at(0)->confirmed!=t+12||s.at(0)->candidates!=2)return false;
 if(s.at(0)->day!=TimeTest::localEpoch(t,480)/86400||s.find(s.context,s.day)->events!=0)return false;
 if(s.find(s.context,s.day-1)->events!=1)return false;
 for(unsigned i=0;i<40;++i){p.candidate(stamp(t+20+i),s);if(!p.confirm(s,stamp(t+30+i)))return false;}
 if(s.count!=32||s.overwritten!=9||s.at(0)->seq!=10||s.at(31)->seq!=41||s.find(s.context,s.day)->events!=40)return false;
 auto e=*s.at(31);if(s.append(e))return false;
 uint32_t c=s.context;s.change(stamp(t-86400),-480);
 if(s.context!=c+1||s.at(0)->zone!=480||s.find(s.context,s.day)->events)return false;
 p.candidate(stamp(t),s);p.cancel();if(p.confirm(s,stamp(t)))return false;
 s.change({123,false},480);p.candidate({44,false},s);p.confirm(s,{55,false});
 if(s.undated!=1||s.at(31)->day!=UnknownDay||s.at(31)->flags&1)return false;
 s.observe(stamp(t));if(s.undated!=1||s.at(31)->flags&1)return false;
 Store isolated;isolated.initialize();Pending test;isolated.observe(stamp(t));test.candidate(stamp(t),isolated);test.confirm(isolated,stamp(t+10),true);
 if(s.undated!=1||isolated.at(0)->flags!=3)return false;
 s.clear();if(s.count||s.overwritten||s.undated||s.lastSeq||s.find(s.context,s.day)->events)return false;
 auto *d=s.find(s.context,s.day);d->events=UINT32_MAX;d->candidates=UINT32_MAX-1;
 p.candidate(stamp(t),s);p.candidate(stamp(t),s);p.confirm(s,stamp(t));
 if(d->events!=UINT32_MAX||d->candidates!=UINT32_MAX)return false;
 s.overwritten=UINT32_MAX;for(unsigned i=0;i<33;++i){p.candidate(stamp(t),s);p.confirm(s,stamp(t));}
 if(s.overwritten!=UINT32_MAX)return false;
 s.lastSeq=UINT32_MAX;if(p.candidate(stamp(t),s))return false;
 s.context=UINT32_MAX;if(s.change(stamp(t),480)||!s.exhausted)return false;
 s.initialize();s.observe(stamp(t));s.lastSeq=UINT32_MAX-1;
 p.candidate(stamp(t),s);if(!p.confirm(s,stamp(t))||s.at(0)->seq!=UINT32_MAX||p.candidate(stamp(t),s))return false;
 return sat(UINT32_MAX)==UINT32_MAX&&sizeof(Event)==32&&alignof(Event)==4;
}
constexpr bool contextChecks(){
 Store s;s.initialize();auto t=TimeTest::epoch({2024,12,31,15,59,55});s.observe(stamp(t));
 Pending p;p.candidate(stamp(t),s);auto old=s.context;
 s.change(stamp(t-1000),480);if(p.confirm(s,stamp(t))||s.count||s.context==old)return false;
 p.candidate(stamp(t),s);s.change(stamp(t+1000),-840);if(p.candidate(stamp(t),s)||p.active)return false;
 s.change({100,false},480);p.candidate({100,false},s);p.confirm(s,{200,false});
 auto e=*s.at(0);s.observe(stamp(t));if(e.day!=UnknownDay||s.at(0)->day!=UnknownDay||s.undated!=1)return false;
 for(unsigned i=0;i<8;++i){s.change(stamp(t+i*86400),480);}
 unsigned retained=0;for(auto &d:s.days)if(d.context)++retained;if(retained!=4)return false;
 // Unmaintainable old-day summary is explicitly counted, never assigned today.
 auto before=s.summaryLost;Event oldDay{2,s.context,t,t,t,1,0,s.zone,1};
 if(!s.append(oldDay)||s.summaryLost!=before+1||s.find(s.context,s.day)->events)return false;
 s.undated=UINT32_MAX;s.change({0,false},480);p.candidate({1,false},s);p.confirm(s,{2,false});
 return s.undated==UINT32_MAX&&!s.change(stamp(t),841);
}
constexpr bool calendarChecks(){
 for(auto d: {TimeTest::Date{2023,12,31,15,59,59},TimeTest::Date{2024,1,31,15,59,59},TimeTest::Date{2024,2,28,15,59,59},TimeTest::Date{2024,2,29,15,59,59}}){
  Store s;s.initialize();auto t=TimeTest::epoch(d);s.observe(stamp(t));auto c=s.context,day=s.day;
  s.observe(stamp(t+1));if(s.context!=c||s.day!=day+1||!s.find(c,day))return false;
  s.observe(stamp(t+86400*4));if(s.context==c||!s.find(c,day))return false;
 }
 auto y=TimeTest::date(TimeTest::epoch({2023,12,31,23,59,59})+1);
 auto leap=TimeTest::date(TimeTest::epoch({2024,2,28,23,59,59})+1);
 auto mar=TimeTest::date(TimeTest::epoch({2024,2,29,23,59,59})+1);
 auto min=TimeTest::date(TimeTest::localEpoch(TimeTest::kMinUtc,-840));
 auto max=TimeTest::date(TimeTest::localEpoch(TimeTest::kMaxUtc,840));
 return y.year==2024&&y.month==1&&y.day==1&&leap.day==29&&mar.month==3&&mar.day==1&&min.year==1999&&max.year==2100;
}
}
