#pragma once
#include "Board.h"
#include "../../TimeManagementTest/TimeLogic.h"
namespace DeadlineRtc {
static RTC_HandleTypeDef handle={};
static bool ready=false;
inline bool configuration() {
  return (RCC->CSR&RCC_CSR_RTCSEL)==RCC_CSR_RTCSEL_LSE &&
    (RCC->CSR&RCC_CSR_LSERDY) && !(RCC->CSR&RCC_CSR_LSEBYP) &&
    (RCC->CSR&RCC_CSR_RTCEN) && RTC->PRER==0x007f00ff && !(RTC->CR&(RTC_CR_FMT|RTC_CR_BYPSHAD));
}
inline uint8_t decimal(uint32_t n) { return uint8_t((n>>4)*10+(n&15)); }
inline bool bcd(uint32_t n) { return (n&15)<=9 && (n>>4)<=9; }
inline bool clock(uint32_t &seconds) {
  if (!ready || !configuration()) return false;
  uint32_t t=RTC->TR; (void)RTC->DR;
  uint32_t h=(t>>16)&63,m=(t>>8)&127,s=t&127;
  if (!bcd(h)||!bcd(m)||!bcd(s)||decimal(h)>23||decimal(m)>59||decimal(s)>59) return false;
  seconds=decimal(h)*3600u+decimal(m)*60u+decimal(s); return true;
}
inline bool timeValid() {
  if (!ready || !configuration() || RTC->BKP2R!=0x544d0101 || !(RTC->ISR&RTC_ISR_INITS)) return false;
  uint32_t t=RTC->TR,d=RTC->DR;
  uint32_t f[]={(d>>16)&255,(d>>8)&31,d&63,(t>>16)&63,(t>>8)&127,t&127};
  for (uint8_t i=0;i<6;++i) if (!bcd(f[i])) return false;
  TimeTest::Date value{2000+decimal(f[0]),decimal(f[1]),decimal(f[2]),decimal(f[3]),decimal(f[4]),decimal(f[5])};
  return TimeTest::valid(value) && TimeTest::epoch(value)>=RTC->BKP3R &&
    TimeTest::weekday(TimeTest::epoch(value))==((d>>13)&7);
}
inline void clear() {
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&handle,RTC_FLAG_WUTF);
  __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG(); HAL_NVIC_ClearPendingIRQ(RTC_IRQn);
}
inline bool stop() {
  if (!ready) return false;
  bool ok=HAL_RTCEx_DeactivateWakeUpTimer(&handle)==HAL_OK; clear(); return ok;
}
inline bool begin() {
  __HAL_RCC_PWR_CLK_ENABLE(); HAL_PWR_EnableBkUpAccess();
  uint32_t source=RCC->CSR&RCC_CSR_RTCSEL;
  if ((source && source!=RCC_CSR_RTCSEL_LSE) || (RCC->CSR&RCC_CSR_LSEBYP)) return false;
  SET_BIT(RCC->CSR,RCC_CSR_LSEON);
  uint32_t start=HAL_GetTick();
  while (!(RCC->CSR&RCC_CSR_LSERDY)) if (uint32_t(HAL_GetTick()-start)>=5000) return false;
  if (!source) SET_BIT(RCC->CSR,RCC_CSR_RTCSEL_LSE);
  __HAL_RCC_RTC_ENABLE();
  if ((RTC->ISR&RTC_ISR_INITS) && !configuration()) return false;
  if (RTC->CR&(RTC_CR_ALRAE|RTC_CR_ALRAIE|RTC_CR_TSE|RTC_CR_TSIE)) return false;
  handle.Instance=RTC; handle.Init.HourFormat=RTC_HOURFORMAT_24;
  handle.Init.AsynchPrediv=127; handle.Init.SynchPrediv=255;
  handle.Init.OutPut=RTC_OUTPUT_DISABLE; handle.Init.OutPutPolarity=RTC_OUTPUT_POLARITY_HIGH;
  handle.Init.OutPutType=RTC_OUTPUT_TYPE_OPENDRAIN;
  ready=HAL_RTC_Init(&handle)==HAL_OK && HAL_RTC_WaitForSynchro(&handle)==HAL_OK && configuration();
  if (ready && !stop())ready=false;
  return ready;
}
inline bool arm() {
  if (!stop()) return false;
  HAL_NVIC_SetPriority(RTC_IRQn,0,0); HAL_NVIC_EnableIRQ(RTC_IRQn);
  return HAL_RTCEx_SetWakeUpTimer_IT(&handle,0,RTC_WAKEUPCLOCK_CK_SPRE_16BITS)==HAL_OK;
}
}
