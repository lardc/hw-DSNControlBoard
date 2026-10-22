#include "InitConfig.h"
#include "Board.h"
#include "SysConfig.h"
#include "BCCIxParams.h"
#include "ZwUSART.h"
#include "OneWire.h"
#include "DS18B20.h"
#include "DS2431.h"

// Functions
//
Boolean INITCFG_ConfigSystemClock()
{
	return RCC_PLL_HSE_Config(QUARTZ_FREQUENCY, PREDIV_4, PLL_14);
}
//------------------------------------------------

void INITCFG_ConfigIO()
{
	RCC_GPIO_Clk_EN(PORTA);
	RCC_GPIO_Clk_EN(PORTB);

	// Push-pull выходы
	GPIO_InitPushPullOutput(GPIO_SFT_IGNORE);
	GPIO_InitPushPullOutput(GPIO_ST_DISCHARGE);
	GPIO_InitPushPullOutput(GPIO_ST_START);
	GPIO_InitPushPullOutput(GPIO_FAN);
	GPIO_InitPushPullOutput(GPIO_SCOPE_SYNC);
	GPIO_InitPushPullOutput(GPIO_LED);
	GPIO_InitPushPullOutput(GPIO_FP_LED);
	GPIO_InitPushPullOutput(GPIO_DQ_CTRL);

	// Open-drain выходы (ТТ)
	GPIO_InitOpenDrainOutput(GPIO_DQ_PWR, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_RCK_EXP, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_OE_EXP, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_GATE_CTRL_H, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_GATE_CTRL_L, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_SPI_SCK_MUX, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_RCK_H, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_RCK_L, NoPull);

	// Входы
	GPIO_InitInput(GPIO_SFT_IN, NoPull);
	GPIO_InitInput(GPIO_DQ_IN, NoPull);

	// Аналог диагностики
	GPIO_InitAnalog(GPIO_ADC_DIAG);

	// Начальные уровни по ТТ
	GPIO_SetState(GPIO_SFT_IGNORE, false);
	GPIO_SetState(GPIO_ST_DISCHARGE, false);
	GPIO_SetState(GPIO_ST_START, false);
	GPIO_SetState(GPIO_DQ_PWR, true);
	GPIO_SetState(GPIO_DQ_CTRL, false);
	GPIO_SetState(GPIO_RCK_EXP, false);
	GPIO_SetState(GPIO_OE_EXP, true);
	GPIO_SetState(GPIO_GATE_CTRL_H, false);
	GPIO_SetState(GPIO_GATE_CTRL_L, false);
	GPIO_SetState(GPIO_SPI_SCK_MUX, true);
	GPIO_SetState(GPIO_RCK_H, false);
	GPIO_SetState(GPIO_RCK_L, false);
	GPIO_SetState(GPIO_FAN, false);
	GPIO_SetState(GPIO_SCOPE_SYNC, false);
	GPIO_SetState(GPIO_LED, false);
	GPIO_SetState(GPIO_FP_LED, false);

	// Альтернативные функции
	GPIO_InitAltFunction(GPIO_ALT_UART3_TX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART3_RX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_CAN1_RX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_CAN1_TX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_SPI3_SCK, AltFn_6);
	GPIO_InitAltFunction(GPIO_ALT_SPI3_MOSI, AltFn_6);
	GPIO_InitAltFunction(GPIO_ALT_TIM2_CH2, AltFn_1);
}
//------------------------------------------------

void INITCFG_ConfigCAN()
{
	RCC_CAN_Clk_EN(CAN_1_ClkEN);
	NCAN_Init(SYSCLK, CAN_BAUDRATE, FALSE);
	NCAN_FIFOInterrupt(TRUE);
	NCAN_FilterInit(0, CAN_SLAVE_FILTER_ID, CAN_MASTER_FILTER_ID);
}
//------------------------------------------------

void INITCFG_ConfigUART()
{
	USARTx_Init(USART3, SYSCLK, USART_BAUDRATE);
	USARTx_RecieveInterrupt(USART3, true);
}
//------------------------------------------------

void INITCFG_ConfigTimer7()
{
	TIM_Clock_En(TIM_7);
	TIM_Config(TIM7, SYSCLK, TIMER7_uS);
	TIM_Interupt(TIM7, 2, true);
	TIM_Start(TIM7);
}
//------------------------------------------------

void INITCFG_ConfigTimer2()
{
	TIM_Clock_En(TIM_2);

	TIM2->PSC = (uint32_t)(SYSCLK / TIMER2_TICK_HZ) - 1;
	TIM2->ARR = 0xFFFF;
	TIM2->EGR |= TIM_EGR_UG;
	TIM2->SR = 0;

	TIM2->CCMR1 &= ~(TIM_CCMR1_CC2S | TIM_CCMR1_IC2F);
	TIM2->CCMR1 |= TIM_CCMR1_CC2S_0;
	TIM2->CCER &= ~(TIM_CCER_CC2P | TIM_CCER_CC2NP);
	TIM2->CCER |= TIM_CCER_CC2E;

	TIM_Start(TIM2);
}
//------------------------------------------------

void INITCFG_ConfigWatchDog()
{
	IWDG_Config();
	IWDG_ConfigureSlowUpdate();
}
//------------------------------------------------

void INITCFG_ConfigADC()
{
	RCC_ADC_Clk_EN(ADC_12_ClkEN);

	ADC_Calibration(ADC2);
	ADC_SoftTrigConfig(ADC2);
	ADC_Enable(ADC2);
}
//-----------------------------------------------

void INITCFG_ConfigSPI()
{
	SPI_Init8b(SPI3, SPI3_BAUDRATE_BITS, SPI3_LSB_FIRST);
}
//-----------------------------------------------

void INITCFG_ConfigOneWire()
{
	OneWireBus Config;

	Config.writePin = GPIO_DQ_CTRL;
	Config.readPin = GPIO_DQ_IN;
	Config.powerPin = GPIO_DQ_PWR;
	Config.useSinglePin = false;
	Config.hasPowerPin = true;
	Config.invertWrite = true;
	Config.invertPower = true;
	OneWire_Init(Config);

	DS18B20_Init(Config.hasPowerPin);
	DS2431_Init();
}
//-----------------------------------------------
