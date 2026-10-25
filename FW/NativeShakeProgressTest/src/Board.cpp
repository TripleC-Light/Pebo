#include "Board.h"
#include <errno.h>
#include <stddef.h>
namespace {
UART_HandleTypeDef uart={};
I2C_HandleTypeDef i2c={};
bool bus=false,serial=false;
uint8_t rxByte=0,rx[64]={},tx[64]={};
volatile uint8_t rxRead=0,rxWrite=0,txRead=0,txWrite=0;
volatile bool transmitting=false;
uint32_t critical() {uint32_t p=__get_PRIMASK();__disable_irq();__DMB();return p;}
void release(uint32_t p){__DMB();__set_PRIMASK(p);}
void startTx(){
  if(!transmitting && txRead!=txWrite){
    transmitting=true;
    if(HAL_UART_Transmit_IT(&uart,&tx[txRead],1)!=HAL_OK)transmitting=false;
  }
}
void pin(GPIO_TypeDef *port,uint32_t mask,uint32_t mode,uint32_t alternate=0,uint32_t pull=GPIO_NOPULL){
  GPIO_InitTypeDef p={};p.Pin=mask;p.Mode=mode;p.Pull=pull;
  p.Speed=GPIO_SPEED_FREQ_LOW;p.Alternate=alternate;HAL_GPIO_Init(port,&p);
}
}
namespace Board {
bool clock(){
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  RCC_OscInitTypeDef o={};o.OscillatorType=RCC_OSCILLATORTYPE_HSI;
  o.HSIState=RCC_HSI_ON;o.HSICalibrationValue=RCC_HSICALIBRATION_DEFAULT;
  o.PLL.PLLState=RCC_PLL_ON;o.PLL.PLLSource=RCC_PLLSOURCE_HSI;
  o.PLL.PLLMUL=RCC_PLLMUL_4;o.PLL.PLLDIV=RCC_PLLDIV_2;
  if(HAL_RCC_OscConfig(&o)!=HAL_OK)return false;
  RCC_ClkInitTypeDef c={};c.ClockType=RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  c.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK;c.AHBCLKDivider=RCC_SYSCLK_DIV1;
  c.APB1CLKDivider=RCC_HCLK_DIV1;c.APB2CLKDivider=RCC_HCLK_DIV1;
  return HAL_RCC_ClockConfig(&c,FLASH_LATENCY_1)==HAL_OK;
}
void initializePins(){
  __HAL_RCC_GPIOA_CLK_ENABLE();__HAL_RCC_GPIOB_CLK_ENABLE();__HAL_RCC_SYSCFG_CLK_ENABLE();
  lpd(true);pin(GPIOA,GPIO_PIN_4,GPIO_MODE_OUTPUT_PP);
  pin(GPIOA,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|
      GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15,GPIO_MODE_ANALOG);
  pin(GPIOB,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|
      GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15,GPIO_MODE_ANALOG);
  pin(GPIOA,GPIO_PIN_3,GPIO_MODE_INPUT);pin(GPIOB,GPIO_PIN_0,GPIO_MODE_INPUT);
}
void lpd(bool high){HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,high?GPIO_PIN_SET:GPIO_PIN_RESET);}
bool motionHigh(){return HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==GPIO_PIN_SET;}
bool gpoHigh(){return HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3)==GPIO_PIN_SET;}
bool lpdHigh(){return HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==GPIO_PIN_SET;}
void motionInterrupt(bool enabled){
  if(enabled){pin(GPIOB,GPIO_PIN_0,GPIO_MODE_IT_RISING);HAL_NVIC_SetPriority(EXTI0_1_IRQn,3,0);HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);}
  else {EXTI->IMR&=~GPIO_PIN_0;HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);}
}
void gpoInterrupt(bool enabled){
  if(enabled){pin(GPIOA,GPIO_PIN_3,GPIO_MODE_IT_RISING);HAL_NVIC_SetPriority(EXTI2_3_IRQn,3,0);HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);}
  else {EXTI->IMR&=~GPIO_PIN_3;HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);}
}
bool beginBus(){
  if(bus)return true;
  __HAL_RCC_I2C1_CONFIG(RCC_I2C1CLKSOURCE_PCLK1);
  i2c.Instance=I2C1;
  // Same 32MHz/100kHz timing search inputs as baseline Wire, fixed at build time.
  i2c.Init.Timing=0x10e0474a;
  i2c.Init.AddressingMode=I2C_ADDRESSINGMODE_7BIT;i2c.Init.DualAddressMode=I2C_DUALADDRESS_DISABLE;
  i2c.Init.OwnAddress2Masks=I2C_OA2_NOMASK;i2c.Init.GeneralCallMode=I2C_GENERALCALL_DISABLE;
  i2c.Init.NoStretchMode=I2C_NOSTRETCH_DISABLE;
  if(HAL_I2C_Init(&i2c)!=HAL_OK)return false;
  if(HAL_I2CEx_ConfigAnalogFilter(&i2c,I2C_ANALOGFILTER_ENABLE)!=HAL_OK ||
     HAL_I2CEx_ConfigDigitalFilter(&i2c,0)!=HAL_OK){HAL_I2C_DeInit(&i2c);return false;}
  bus=true;return true;
}
void endBus(){
  if(bus)HAL_I2C_DeInit(&i2c);
  bus=false;pin(GPIOB,GPIO_PIN_6|GPIO_PIN_7,GPIO_MODE_INPUT);__HAL_RCC_I2C1_CLK_DISABLE();
}
bool read(uint8_t a,uint16_t reg,bool wide,uint8_t &v){
  return beginBus() && HAL_I2C_Mem_Read(&i2c,uint16_t(a)<<1,reg,
    wide?I2C_MEMADD_SIZE_16BIT:I2C_MEMADD_SIZE_8BIT,&v,1,100)==HAL_OK;
}
bool readBlock(uint8_t a,uint8_t reg,uint8_t *data,uint8_t length){
  return length && length<=6 && beginBus() && HAL_I2C_Mem_Read(&i2c,uint16_t(a)<<1,
    reg,I2C_MEMADD_SIZE_8BIT,data,length,20)==HAL_OK;
}
bool write(uint8_t a,uint16_t reg,bool wide,uint8_t v){
  return beginBus() && HAL_I2C_Mem_Write(&i2c,uint16_t(a)<<1,reg,
    wide?I2C_MEMADD_SIZE_16BIT:I2C_MEMADD_SIZE_8BIT,&v,1,100)==HAL_OK;
}
bool Console::begin(uint32_t baud){
  uart.Instance=USART2;uart.Init.BaudRate=baud;uart.Init.WordLength=UART_WORDLENGTH_8B;
  uart.Init.StopBits=UART_STOPBITS_1;uart.Init.Parity=UART_PARITY_NONE;
  uart.Init.Mode=UART_MODE_TX_RX;uart.Init.HwFlowCtl=UART_HWCONTROL_NONE;
  uart.Init.OverSampling=UART_OVERSAMPLING_16;uart.Init.OneBitSampling=UART_ONE_BIT_SAMPLE_DISABLE;
  uint32_t p=critical();rxRead=rxWrite=txRead=txWrite=0;transmitting=false;release(p);
  serial=HAL_UART_Init(&uart)==HAL_OK;
  if(serial)serial=HAL_UART_Receive_IT(&uart,&rxByte,1)==HAL_OK;
  return serial;
}
bool Console::flush(){
  uint32_t start=HAL_GetTick();
  while(txRead!=txWrite || transmitting || !__HAL_UART_GET_FLAG(&uart,UART_FLAG_TC)){
    if(uint32_t(HAL_GetTick()-start)>=100)return false;
  }
  return true;
}
void Console::end(){HAL_UART_DeInit(&uart);serial=false;}
unsigned Console::available()const{return (rxWrite+64-rxRead)%64;}
int Console::read(){
  uint32_t p=critical();int value=-1;
  if(rxRead!=rxWrite){value=rx[rxRead];rxRead=(rxRead+1)%64;}release(p);return value;
}
unsigned Console::availableForWrite()const{return (txRead+64-txWrite-1)%64;}
void Console::write(uint8_t value){
  uint32_t p=critical();uint8_t next=(txWrite+1)%64;
  if(next!=txRead){tx[txWrite]=value;txWrite=next;startTx();}release(p);
}
}
extern "C" void HAL_MspInit(){__HAL_RCC_SYSCFG_CLK_ENABLE();__HAL_RCC_PWR_CLK_ENABLE();}
extern "C" void HAL_UART_MspInit(UART_HandleTypeDef *h){
  if(h->Instance!=USART2)return;
  __HAL_RCC_USART2_CONFIG(RCC_USART2CLKSOURCE_PCLK1);__HAL_RCC_USART2_CLK_ENABLE();
  pin(GPIOA,GPIO_PIN_9|GPIO_PIN_10,GPIO_MODE_AF_PP,GPIO_AF4_USART2,GPIO_PULLUP);
  HAL_NVIC_SetPriority(USART2_IRQn,3,0);HAL_NVIC_EnableIRQ(USART2_IRQn);
}
extern "C" void HAL_UART_MspDeInit(UART_HandleTypeDef *h){
  if(h->Instance!=USART2)return;
  HAL_NVIC_DisableIRQ(USART2_IRQn);__HAL_RCC_USART2_CLK_DISABLE();
  pin(GPIOA,GPIO_PIN_9|GPIO_PIN_10,GPIO_MODE_ANALOG);
}
extern "C" void HAL_I2C_MspInit(I2C_HandleTypeDef *h){
  if(h->Instance!=I2C1)return;
  __HAL_RCC_I2C1_CLK_ENABLE();pin(GPIOB,GPIO_PIN_6|GPIO_PIN_7,GPIO_MODE_AF_OD,GPIO_AF1_I2C1);
}
extern "C" void HAL_I2C_MspDeInit(I2C_HandleTypeDef *h){
  if(h->Instance==I2C1)__HAL_RCC_I2C1_CLK_DISABLE();
}
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *h){
  if(h!=&uart)return;
  uint8_t next=(rxWrite+1)%64;
  if(next!=rxRead){rx[rxWrite]=rxByte;__DMB();rxWrite=next;}
  HAL_UART_Receive_IT(&uart,&rxByte,1);
}
extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *h){
  if(h!=&uart)return;
  txRead=(txRead+1)%64;transmitting=false;startTx();
}
extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef *h){
  if(h==&uart && serial && h->RxState==HAL_UART_STATE_READY)HAL_UART_Receive_IT(h,&rxByte,1);
}
extern "C" void USART2_IRQHandler(){HAL_UART_IRQHandler(&uart);}
extern "C" void SysTick_Handler(){HAL_IncTick();}
extern "C" void SystemClock_Config(){
  if(!Board::clock()){Board::lpd(true);__disable_irq();while(true)__WFI();}
}
extern "C" __attribute__((used,externally_visible)) void *_sbrk(ptrdiff_t){errno=ENOMEM;return reinterpret_cast<void *>(-1);}
extern "C" void _init(){}
extern "C" void _fini(){}
