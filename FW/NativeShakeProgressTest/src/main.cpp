#include "Board.h"
#include <stdio.h>
#include "../../EventLoopStopTest/EventLogic.h"
#include "DeadlineRtc.h"
#include "WakeSources.h"
#include "AppParser.h"
#include "TimeSupport.h"
#include "ProgressDisplay.h"
#include "Observation.h"
extern "C" void SystemClock_Config(void);
extern "C" void RTC_IRQHandler(void);
using namespace EventLoop;
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
static void cancelApplication();
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
  snprintf(line,sizeof(line),"APP version=NativeShakeProgressTest-v1 progress=%u display=%u pattern=%02X pwm=%u healthy=%u sampling=%u cooldown=%u trace=%u\r\n",
    unsigned(Progress::display.level),unsigned(Progress::active()),unsigned(Progress::display.pattern),
    unsigned(Progress::running),unsigned(Progress::healthy),unsigned(Observation::active),
    unsigned(Observation::gate.cooldown(AppTime::now())),unsigned(Observation::rows));queue(line);
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
      bool allow=Observation::gate.allow(now);Observation::gate.motion(now);
      if(allow&&!Progress::active()&&!Observation::dumping){
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
  if(runState.mode!=Mode::Run || txRead!=txWrite || Observation::active || Progress::active() || Observation::dumping)return false;
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
static void cancelApplication(){
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
static void handle(AppCommand c) {
  if(c.extra!=Extra::None){
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
    cancelApplication();
    queue(AppTime::set(c.value)?"OK setutc\r\n":"ERR setutc_failed\r\n");clockReport();return;
  }
  switch(c.kind) {
    case Kind::None:return;
    case Kind::Help:queue("OK help commands=status,counters,counters clear,mode debug,run 1..60,off,help,progress 0..8,show,observe,trace,clock,ledregs,lisregs,setutc YYYY-MM-DD HH:MM:SS\r\n");return;
    case Kind::Status:status();return;
    case Kind::Probe:diagnosticProbe();return;
    case Kind::Counters:counters();return;
    case Kind::Clear:clearCounters();Observation::entries=Observation::windows=Observation::successes=Observation::rejected=Observation::errors=0;queue("OK counters_clear\r\n");return;
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
  HAL_Init();SystemClock_Config();Board::initializePins();
  if(!DebugUart.begin(115200))while(true)__WFI();
  if(!DeadlineRtc::begin())lastError="rtc_init";
  Progress::stopProgressDisplay();AppTime::update();
  initializeEventSources();
}
static void service() {

  AppTime::update();drain();processEvents();
  if(!serialOpen && (Observation::active || Progress::active())){
    if(!DebugUart.begin(115200)){lastError="uart_resume";finishRun("uart_fault");}
    else serialOpen=true;
  }
  uint8_t outcome=Observation::update(AppTime::now());
  if(outcome){
    bool restored=Observation::restore();
    if(!restored)quarantine(Motion,"lis_restore");
    sources[Motion].retry=false;
    if(outcome==2 && restored){queue("SHAKE_CONFIRMED\r\n");Progress::showProgress();}
    else queue(outcome==3?"SHAKE_ERROR\r\n":"SHAKE_NOT_CONFIRMED\r\n");
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
