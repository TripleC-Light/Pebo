#include <Arduino.h>
#include <stdio.h>
#include "EventLogic.h"
#include "DeadlineRtc.h"
#include "WakeSources.h"
extern "C" void SystemClock_Config(void);
extern "C" void RTC_IRQHandler(void);
using namespace EventLoop;
static Uart DebugUart(PA10,PA9);
static Parser parser;
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
static bool serialOpen=true;
static bool probeRtcConfirmed=false;
constexpr uint16_t kTxSize=640,kAdmission=480;
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
static void motionIsr() { record(Motion); }
static void gpoIsr() { record(Gpo); }
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
}
static void status() {
  char line[256];
  snprintf(line,sizeof(line),"OK mode=%s rtc=%u/%u lis=%u/%u st=%u/%u pins=%u,%u lpd=%u timeValid=%u stops=%lu aborted=%lu field=%d fieldOn=%lu fieldOff=%lu error=%s\r\n",
    runState.mode==Mode::Debug?"debug":runState.mode==Mode::Probe?"probe":"run",
    sources[Rtc].enabled,sources[Rtc].fault,sources[Motion].enabled,sources[Motion].fault,
    sources[Gpo].enabled,sources[Gpo].fault,unsigned(digitalRead(PB0)),unsigned(digitalRead(PA3)),
    unsigned(digitalRead(PA4)),unsigned(DeadlineRtc::timeValid()),(unsigned long)stops,
    (unsigned long)aborted,int(lastField),(unsigned long)fieldOn,(unsigned long)fieldOff,lastError);queue(line);
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
    unsigned(digitalRead(PA3)),(unsigned long)((GPIOA->PUPDR>>6)&3),
    (unsigned long)EXTI->IMR,(unsigned long)EXTI->RTSR);queue(line);
}
static void detachSource(Source source) {
  if(source==Motion)detachInterrupt(digitalPinToInterrupt(PB0));
  if(source==Gpo)detachInterrupt(digitalPinToInterrupt(PA3));
  sources[source].enabled=false;sources[source].retry=false;
}
static void quarantine(Source source,const char *error) {
  detachSource(source);sources[source].fault=true;lastError=error;
  bool clean=source==Motion?WakeSources::stopMotion():source==Gpo?WakeSources::stopGpo():DeadlineRtc::stop();
  if(!clean)lastError="source_cleanup_failed";
}
void stopEventSources() {
  detachSource(Motion);detachSource(Gpo);
  if(!WakeSources::stopMotion())lastError="lis_off_failed";
  if(!WakeSources::stopGpo())lastError="st_off_failed";
  if(DeadlineRtc::ready && !DeadlineRtc::stop())lastError="rtc_off_failed";
  sources[Rtc].enabled=false;sources[Rtc].retry=false;
  // Off deliberately cancels pending work only after producers have been detached.
  (void)takeEvents();WakeSources::endBus();digitalWrite(PA4,HIGH);
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
  if(motion){pinMode(PB0,INPUT);attachInterrupt(digitalPinToInterrupt(PB0),motionIsr,RISING);}
  else {lastError="lis_init";WakeSources::stopMotion();}
  if(gpo){pinMode(PA3,INPUT);attachInterrupt(digitalPinToInterrupt(PA3),gpoIsr,RISING);}
  else {lastError="st_init_field_config_or_i2c";WakeSources::stopGpo();}
  sources[Rtc].enabled=DeadlineRtc::arm();sources[Rtc].fault=!sources[Rtc].enabled;
  if(!sources[Rtc].enabled){lastError="rtc_arm";stopEventSources();return false;}
  return true;
}
void processEvents() {
  const uint8_t events=takeEvents();
  for(uint8_t index=0;index<Count;++index) {
    Source source=Source(index);SourceState &s=sources[index];
    bool active=source==Motion?digitalRead(PB0)==HIGH:source==Gpo?digitalRead(PA3)==HIGH:false;
    if(!s.enabled)continue;
    bool work=(events&sourceBit(source)) || s.due(millis()) || (active&&!s.retry);
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
    if(source!=Rtc)active=source==Motion?digitalRead(PB0)==HIGH:digitalRead(PA3)==HIGH;
    if(!s.outcome(ok,active,millis()) || (source!=Rtc && s.attempts>=kMaxServiceAttempts))
      quarantine(source,source==Motion?"lis_fault_or_budget":source==Gpo?"st_fault_or_budget":"rtc_read_fault");
  }
}
static void finishRun(const char *reason) {
  runState.stop();DeadlineRtc::stop();sources[Rtc].enabled=false;
  WakeSources::endBus();
  if(!serialOpen){DebugUart.begin(115200);serialOpen=true;}
  char line[160];
  snprintf(line,sizeof(line),"RUN_END reason=%s requested=%u elapsed=%lu stops=%lu unknownWake=%lu lastLis=%02X lastSt=%02X\r\n",
    reason,unsigned(runState.requested),(unsigned long)runElapsed,(unsigned long)stops,
    (unsigned long)unknownWake,unsigned(lastLis),unsigned(lastSt));queue(line);counters();
}
bool standbyIfIdle() {
  if(runState.mode!=Mode::Run || txRead!=txWrite)return false;
  WakeSources::endBus();
  if(serialOpen){DebugUart.flush();DebugUart.end();serialOpen=false;pinMode(PA9,INPUT_ANALOG);pinMode(PA10,INPUT_ANALOG);}
  uint32_t mask=lock();
  uint32_t tick=SysTick->CTRL,debug=DBGMCU->CR;
  SysTick->CTRL=0;SCB->ICSR=SCB_ICSR_PENDSTCLR_Msk;DBGMCU->CR=0;
  bool active=(sources[Motion].enabled&&digitalRead(PB0)==HIGH)||(sources[Gpo].enabled&&digitalRead(PA3)==HIGH);
  bool busy=pending || active || (EXTI->PR&EXTI->IMR&(GPIO_PIN_0|GPIO_PIN_3)) ||
    (RTC->ISR&RTC_ISR_WUTF) || (NVIC->ISPR[0]&NVIC->ISER[0]);
  if(!busy){__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);stops=saturated(stops);
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_STOPENTRY_WFI);}
  else aborted=saturated(aborted);
  SysTick->CTRL=tick;unlock(mask);
  // ISRs may now record all sources; they never require restored I2C/UART.
  SystemClock_Config();HAL_ResumeTick();DBGMCU->CR=debug;
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
static void handle(Command c) {
  switch(c.kind) {
    case Kind::None:return;
    case Kind::Help:queue("OK help commands=status,counters,counters clear,mode debug,run 1..60,off,help\r\n");return;
    case Kind::Status:status();return;
    case Kind::Probe:diagnosticProbe();return;
    case Kind::Counters:counters();return;
    case Kind::Clear:clearCounters();queue("OK counters_clear\r\n");return;
    case Kind::TooLong:queue("ERR too_long\r\n");return;
    case Kind::Invalid:queue("ERR invalid\r\n");return;
    case Kind::Debug:runState.stop();DeadlineRtc::stop();sources[Rtc].enabled=false;queue("OK debug\r\n");return;
    case Kind::Off:runState.stop();stopEventSources();queue("OK off\r\n");return;
    case Kind::Run:{
      if(runState.mode!=Mode::Debug){queue("ERR busy\r\n");return;}
      uint32_t clock=0;
      if(!DeadlineRtc::clock(clock)||!initializeEventSources()){queue("ERR rtc_deadline_unavailable\r\n");return;}
      probeRtcConfirmed=false;runState.begin(c.seconds,clock);probeMillis=millis();runElapsed=0;
      queue("RUN_BEGIN uart_rx_wake=0 preflight_max_ms=1500\r\n");return;
    }
  }
}
void setup() {
  __HAL_RCC_GPIOA_CLK_ENABLE();__HAL_RCC_GPIOB_CLK_ENABLE();
  digitalWrite(PA4,HIGH);pinMode(PA4,OUTPUT);
  // Keep PA3/PB0/PA4, SWD and LSE out of unused-pin shutdown.
  GPIO_InitTypeDef pins={};pins.Mode=GPIO_MODE_ANALOG;pins.Pull=GPIO_NOPULL;
  pins.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|
    GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;HAL_GPIO_Init(GPIOA,&pins);
  pins.Pin=GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|
    GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  HAL_GPIO_Init(GPIOB,&pins);pinMode(PA3,INPUT);pinMode(PB0,INPUT);
  DebugUart.begin(115200);
  if(!DeadlineRtc::begin())lastError="rtc_init";
}
void loop() {
  drain();processEvents();
  if(serialOpen)for(uint8_t i=0;i<32 && DebugUart.available()>0 && freeTx()>=kAdmission;++i){
    int c=DebugUart.read();if(c>=0)handle(parser.push(char(c)));
  }
  if(runState.mode!=Mode::Debug) {
    uint32_t clock=0;
    if(!sources[Rtc].enabled || !DeadlineRtc::clock(clock)){lastError="rtc_lost";finishRun("rtc_fault");}
    else if(runState.mode==Mode::Probe) {
      if(elapsedDay(clock,runState.start)>0 && probeRtcConfirmed){runState.validated(clock);queue("RUN_READY rtc_probe=1\r\n");}
      else if(uint32_t(millis()-probeMillis)>=1500){lastError="rtc_probe_timeout";finishRun("rtc_fault");}
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
