// Include
#include "Interrupts.h"
//
#include "Controller.h"
#include "LowLevel.h"
#include "Board.h"
#include "SysConfig.h"
#include "Global.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "ZwUSART.h"

// Functions
//
void USART3_IRQHandler()
{
	if(USARTx_RecieveCheck(USART3))
	{
		USARTx_RegisterToFIFO(USART3);
		USARTx_OverrunFlagClear(USART3);
	}
}
//-----------------------------------------

void USB_LP_CAN_RX0_IRQHandler()
{
	if(NCAN_RecieveCheck())
	{
		NCAN_RecieveData();
		NCAN_RecieveFlagReset();
	}
}
//-----------------------------------------

void TIM7_IRQHandler()
{
	static uint16_t LED_BlinkTimeCounter = 0;

	if(TIM_StatusCheck(TIM7))
	{
		CONTROL_TimeCounter++;
		if(++LED_BlinkTimeCounter > TIME_LED_BLINK)
		{
			LL_ToggleBoardLED();
			LED_BlinkTimeCounter = 0;
		}

		CONTROL_SafetyIrqTick();
		CONTROL_HandleFrontPanelLamp(false);

		TIM_StatusClear(TIM7);
	}
}
//-----------------------------------------
