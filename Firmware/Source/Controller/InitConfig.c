#include "InitConfig.h"
#include "Board.h"
#include "SysConfig.h"
#include "BCCIxParams.h"

// Functions
//
Boolean INITCFG_ConfigSystemClock()
{
	return RCC_PLL_HSE_Config(QUARTZ_FREQUENCY, PREDIV_4, PLL_14);
}
//------------------------------------------------

void INITCFG_ConfigIO()
{
	// Включение тактирования портов
	RCC_GPIO_Clk_EN(PORTA);
	RCC_GPIO_Clk_EN(PORTB);
	RCC_GPIO_Clk_EN(PORTC);

	// Выходы (PushPull)
	GPIO_InitPushPullOutput(GPIO_LED);
	GPIO_InitPushPullOutput(GPIO_FP_LED);
	GPIO_InitPushPullOutput(GPIO_SD_EN);

	// Выходы CS сдвиговых регистров (OpenDrain, внешняя подтяжка)
	GPIO_InitOpenDrainOutput(GPIO_SPI_SS1, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_SPI_SS2, NoPull);
	GPIO_InitOpenDrainOutput(GPIO_SPI_SS3, NoPull);

	// Управление OE: OpenDrain, при активации контура безопасности уходит в high-Z (на плате подтяжка к +5В запирает OE)
	GPIO_InitOpenDrainOutput(GPIO_SFT_ENABLE, NoPull);

	// Входы
	GPIO_InitInput(GPIO_SFT_IN, NoPull);
	// Для временного теста: перемычка pin 17 (PA7) → pin 40 (PB4).
	GPIO_InitInput(GPIO_SPI_TEST_INPUT, NoPull);

	// Начальная установка состояний выводов
	GPIO_SetState(GPIO_LED, false);
	GPIO_SetState(GPIO_FP_LED, false);
	GPIO_SetState(GPIO_SD_EN, false);
	//
	GPIO_SetState(GPIO_SPI_SS1, true);
	GPIO_SetState(GPIO_SPI_SS2, true);
	GPIO_SetState(GPIO_SPI_SS3, true);
	//
	// SFT_ENABLE = false → пин притянут к земле (OE разрешён, коммутация активна)
	GPIO_SetState(GPIO_SFT_ENABLE, false);

	// Альтернативные функции
	GPIO_InitAltFunction(GPIO_ALT_UART1_RX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART1_TX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_CAN1_RX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_CAN1_TX, AltFn_9);
	//
	GPIO_InitAltFunction(GPIO_ALT_SPI1_SCLK, AltFn_5);
	GPIO_InitAltFunction(GPIO_ALT_SPI1_MOSI, AltFn_5);
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
	USART_Init(USART1, SYSCLK, USART_BAUDRATE);
	USART_Recieve_Interupt(USART1, 0, true);
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

void INITCFG_ConfigWatchDog()
{
	IWDG_Config();
	IWDG_ConfigureSlowUpdate();
}
//------------------------------------------------

void INITCFG_ConfigADC()
{
	RCC_ADC_Clk_EN(ADC_12_ClkEN);

	// ADC1
	ADC_Calibration(ADC1);
	ADC_SoftTrigConfig(ADC1);
	ADC_Enable(ADC1);
}
//-----------------------------------------------

void INITCFG_ConfigSPI()
{
	SPI_Init8b(SPI1, SPI1_BAUDRATE_BITS, SPI1_LSB_FIRST);
}
//-----------------------------------------------
