#pragma once
#include "DeadlineRtc.h"
#include "ElapsedLogic.h"
namespace AppTime {
static ElapsedClock clock;
static uint32_t lastPoll=UINT32_MAX;
inline bool readDayMs(uint32_t &out) {
  if(!DeadlineRtc::ready||!DeadlineRtc::configuration())return false;
  uint32_t ss=0,t=0;bool coherent=false;
  // ES0322 Rev5 2.9.2: re-read SSR after SSR/TR/DR, with bounded retries.
  for(uint8_t i=0;i<3;++i){
    ss=RTC->SSR;t=RTC->TR;(void)RTC->DR;
    uint32_t check=RTC->SSR;(void)RTC->DR;
    if(ss==check){coherent=true;break;}
  }
  if(!coherent)return false;
  uint32_t h=(t>>16)&63,m=(t>>8)&127,s=t&127;
  if(ss>255||!DeadlineRtc::bcd(h)||!DeadlineRtc::bcd(m)||!DeadlineRtc::bcd(s)||
    DeadlineRtc::decimal(h)>23||DeadlineRtc::decimal(m)>59||DeadlineRtc::decimal(s)>59)return false;
  out=(DeadlineRtc::decimal(h)*3600u+DeadlineRtc::decimal(m)*60u+DeadlineRtc::decimal(s))*1000u+
    (255-ss)*1000u/256;return true;
}
inline void update(bool force=false){
  uint32_t tick=HAL_GetTick();
  if(!force && clock.seeded && tick==lastPoll)return;
  lastPoll=tick;
  uint32_t value=0;
  if(!readDayMs(value))return;
  clock.accept(value);
}
inline uint32_t now(){return clock.elapsed;}
inline bool epoch(uint32_t &out) {
  if(!DeadlineRtc::timeValid())return false;
  uint32_t t=RTC->TR,d=RTC->DR;
  TimeTest::Date date{2000+DeadlineRtc::decimal((d>>16)&255),DeadlineRtc::decimal((d>>8)&31),
    DeadlineRtc::decimal(d&63),DeadlineRtc::decimal((t>>16)&63),
    DeadlineRtc::decimal((t>>8)&127),DeadlineRtc::decimal(t&127)};
  out=TimeTest::epoch(date);return true;
}
inline bool set(uint32_t utc){
  if(!DeadlineRtc::ready||utc<TimeTest::kMinUtc||utc>TimeTest::kMaxUtc)return false;
  if((RTC->BKP2R && RTC->BKP2R!=0x544d0101)||(!RTC->BKP2R&&RTC->BKP3R))return false;
  auto d=TimeTest::date(utc);RTC_TimeTypeDef t={};RTC_DateTypeDef date={};
  t.Hours=d.hour;t.Minutes=d.minute;t.Seconds=d.second;t.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
  t.StoreOperation=RTC_STOREOPERATION_RESET;
  date.Year=d.year-2000;date.Month=d.month;date.Date=d.day;date.WeekDay=TimeTest::weekday(utc);
  AppTime::update();RTC->BKP2R=0;
  bool ok=HAL_RTC_SetTime(&DeadlineRtc::handle,&t,RTC_FORMAT_BIN)==HAL_OK &&
    HAL_RTC_SetDate(&DeadlineRtc::handle,&date,RTC_FORMAT_BIN)==HAL_OK;
  if(ok){RTC->BKP3R=utc;RTC->BKP2R=0x544d0101;}
  clock.seeded=false;update();return ok&&DeadlineRtc::timeValid();
}
}
