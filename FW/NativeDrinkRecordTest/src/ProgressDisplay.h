#pragma once
#include "Board.h"
#include "../../BlueLedProgressTest/ProgressLogic.h"
namespace Progress {
static BlueProgress::Display display;
static TIM_HandleTypeDef pwm={};
static uint32_t channel=TIM_CHANNEL_1;
static uint8_t applied=255;
static bool running=false,healthy=true;
constexpr uint16_t pins[]={GPIO_PIN_5,GPIO_PIN_6,GPIO_PIN_7,GPIO_PIN_8};
inline void release() {
  // This independent project owns TIM2/TIM22; never use alongside a piezo owner.
  if(running){HAL_TIM_PWM_Stop(&pwm,channel);HAL_TIM_PWM_DeInit(&pwm);running=false;}
  __HAL_RCC_TIM2_CLK_DISABLE();__HAL_RCC_TIM22_CLK_DISABLE();
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8,GPIO_PIN_RESET);
  GPIO_InitTypeDef p={};p.Pin=GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
  p.Mode=GPIO_MODE_OUTPUT_PP;p.Pull=GPIO_NOPULL;p.Speed=GPIO_SPEED_FREQ_LOW;HAL_GPIO_Init(GPIOA,&p);
}
inline void apply() {
  if(applied==display.pattern)return;
  release();applied=display.pattern;
  for(uint8_t i=0;i<4;++i){
    uint8_t state=(applied>>(2*i))&3;
    if(state==2)HAL_GPIO_WritePin(GPIOA,pins[i],GPIO_PIN_SET);
    if(state!=1)continue;
    pwm={};pwm.Instance=(i==0||i==3)?TIM2:TIM22;
    if(pwm.Instance==TIM2)__HAL_RCC_TIM2_CLK_ENABLE();else __HAL_RCC_TIM22_CLK_ENABLE();
    channel=i==2?TIM_CHANNEL_2:TIM_CHANNEL_1;
    pwm.Init.Prescaler=31;pwm.Init.CounterMode=TIM_COUNTERMODE_UP;
    pwm.Init.Period=1000000/BlueProgress::kPwmHz-1;pwm.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    TIM_OC_InitTypeDef c={};c.OCMode=TIM_OCMODE_PWM1;
    c.Pulse=(pwm.Init.Period+1)*BlueProgress::kLowDutyPercent/100;c.OCPolarity=TIM_OCPOLARITY_HIGH;
    GPIO_InitTypeDef p={};p.Pin=pins[i];p.Mode=GPIO_MODE_AF_PP;p.Pull=GPIO_NOPULL;
    p.Alternate=GPIO_AF5_TIM2;p.Speed=GPIO_SPEED_FREQ_LOW;
    healthy=HAL_TIM_PWM_Init(&pwm)==HAL_OK && HAL_TIM_PWM_ConfigChannel(&pwm,&c,channel)==HAL_OK;
    if(healthy){HAL_GPIO_Init(GPIOA,&p);healthy=HAL_TIM_PWM_Start(&pwm,channel)==HAL_OK;}
    running=true;
    if(!healthy){release();display.off();applied=0;return;}
  }
}
inline bool setProgress(uint8_t n){if(n>8)return false;display.level=n;return true;}
inline void showProgress(){display.show(display.level,HAL_GetTick());apply();}
inline bool active(){return display.phase!=BlueProgress::Phase::Off;}
inline void stopProgressDisplay(){display.off();applied=255;apply();}
inline void updateProgressDisplay(){display.update(HAL_GetTick());apply();}
}
