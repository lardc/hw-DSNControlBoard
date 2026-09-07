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
	SPI_WriteByte8b(SPI3, Data);
}
//-----------------------------

void LL_Safety(bool Enable)
{
	if(Enable)
	{
		GPIO_SetState(GPIO_GATE_CTRL_H, false);
		GPIO_SetState(GPIO_GATE_CTRL_L, false);
	}
}
//-----------------------------

bool LL_IsSafetyTrig()
{
	return GPIO_GetState(GPIO_SFT_IN);
}
//-----------------------------
