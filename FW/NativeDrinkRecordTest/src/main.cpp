#include "Board.h"
#include <stdio.h>
#include "../../EventLoopStopTest/EventLogic.h"
#include "DeadlineRtc.h"
#include "WakeSources.h"
#include "AppParser.h"
#include "TimeSupport.h"
#include "ProgressDisplay.h"
#include "Observation.h"
#include "DrinkRecords.h"
extern "C" void SystemClock_Config(void);
extern "C" void RTC_IRQHandler(void);
using namespace EventLoop;
static Records::Store realRecords;
static Records::Store &syntheticStore(){return Observation::diagnostic.synthetic;}
static Records::Pending recordPending;
static bool recordTest=false;
static uint32_t syntheticUtc=0;
static Records::Event recordPage[1];
static uint8_t pageCount=0,pageAt=0;
static bool pageActive=false,pageSynthetic=false;
static Board::Console DebugUart;
static AppParser parser;
static RunState runState;
static SourceState sources[Count];
static volatile uint8_t pending=0;
static volatile uint32_t observed[Count]={};
static uint32_t wakeWindow[Count]={};
static uint32_t stops=0,aborted=0,unknownWake=0,fieldOn=0,fieldOff=0;
static uint32_t probeMillis=0,runElapsed=0;
static uint8_t lastLis=0,lastAll=0,lastSt=0,staticGpo=0;
static int8_t lastField=-1;
static const char *lastError="none";
static void cancelApplication(const char *reason="off_debug_run_deadline");
static bool serialOpen=true;
static bool probeRtcConfirmed=false;
constexpr uint16_t kTxSize=1024,kAdmission=900;
static char tx[kTxSize];
static uint16_t txRead=0,txWrite=0;

