#pragma once
#include <stdint.h>
#include <stddef.h>

namespace TimeTest {
constexpr uint32_t kMinUtc = 946684800u, kMaxUtc = 4102444799u;
constexpr uint32_t kMaxSleep = 3600;
constexpr int16_t kDefaultZone = 480;
constexpr size_t kInputBytes = 64;
struct Date { int year; uint8_t month, day, hour, minute, second; };
constexpr bool leap(int y) { return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0); }
constexpr uint8_t monthDays(int y, int m) {
  return m == 2 ? (leap(y) ? 29 : 28) :
    (m == 4 || m == 6 || m == 9 || m == 11 ? 30 : 31);
}
constexpr bool valid(const Date &d, int first = 2000, int last = 2099) {
  return d.year >= first && d.year <= last && d.month >= 1 && d.month <= 12 &&
    d.day >= 1 && d.day <= monthDays(d.year,d.month) && d.hour < 24 &&
    d.minute < 60 && d.second < 60;
}
constexpr uint32_t epoch(const Date &d) {
  uint32_t days = 0;
  for (int y=1970; y<d.year; ++y) days += leap(y) ? 366 : 365;
  for (int m=1; m<d.month; ++m) days += monthDays(d.year,m);
  return (days+d.day-1)*86400u+d.hour*3600u+d.minute*60u+d.second;
}
constexpr Date date(uint32_t seconds) {
  uint32_t days = seconds / 86400;
  Date d{1970,1,1,uint8_t(seconds%86400/3600),uint8_t(seconds%3600/60),uint8_t(seconds%60)};
  while (days >= uint32_t(leap(d.year) ? 366 : 365)) {
    days -= leap(d.year) ? 366 : 365; ++d.year;
  }
  while (days >= monthDays(d.year,d.month)) { days -= monthDays(d.year,d.month); ++d.month; }
  d.day = uint8_t(days+1); return d;
}
// ISO weekday: Monday=1 .. Sunday=7. Epoch 1970-01-01 was Thursday.
constexpr uint8_t weekday(uint32_t seconds) { return uint8_t((seconds/86400+3)%7+1); }
constexpr uint32_t localEpoch(uint32_t utc, int16_t zone) {
  return uint32_t(int64_t(utc)+int32_t(zone)*60);
}
constexpr bool equal(const char *a, const char *b) {
  while (*a && *a == *b) { ++a; ++b; }
  return *a == *b;
}
constexpr bool prefix(const char *s, const char *p) {
  while (*p) { if (*s++ != *p++) return false; } return true;
}
constexpr bool number(const char *s, uint32_t &value) {
  if (!*s) return false;
  uint32_t n=0;
  for (;*s;++s) {
    if (*s<'0' || *s>'9' || n>(UINT32_MAX-uint32_t(*s-'0'))/10) return false;
    n=n*10+uint32_t(*s-'0');
  }
  value=n; return true;
}
constexpr bool parseDate(const char *s, uint32_t &utc) {
  size_t len=0; while (s[len]) ++len;
  if (len != 19) return false;
  int values[6]={};
  const uint8_t starts[6]={0,5,8,11,14,17}, sizes[6]={4,2,2,2,2,2};
  if (s[4]!='-' || s[7]!='-' || s[10]!=' ' || s[13]!=':' || s[16]!=':') return false;
  for (uint8_t i=0;i<6;++i) for (uint8_t j=0;j<sizes[i];++j) {
    const char c=s[starts[i]+j]; if (c<'0'||c>'9') return false;
    values[i]=values[i]*10+c-'0';
  }
  Date d{values[0],uint8_t(values[1]),uint8_t(values[2]),uint8_t(values[3]),uint8_t(values[4]),uint8_t(values[5])};
  if (!valid(d)) return false;
  utc=epoch(d); return true;
}
constexpr bool parseZone(const char *s, int16_t &zone) {
  size_t len=0; while (s[len]) ++len;
  if (len!=6 || (s[0]!='+'&&s[0]!='-') || s[3]!=':') return false;
  for (uint8_t i=1;i<6;++i)
    if (i!=3 && (s[i]<'0'||s[i]>'9')) return false;
  int h=(s[1]-'0')*10+s[2]-'0', m=(s[4]-'0')*10+s[5]-'0';
  if (m>=60 || h>14 || (h==14 && m)) return false;
  zone=int16_t((h*60+m)*(s[0]=='-'?-1:1)); return true;
}
enum class Kind : uint8_t { None, Help, Status, SetUtc, Zone, Real, SimSet, Advance, Sleep, Invalid, TooLong };
struct Command { Kind kind=Kind::Invalid; uint32_t value=0; int16_t zone=0; };
constexpr Command parse(const char *s) {
  Command c;
  if (equal(s,"help")) c.kind=Kind::Help;
  else if (equal(s,"status")) c.kind=Kind::Status;
  else if (equal(s,"mode real")) c.kind=Kind::Real;
  else if (prefix(s,"setutc ") && parseDate(s+7,c.value)) c.kind=Kind::SetUtc;
  else if (prefix(s,"sim set ") && parseDate(s+8,c.value)) c.kind=Kind::SimSet;
  else if (prefix(s,"tz ") && parseZone(s+3,c.zone)) c.kind=Kind::Zone;
  else if (prefix(s,"sim advance ") && number(s+12,c.value)) c.kind=Kind::Advance;
  else if (prefix(s,"sleep ") && number(s+6,c.value) && c.value>=1 && c.value<=kMaxSleep) c.kind=Kind::Sleep;
  return c;
}
struct Parser {
  char line[kInputBytes]={}; uint8_t used=0; bool discard=false, embeddedNull=false;
  constexpr Command push(char c) {
    if (c!='\n') {
      if (!c) embeddedNull=true;
      if (!discard && used<kInputBytes-1) line[used++]=c; else discard=true;
      return {Kind::None,0,0};
    }
    Command result;
    if (discard) result.kind=Kind::TooLong;
    else if (!embeddedNull) {
      if (used && line[used-1]=='\r') --used;
      line[used]=0; result=parse(line);
    }
    used=0; discard=embeddedNull=false; return result;
  }
};
struct Change { bool changed=false; uint32_t from=0,to=0; int32_t days=0; };
struct Context {
  bool sim=false, simValid=false, baseline=false;
  uint32_t simUtc=0, lastDay=0;
  int16_t zone=kDefaultZone;
  constexpr bool application(uint32_t rtc, bool rtcValid, uint32_t &out) const {
    if (sim) { if (!simValid) return false; out=simUtc; return true; }
    if (!rtcValid) return false;
    out=rtc; return true;
  }
  constexpr Change observe(uint32_t utc, bool isValid) {
    if (!isValid) { baseline=false; return {}; }
    uint32_t day=localEpoch(utc,zone)/86400;
    Change c{baseline&&day!=lastDay,lastDay,day,int32_t(day)-int32_t(lastDay)};
    lastDay=day; baseline=true; return c;
  }
  constexpr void rebase(uint32_t utc, bool isValid) { baseline=false; observe(utc,isValid); }
  constexpr bool advance(uint32_t seconds) {
    if (!sim || !simValid || seconds>kMaxUtc-simUtc) return false;
    simUtc+=seconds; return true;
  }
};
}
