// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "Global.h"
#include "DataTable.h"

// Functions
//
void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------

void LL_SetStateFPLed(bool State)
{
	GPIO_SetState(GPIO_FP_LED, State);
}
//-----------------------------

void LL_ToggleFPLed()
{
	GPIO_Toggle(GPIO_FP_LED);
}
//-----------------------------

void LL_SPI_WriteByte(Int8U Data)
{
	SPI_WriteByte8b(SPI1, Data);
}
//-----------------------------

void LL_SPI_LatchBoardTemplate(Int8U BoardIdx, Int32U PulseLen)
{
	GPIO_PortPinSetting SS;
	switch(BoardIdx)
	{
		case 0:  SS = GPIO_SPI_SS1; break;
		case 1:  SS = GPIO_SPI_SS2; break;
		case 2:  SS = GPIO_SPI_SS3; break;
	}

	DELAY_US(TIME_SPI_DELAY_US);
	GPIO_SetState(SS, false);
	(PulseLen > 1000) ? DELAY_MS(PulseLen / 1000) : DELAY_US(PulseLen);
	GPIO_SetState(SS, true);
	DELAY_US(TIME_SPI_DELAY_US);
}
//-----------------------------

void LL_SPI_LatchBoard(Int8U BoardIdx)
{
	LL_SPI_LatchBoardTemplate(BoardIdx, TIME_SPI_DELAY_US);
}
//-----------------------------

void LL_SPI_TestSS(Int8U BoardIdx)
{
	LL_SPI_LatchBoardTemplate(BoardIdx, 100000);
}
//-----------------------------

void LL_SafetyForceRelaysOff(bool Enable)
{
	// OpenDrain: false = активное притягивание к GND, true = high-Z
	GPIO_SetState(GPIO_SFT_ENABLE, Enable);
}
//-----------------------------

void LL_SelfTestCurrentEnable(bool State)
{
	GPIO_SetState(GPIO_SD_EN, State);
}
//-----------------------------

float GetTestVoltage()
{
	return (float)ADC_Measure(ADC1, ADC_V_CHANNEL) * ADC_REF_VOLTAGE / ADC_RESOLUTION;
}
//-----------------------------

bool LL_IsSafetyTrig()
{
	return GPIO_GetState(GPIO_SFT_IN);
}
//-----------------------------