static uint32_t lock() { uint32_t old=__get_PRIMASK();__disable_irq();__DMB();return old; }
static void unlock(uint32_t mask) { __DMB();__set_PRIMASK(mask); }
static void record(Source source) {
  // RTC can preempt an EXTI callback: protect shared read-modify-write in ISR too.
  uint32_t mask=lock();
  pending|=sourceBit(source);observed[source]=saturated(observed[source]);
  unlock(mask);
}
static uint8_t takeEvents() { uint32_t mask=lock();uint8_t flags=pending;pending=0;unlock(mask);return flags; }
static uint16_t freeTx() { return (txRead+kTxSize-txWrite-1)%kTxSize; }
static void queue(const char *s) {
  while (*s && freeTx()) {tx[txWrite]=*s++;txWrite=(txWrite+1)%kTxSize;}
}
static void drain() {
  if (!serialOpen) return;
  while(txRead!=txWrite && DebugUart.availableForWrite()>0) {
    DebugUart.write(uint8_t(tx[txRead]));txRead=(txRead+1)%kTxSize;
  }
}
static const char *sourceName(uint8_t i) {return i==Rtc?"rtc":i==Motion?"motion":"gpo";}
static void counters() {
  uint32_t seen[Count];uint32_t mask=lock();
  for(uint8_t i=0;i<Count;++i)seen[i]=observed[i];
  unlock(mask);
  char line[160];
  for(uint8_t i=0;i<Count;++i) {
    const SourceState &s=sources[i];
    snprintf(line,sizeof(line),"COUNT source=%s isr=%lu wake=%lu confirmed=%lu handled=%lu errors=%lu unknown=%lu\r\n",
      sourceName(i),(unsigned long)seen[i],(unsigned long)wakeWindow[i],(unsigned long)s.confirmed,(unsigned long)s.handled,
      (unsigned long)s.errors,(unsigned long)s.unknown);queue(line);
  }
  queue("END counters\r\n");
  char shake[192];
  snprintf(shake,sizeof(shake),"SHAKE entries=%lu windows=%lu confirmed=%lu rejected=%lu errors=%lu samples=%lu reversals=%u peak2=%lu\r\n",
    (unsigned long)Observation::entries,(unsigned long)Observation::windows,(unsigned long)Observation::successes,
    (unsigned long)Observation::rejected,(unsigned long)Observation::errors,(unsigned long)Observation::detector.samples,
    unsigned(Observation::detector.reversals),(unsigned long)Observation::detector.peak2);queue(shake);
}
static void status() {
  char line[256];
  snprintf(line,sizeof(line),"OK mode=%s rtc=%u/%u lis=%u/%u st=%u/%u pins=%u,%u lpd=%u timeValid=%u stops=%lu aborted=%lu field=%d fieldOn=%lu fieldOff=%lu error=%s\r\n",
    runState.mode==Mode::Debug?"debug":runState.mode==Mode::Probe?"probe":"run",
    sources[Rtc].enabled,sources[Rtc].fault,sources[Motion].enabled,sources[Motion].fault,
    sources[Gpo].enabled,sources[Gpo].fault,unsigned(Board::motionHigh()),unsigned(Board::gpoHigh()),
    unsigned(Board::lpdHigh()),unsigned(DeadlineRtc::timeValid()),(unsigned long)stops,
    (unsigned long)aborted,int(lastField),(unsigned long)fieldOn,(unsigned long)fieldOff,lastError);queue(line);
  snprintf(line,sizeof(line),"APP version=NativeDrinkRecordTest-v1 progress=%u display=%u pattern=%02X pwm=%u healthy=%u sampling=%u cooldown=%u trace=%u\r\n",
    unsigned(Progress::display.level),unsigned(Progress::active()),unsigned(Progress::display.pattern),
    unsigned(Progress::running),unsigned(Progress::healthy),unsigned(Observation::active),
    unsigned(Observation::gate.cooldown(AppTime::now())),unsigned(Observation::rows));queue(line);
}
static void drinkReport(){
  char line[256];
  snprintf(line,sizeof(line),"DRINK calibration=%u ref=%d,%d,%d phase=%u merge=%u candidates=%lu events=%lu rejected=%lu timeout=%lu errors=%lu cancelled=%lu dropped=%lu first=%lu last=%lu output=%lu limits=motion_not_intake\r\n",
    unsigned(Observation::reference.valid),Observation::reference.value.x,Observation::reference.value.y,Observation::reference.value.z,
    unsigned(Observation::drink.phase),unsigned(Observation::merge.pending),
    (unsigned long)Observation::candidates,(unsigned long)Observation::events,
    (unsigned long)Observation::drink.rejected,(unsigned long)Observation::drink.timeouts,
    (unsigned long)Observation::errors,(unsigned long)Observation::merge.cancelled,
    (unsigned long)Observation::traceDropped,(unsigned long)Observation::merge.first,
    (unsigned long)Observation::merge.last,(unsigned long)Observation::merge.confirmedAt);queue(line);
}
static Records::Stamp wallStamp(){
  uint32_t utc=0;bool valid=AppTime::epoch(utc);
  return {valid?utc:AppTime::now(),valid};
}
static void recordClock(bool edited=false){
  auto stamp=wallStamp();
  bool changed=edited?realRecords.change(stamp,realRecords.zone):realRecords.observe(stamp);
  if(changed||realRecords.exhausted){
    if(Observation::merge.pending)queue("DRINK_CANCELLED reason=time_context_change\r\n");
    Observation::merge.cancel();recordPending.cancel();
  }
}
static void confirmDrink(){
  if(!Observation::merge.confirm(AppTime::now()))return;
  if(!recordPending.confirm(realRecords,wallStamp()))queue("ERR record_rejected\r\n");
  Observation::events=saturated(Observation::events);char line[160];
  snprintf(line,sizeof(line),"DRINK_CONFIRMED first=%lu last=%lu count=%lu output=%lu stops=%lu clock=elapsed_ms intake_unproven=1\r\n",
    (unsigned long)Observation::merge.first,(unsigned long)Observation::merge.last,
    (unsigned long)Observation::merge.candidates,(unsigned long)Observation::merge.confirmedAt,(unsigned long)stops);queue(line);
}
static void diagnosticProbe() {
  if(runState.mode!=Mode::Debug){queue("ERR probe_requires_debug\r\n");return;}
  uint8_t config=0,gate=0,rf=0,eh=0;bool ok=false;
  {WakeSources::StAccess access;
    ok=WakeSources::read(0x57,0,true,config)&&WakeSources::read(0x53,0x2000,true,gate)&&
      WakeSources::read(0x53,0x2003,true,rf)&&WakeSources::read(0x53,0x2002,true,eh);}
  WakeSources::endBus();char line[192];
  snprintf(line,sizeof(line),"PROBE ok=%u static=%02X gate=%02X rf=%02X eh=%02X field=%d pa3=%u pull=%lu imr=%08lX rising=%08lX\r\n",
    unsigned(ok),unsigned(config),unsigned(gate),unsigned(rf),unsigned(eh),ok?int(bool(eh&4)):-1,
    unsigned(Board::gpoHigh()),(unsigned long)((GPIOA->PUPDR>>6)&3),
    (unsigned long)EXTI->IMR,(unsigned long)EXTI->RTSR);queue(line);
}
static void detachSource(Source source) {
  if(source==Motion)Board::motionInterrupt(false);
  if(source==Gpo)Board::gpoInterrupt(false);
  sources[source].enabled=false;sources[source].retry=false;
}
static void quarantine(Source source,const char *error) {
  detachSource(source);sources[source].fault=true;lastError=error;
  bool clean=source==Motion?WakeSources::stopMotion():source==Gpo?WakeSources::stopGpo():DeadlineRtc::stop();
  if(!clean)lastError="source_cleanup_failed";
}
void stopEventSources() {
  cancelApplication();
  detachSource(Motion);detachSource(Gpo);
  if(!WakeSources::stopMotion())lastError="lis_off_failed";
  if(!WakeSources::stopGpo())lastError="st_off_failed";
  if(DeadlineRtc::ready && !DeadlineRtc::stop())lastError="rtc_off_failed";
  sources[Rtc].enabled=false;sources[Rtc].retry=false;
  // Off deliberately cancels pending work only after producers have been detached.
  (void)takeEvents();WakeSources::endBus();Board::lpd(true);
}
bool initializeEventSources() {
  stopEventSources();
  for(uint8_t i=0;i<Count;++i){sources[i].fault=false;sources[i].failures=0;sources[i].attempts=0;}
  if(!DeadlineRtc::ready || !DeadlineRtc::configuration()) {lastError="rtc_not_ready";return false;}
  lastError="none";
  bool motion=false,gpo=false;
  for(uint8_t i=0;i<3 && !motion;++i)motion=WakeSources::configureMotion();
  for(uint8_t i=0;i<3 && !gpo;++i)gpo=WakeSources::configureGpo(staticGpo);
  sources[Motion].enabled=motion;sources[Motion].fault=!motion;
  sources[Gpo].enabled=gpo;sources[Gpo].fault=!gpo;
  if(motion){Board::motionInterrupt(true);}
  else {lastError="lis_init";WakeSources::stopMotion();}
  if(gpo){Board::gpoInterrupt(true);}
  else {lastError="st_init_field_config_or_i2c";WakeSources::stopGpo();}
  sources[Rtc].enabled=DeadlineRtc::arm();sources[Rtc].fault=!sources[Rtc].enabled;
  if(!sources[Rtc].enabled){lastError="rtc_arm";stopEventSources();return false;}
  return true;
}
void processEvents() {
  const uint8_t events=takeEvents();
  for(uint8_t index=0;index<Count;++index) {
    Source source=Source(index);SourceState &s=sources[index];
    if(source==Motion && Observation::active)continue;
    bool active=source==Motion?Board::motionHigh():source==Gpo?Board::gpoHigh():false;
    if(!s.enabled)continue;
    bool work=(events&sourceBit(source)) || s.due(HAL_GetTick()) || (active&&!s.retry);
    if(!work)continue;
    s.handled=saturated(s.handled);
    bool ok=false,confirmed=false;
    if(source==Rtc){uint32_t clock=0;ok=DeadlineRtc::clock(clock);confirmed=ok&&(events&sourceBit(Rtc));
      if(confirmed && runState.mode==Mode::Probe)probeRtcConfirmed=true;}
    else if(source==Motion){ok=WakeSources::motionCause(lastLis,lastAll);confirmed=ok&&((lastLis&8)||(lastAll&2));}
    else {
      ok=WakeSources::gpoCause(lastSt,lastField);confirmed=ok&&(lastSt&0x18);
      if(ok){if(lastField==1)fieldOn=saturated(fieldOn);else fieldOff=saturated(fieldOff);}
    }
    if(confirmed)s.confirmed=saturated(s.confirmed);else s.unknown=saturated(s.unknown);
    if(source!=Rtc)active=source==Motion?Board::motionHigh():Board::gpoHigh();
    if(!s.outcome(ok,active,HAL_GetTick()) || (source!=Rtc && s.attempts>=kMaxServiceAttempts))
      quarantine(source,source==Motion?"lis_fault_or_budget":source==Gpo?"st_fault_or_budget":"rtc_read_fault");
    if(source==Motion && confirmed && s.enabled){
      AppTime::update(true);
      uint32_t now=AppTime::now();Observation::entries=saturated(Observation::entries);
      if(!Observation::active&&!Observation::dumping){
        if(!Observation::begin(now)){
          if(!Observation::restore())quarantine(Motion,"lis_restore");
          lastError="observation_start";
        }else s.retry=false;
      }
    }
  }
}
static void finishRun(const char *reason) {
  cancelApplication();
  runState.stop();DeadlineRtc::stop();sources[Rtc].enabled=false;
  WakeSources::endBus();
  if(!serialOpen){DebugUart.begin(115200);serialOpen=true;}
  char line[160];
  snprintf(line,sizeof(line),"RUN_END reason=%s requested=%u elapsed=%lu stops=%lu unknownWake=%lu lastLis=%02X lastSt=%02X\r\n",
    reason,unsigned(runState.requested),(unsigned long)runElapsed,(unsigned long)stops,
    (unsigned long)unknownWake,unsigned(lastLis),unsigned(lastSt));queue(line);counters();
}
bool standbyIfIdle() {
  if(runState.mode!=Mode::Run || txRead!=txWrite || Observation::active || Progress::active() || Observation::dumping || pageActive)return false;
  WakeSources::endBus();
  if(serialOpen){if(!DebugUart.flush()){lastError="uart_flush_timeout";finishRun("uart_fault");return false;}DebugUart.end();serialOpen=false;}
  uint32_t mask=lock();
  uint32_t tick=SysTick->CTRL,debug=DBGMCU->CR;
  SysTick->CTRL=0;SCB->ICSR=SCB_ICSR_PENDSTCLR_Msk;DBGMCU->CR=0;
  bool active=(sources[Motion].enabled&&Board::motionHigh())||(sources[Gpo].enabled&&Board::gpoHigh());
  bool busy=pending || active || (EXTI->PR&EXTI->IMR&(GPIO_PIN_0|GPIO_PIN_3)) ||
    (RTC->ISR&RTC_ISR_WUTF) || (NVIC->ISPR[0]&NVIC->ISER[0]);
  if(!busy){__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);stops=saturated(stops);
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_STOPENTRY_WFI);}
  else aborted=saturated(aborted);
  SysTick->CTRL=tick;unlock(mask);
  // ISRs may now record all sources; they never require restored I2C/UART.
  SystemClock_Config();HAL_ResumeTick();DBGMCU->CR=debug;
  // HAL tick can equal the pre-STOP value: force RTC elapsed refresh before
  // a wake event starts an observation or evaluates its cooldown.
  if(!busy)AppTime::update(true);
  if(!busy)for(uint8_t i=0;i<Count;++i)if(pending&sourceBit(Source(i)))wakeWindow[i]=saturated(wakeWindow[i]);
  if(!busy && !pending)unknownWake=saturated(unknownWake);
  return !busy;
}
static void clearCounters() {
  uint32_t mask=lock();for(uint8_t i=0;i<Count;++i)observed[i]=0;unlock(mask);
  for(uint8_t i=0;i<Count;++i){sources[i].confirmed=sources[i].handled=sources[i].errors=sources[i].unknown=0;wakeWindow[i]=0;}
  stops=aborted=unknownWake=fieldOn=fieldOff=0;
  // Keep pending work, retry budgets and fault state intact.
}
static void cancelApplication(const char *reason){
  recordPending.cancel();
  if(Observation::merge.pending){queue("DRINK_CANCELLED reason=");queue(reason);queue("\r\n");}
  Progress::stopProgressDisplay();Observation::dumping=false;
  if(!Observation::cancel())quarantine(Motion,"lis_restore");
}
static void clockReport(){
  uint32_t utc=0;bool valid=AppTime::epoch(utc);char line[160];
  snprintf(line,sizeof(line),"CLOCK valid=%u utc=%lu lse=%u bkp2=%08lX bkp3=%08lX monotonicMs=%lu bkp0=%08lX bkp1=%08lX bkp4=%08lX\r\n",
    unsigned(valid),(unsigned long)utc,unsigned(DeadlineRtc::configuration()),
    (unsigned long)RTC->BKP2R,(unsigned long)RTC->BKP3R,(unsigned long)AppTime::now(),
    (unsigned long)RTC->BKP0R,(unsigned long)RTC->BKP1R,(unsigned long)RTC->BKP4R);queue(line);
}
static void recordsReport(bool today){
  const auto &s=recordTest?syntheticStore():realRecords;char line[256];
  const char *label=recordTest?"SYNTHETIC":"REAL";
  if(!today){
    snprintf(line,sizeof(line),"RECORDS %s ctx=%lu day=%lu tz=%d valid=%u kept=%u first=%lu last=%lu overwrite=%lu undated=%lu summaryLost=%lu rejected=%lu exhausted=%u bytes=%u store=%u RAM_ONLY reset_clears=1\r\n",
      label,(unsigned long)s.context,(unsigned long)s.day,int(s.zone),unsigned(s.valid),unsigned(s.count),
      (unsigned long)(s.count?s.at(0)->seq:0),(unsigned long)s.lastSeq,(unsigned long)s.overwritten,
      (unsigned long)s.undated,(unsigned long)s.summaryLost,(unsigned long)s.rejected,unsigned(s.exhausted),unsigned(sizeof(Records::Event)),unsigned(sizeof(Records::Store)));queue(line);
    return;
  }
  auto date=s.valid?TimeTest::date(s.day*86400u):TimeTest::Date{0,0,0,0,0,0};
  snprintf(line,sizeof(line),"TODAY %s date=%04d-%02u-%02u ctx=%lu day=%lu tz=%d valid=%u undated=%lu summaries=4_FIFO sat32 candidates_not_intake\r\n",
    label,date.year,unsigned(date.month),unsigned(date.day),(unsigned long)s.context,(unsigned long)s.day,int(s.zone),unsigned(s.valid),(unsigned long)s.undated);queue(line);
  for(const auto &d:s.days)if(d.context){
    snprintf(line,sizeof(line),"DAY %s ctx=%lu day=%lu tz=%d events=%lu candidates=%lu lastFirstUtc=%lu current=%u\r\n",label,
      (unsigned long)d.context,(unsigned long)d.day,int(d.zone),(unsigned long)d.events,(unsigned long)d.candidates,
      (unsigned long)d.last,unsigned(d.context==s.context&&d.day==s.day));queue(line);
  }
}
static void recordsPage(uint8_t page){
  if(pageActive){queue("ERR records_list_busy\r\n");return;}
  auto &s=recordTest?syntheticStore():realRecords;pageAt=pageCount=0;pageSynthetic=recordTest;
  for(uint8_t i=0;i<1;++i)if(const auto *e=s.at(page+i))recordPage[pageCount++]=*e;
  pageActive=true;char line[128];
  snprintf(line,sizeof(line),"LIST_BEGIN %s page=%u count=%u first=%lu last=%lu overwrite=%lu snapshot=page1\r\n",
    recordTest?"SYNTHETIC":"REAL",unsigned(page),unsigned(pageCount),
    (unsigned long)(s.count?s.at(0)->seq:0),(unsigned long)s.lastSeq,(unsigned long)s.overwritten);queue(line);
}
static void recordsDrain(){
  if(!pageActive||freeTx()<300)return;
  if(pageAt==pageCount){queue(pageSynthetic?"LIST_END SYNTHETIC\r\n":"LIST_END REAL\r\n");pageActive=false;return;}
  const auto &e=recordPage[pageAt++];char line[240];
  snprintf(line,sizeof(line),"RECORD %s seq=%lu ctx=%lu first=%lu last=%lu confirmed=%lu candidates=%lu day=%lu tz=%d utcValid=%u units=%s\r\n",
    pageSynthetic?"SYNTHETIC":"REAL",(unsigned long)e.seq,(unsigned long)e.context,(unsigned long)e.first,
    (unsigned long)e.last,(unsigned long)e.confirmed,(unsigned long)e.candidates,(unsigned long)e.day,int(e.zone),unsigned(e.flags&1),
    e.flags&1?"UTC_seconds":"elapsed_ms");queue(line);
}
static bool recordCommand(AppCommand c){
  if(c.extra<Extra::RecordsStatus)return false;
  if(c.extra==Extra::RecordsStatus){recordsReport(false);return true;}
  if(c.extra==Extra::RecordsToday){recordsReport(true);return true;}
  if(c.extra==Extra::RecordsList){recordsPage(c.value);return true;}
  if(pageActive){queue("ERR records_list_busy\r\n");return true;}
  if(c.extra==Extra::RecordsClear){
    if(Observation::merge.pending||Observation::active){queue("ERR records_clear_busy\r\n");return true;}
    (recordTest?syntheticStore():realRecords).clear();recordPending.cancel();
    queue(recordTest?"CLEARED SYNTHETIC events_stats_seq RTC_kept\r\n":"CLEARED REAL events_stats_seq RTC_calibration_kept\r\n");return true;
  }
  if(c.extra==Extra::TestEnter){
    if(recordTest||runState.mode!=Mode::Debug||Observation::active||Observation::merge.pending||Observation::dumping||Progress::active()||sources[Rtc].enabled||sources[Motion].enabled||sources[Gpo].enabled){queue("ERR test_requires_off_idle\r\n");return true;}
    new (&Observation::diagnostic.synthetic) Records::Store{};syntheticStore().initialize();Observation::rows=0;syntheticUtc=TimeTest::epoch({2024,2,28,15,59,55});
    syntheticStore().observe({syntheticUtc,true});recordTest=true;queue("TEST_BEGIN SYNTHETIC RTC_unchanged\r\n");return true;
  }
  if(!recordTest){queue("ERR not_record_test\r\n");return true;}
  if(c.extra==Extra::TestExit){recordTest=false;new (&Observation::diagnostic) Observation::Diagnostics{};Observation::rows=0;queue("TEST_END SYNTHETIC REAL_retained\r\n");return true;}
  if(c.extra==Extra::TestDay){syntheticUtc+=86400;syntheticStore().observe({syntheticUtc,true});queue("TEST_DAY SYNTHETIC\r\n");return true;}
  Records::Pending test;test.candidate({syntheticUtc,true},syntheticStore());test.candidate({syntheticUtc+2,true},syntheticStore());
  syntheticStore().observe({syntheticUtc+12,true});bool ok=test.confirm(syntheticStore(),{syntheticUtc+12,true},true);
  syntheticUtc+=20;queue(ok?"TEST_ADDED SYNTHETIC candidates=2 intake_unproven=1\r\n":"ERR SYNTHETIC record_rejected\r\n");return true;
}
static void handle(AppCommand c) {
  if(recordCommand(c))return;
  if(recordTest && (c.extra!=Extra::None || (c.kind!=Kind::None&&c.kind!=Kind::Status&&c.kind!=Kind::Help&&c.kind!=Kind::Off&&c.kind!=Kind::Debug&&c.kind!=Kind::Invalid&&c.kind!=Kind::TooLong))){queue("ERR record_test_active\r\n");return;}

  if(c.extra!=Extra::None){
    if(c.extra==Extra::CalibrationStatus||c.extra==Extra::DrinkStatus||c.extra==Extra::DrinkCounters){drinkReport();return;}
    if(c.extra==Extra::CalibrationStart||c.extra==Extra::Capture){
      if(runState.mode!=Mode::Debug||Observation::active||Observation::dumping||!sources[Motion].enabled){queue("ERR acquisition_busy_or_disabled\r\n");return;}
      if(c.extra==Extra::CalibrationStart&&Observation::merge.pending){queue("ERR merge_pending\r\n");return;}
      AppTime::update(true);
      bool cal=c.extra==Extra::CalibrationStart;
      if(Observation::begin(AppTime::now(),cal,cal?2200:c.value*1000,!cal))queue(cal?"CALIBRATION_BEGIN keep_upright_still\r\n":"CAPTURE_BEGIN units=LP1_counts odr=100 max_seconds=30\r\n");
      else {if(!Observation::restore())quarantine(Motion,"lis_restore");queue("ERR acquisition_failed\r\n");}
      return;
    }
    if(c.extra==Extra::LisRegs){
      uint8_t v[6]={};bool ok=true;
      for(uint8_t i=0;i<6;++i)if(!WakeSources::read(0x19,0x20+i,false,v[i]))ok=false;
      char line[100];snprintf(line,sizeof(line),"LISREG ok=%u ctrl1=%02X ctrl2=%02X ctrl3=%02X ctrl4=%02X ctrl5=%02X ctrl6=%02X\r\n",
        unsigned(ok),unsigned(v[0]),unsigned(v[1]),unsigned(v[2]),unsigned(v[3]),unsigned(v[4]),unsigned(v[5]));
      queue(line);return;
    }
    if(c.extra==Extra::Observe){
      if(runState.mode!=Mode::Debug||!sources[Motion].enabled||Observation::active||
        Progress::active()||Observation::dumping){queue("ERR observe_busy_or_disabled\r\n");return;}
      AppTime::update(true);
      if(Observation::begin(AppTime::now())){queue("OK observe diagnostic_not_motion_wake\r\n");}
      else {if(!Observation::restore())quarantine(Motion,"lis_restore");queue("ERR observe_failed\r\n");}
      return;
    }
    if(c.extra==Extra::LedRegs){
      char line[128];snprintf(line,sizeof(line),"LEDREG apb1=%08lX apb2=%08lX mode=%08lX odr=%08lX\r\n",
        (unsigned long)RCC->APB1ENR,(unsigned long)RCC->APB2ENR,
        (unsigned long)GPIOA->MODER,(unsigned long)GPIOA->ODR);queue(line);return;
    }
    if(c.extra==Extra::Progress || c.extra==Extra::Show){
      if(c.extra==Extra::Progress)Progress::setProgress(c.value);
      Progress::showProgress();queue("OK display\r\n");return;
    }
    if(c.extra==Extra::Clock){clockReport();return;}
    if(c.extra==Extra::Trace){
      if(runState.mode!=Mode::Debug || Observation::active){queue("ERR trace_busy\r\n");return;}
      Observation::dumpAt=0;Observation::dumping=true;queue("TRACE_BEGIN units=LP1_counts time=observation_ms\r\n");return;
    }
    if(runState.mode!=Mode::Debug){queue("ERR setutc_requires_debug\r\n");return;}
    cancelApplication("setutc");
    bool setOk=AppTime::set(c.value);recordClock(true);queue(setOk?"OK setutc\r\n":"ERR setutc_failed\r\n");clockReport();return;
  }
  switch(c.kind) {
    case Kind::None:return;
    case Kind::Help:queue("RECORD_HELP records status,today,list 0..31,clear confirm,test enter/add/day/exit\r\n");queue("HELP status,counters[ clear],mode debug,run,off,progress,show,observe,trace,clock,setutc,calibration start/status,drink status/counters,capture,ledregs,lisregs\r\n");return;
    case Kind::Status:status();drinkReport();return;
    case Kind::Probe:diagnosticProbe();return;
    case Kind::Counters:counters();return;
    case Kind::Clear:clearCounters();Observation::entries=Observation::windows=Observation::successes=Observation::rejected=Observation::errors=0;
      Observation::candidates=Observation::events=Observation::cancelledWindows=Observation::traceDropped=Observation::calFailures=0;
      Observation::drink.rejected=Observation::drink.timeouts=Observation::merge.cancelled=0;
      queue("OK counters_clear\r\n");return;
    case Kind::TooLong:queue("ERR too_long\r\n");return;
    case Kind::Invalid:queue("ERR invalid\r\n");return;
    case Kind::Debug:cancelApplication();runState.stop();DeadlineRtc::stop();sources[Rtc].enabled=false;queue("OK debug\r\n");return;
    case Kind::Off:runState.stop();stopEventSources();queue("OK off\r\n");return;
    case Kind::Run:{
      if(runState.mode!=Mode::Debug){queue("ERR busy\r\n");return;}
      uint32_t clock=0;
      if(!DeadlineRtc::clock(clock)||!initializeEventSources()){queue("ERR rtc_deadline_unavailable\r\n");return;}
      probeRtcConfirmed=false;runState.begin(c.seconds,clock);probeMillis=HAL_GetTick();runElapsed=0;
      queue("RUN_BEGIN uart_rx_wake=0 preflight_max_ms=1500\r\n");return;
    }
  }
}
static void initialize() {
  realRecords.initialize();
  HAL_Init();SystemClock_Config();Board::initializePins();
  if(!DebugUart.begin(115200))while(true)__WFI();
  if(!DeadlineRtc::begin())lastError="rtc_init";
  Progress::stopProgressDisplay();AppTime::update();
  initializeEventSources();recordClock();queue("RECORD_BOOT RAM_seq_reset RTC_kept_if_valid\r\n");
}
static void service() {

  AppTime::update();recordClock();drain();processEvents();recordsDrain();
  if(!serialOpen && (txRead!=txWrite || Observation::active || Progress::active())){
    if(!DebugUart.begin(115200)){lastError="uart_resume";finishRun("uart_fault");}
    else serialOpen=true;
  }
  // Flush at the merge boundary before accepting another candidate. In run
  // mode, expiry of the entire run takes precedence over deferred output.
  bool runningExpired=false;uint32_t current=0;
  if(runState.mode==Mode::Run){
    runningExpired=!DeadlineRtc::clock(current)||runState.expired(current);
  }
  if(!runningExpired)confirmDrink();
  uint8_t outcome=runningExpired?0:Observation::update(AppTime::now());
  if(Observation::freshReady){
    if(freeTx()>=100){char line[80];const auto &r=Observation::fresh;
      snprintf(line,sizeof(line),"RAW %u %d %d %d\r\n",unsigned(r.ms),r.value.x,r.value.y,r.value.z);queue(line);
    }else Observation::traceDropped=saturated(Observation::traceDropped);
    Observation::freshReady=false;
  }
  if(outcome==2){queue("SHAKE_CONFIRMED\r\n");Progress::showProgress();}
  if(outcome==4){
    // Completion time captured now, never reconstructed from delayed confirmation.
    auto completed=wallStamp();
    if(Observation::merge.add(AppTime::now(),AppTime::now()))recordPending.candidate(completed,realRecords);
    char line[100];snprintf(line,sizeof(line),"DRINK_CANDIDATE time=%lu stops=%lu intake_unproven=1\r\n",(unsigned long)AppTime::now(),(unsigned long)stops);queue(line);
    if(!Observation::capturing){
      Observation::active=false;
      if(!Observation::restore())quarantine(Motion,"lis_restore");
      sources[Motion].retry=false;
    }
  }
  if(outcome==1||outcome==3||outcome==5||outcome==6){
    bool restored=Observation::restore();
    if(!restored)quarantine(Motion,"lis_restore");
    sources[Motion].retry=false;
    if(outcome==5)queue("CALIBRATION_OK ram_only=1\r\n");
    else if(outcome==6)queue("CALIBRATION_REJECTED previous_reference_retained=1\r\n");
    else queue(outcome==3?"OBSERVATION_ERROR\r\n":"OBSERVATION_END\r\n");
    if(Observation::capturing){Observation::capturing=false;queue("CAPTURE_END\r\n");}
  }
  Progress::updateProgressDisplay();
  if(Observation::dumping && freeTx()>=100){
    if(Observation::dumpAt>=Observation::rows){queue("TRACE_END\r\n");Observation::dumping=false;}
    else {const auto &r=Observation::trace[Observation::dumpAt++];char line[80];
      snprintf(line,sizeof(line),"SAMPLE %u %d %d %d\r\n",unsigned(r.ms),r.value.x,r.value.y,r.value.z);queue(line);}
  }
  if(serialOpen)for(uint8_t i=0;i<32 && DebugUart.available()>0 && freeTx()>=kAdmission;++i){
    int c=DebugUart.read();if(c>=0)handle(parser.push(char(c)));
  }
  if(runState.mode!=Mode::Debug) {
    uint32_t clock=0;
    if(!sources[Rtc].enabled || !DeadlineRtc::clock(clock)){lastError="rtc_lost";finishRun("rtc_fault");}
    else if(runState.mode==Mode::Probe) {
      if(elapsedDay(clock,runState.start)>0 && probeRtcConfirmed){runState.validated(clock);queue("RUN_READY rtc_probe=1\r\n");}
      else if(uint32_t(HAL_GetTick()-probeMillis)>=1500){lastError="rtc_probe_timeout";finishRun("rtc_fault");}
    } else {
      runElapsed=elapsedDay(clock,runState.start);
      if(unknownWake>=3){lastError="unknown_wake_limit";finishRun("unknown_wake");}
      else if(runState.expired(clock))finishRun("deadline");else if(!sources[Motion].retry&&!sources[Gpo].retry)standbyIfIdle();
    }
  }
  drain();
}
extern "C" void RTC_IRQHandler(void){HAL_RTCEx_WakeUpTimerIRQHandler(&DeadlineRtc::handle);}
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *h){(void)h;record(Rtc);}
extern "C" void EXTI0_1_IRQHandler(){HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);}
extern "C" void EXTI2_3_IRQHandler(){HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);}
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t pin){
  if(pin==GPIO_PIN_0 && sources[Motion].enabled)record(Motion);
  if(pin==GPIO_PIN_3 && sources[Gpo].enabled)record(Gpo);
}
int main(){initialize();while(true)service();}
