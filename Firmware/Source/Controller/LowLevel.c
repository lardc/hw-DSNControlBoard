// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "Global.h"
#include "SysConfig.h"
#include "ZwTIM.h"
#include "ZwADC.h"

// Forward
//
static void LL_PulseRck(GPIO_PortPinSetting Rck);
static Boolean LL_CaptureWaitEdge(volatile Int64U *pTimeMs, Boolean Falling, Int32U TimeoutMs, pInt16U pStamp);

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

static void LL_PulseRck(GPIO_PortPinSetting Rck)
{
	DELAY_US(TIME_SPI_DELAY_US);
	GPIO_SetState(Rck, true);
	DELAY_US(TIME_SPI_DELAY_US);
	GPIO_SetState(Rck, false);
	DELAY_US(TIME_SPI_DELAY_US);
}
//-----------------------------

void LL_SPI_LatchRckH()
{
	LL_PulseRck(GPIO_RCK_H);
}
//-----------------------------

void LL_SPI_LatchRckL()
{
	LL_PulseRck(GPIO_RCK_L);
}
//-----------------------------

void LL_SPI_LatchRckExp()
{
	LL_PulseRck(GPIO_RCK_EXP);
}
//-----------------------------

void LL_SetSpiSckMux(bool ToDac)
{
	GPIO_SetState(GPIO_SPI_SCK_MUX, ToDac);
}
//-----------------------------

void LL_SetGateCtrlH(bool State)
{
	GPIO_SetState(GPIO_GATE_CTRL_H, State);
}
//-----------------------------

void LL_SetGateCtrlL(bool State)
{
	GPIO_SetState(GPIO_GATE_CTRL_L, State);
}
//-----------------------------

void LL_SetFan(bool State)
{
	GPIO_SetState(GPIO_FAN, State);
}
//-----------------------------

void LL_SetScopeSync(bool State)
{
	GPIO_SetState(GPIO_SCOPE_SYNC, State);
}
//-----------------------------

void LL_SetOeExp(bool OutputsEnabled)
{
	// OE_EXP инвертированный: 1 = запрет выходов
	GPIO_SetState(GPIO_OE_EXP, !OutputsEnabled);
}
//-----------------------------

void LL_SetStDischarge(bool State)
{
	GPIO_SetState(GPIO_ST_DISCHARGE, State);
}
//-----------------------------

void LL_SetStStart(bool State)
{
	GPIO_SetState(GPIO_ST_START, State);
}
//-----------------------------

bool LL_IsSafetyTrig()
{

	return !GPIO_GetState(GPIO_SFT_IN);
}
//-----------------------------

void LL_SafetyIgnore(bool Ignore)
{
	GPIO_SetState(GPIO_SFT_IGNORE, Ignore);
}
//-----------------------------

void LL_Safety()
{
	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);
}
//-----------------------------

float LL_MeasureAdcDiag()
{
	float RawMv = (float)ADC_Measure(ADC2, ADC_DIAG_CHANNEL) * ADC_REF_VOLTAGE / ADC_RESOLUTION;
	return RawMv;
}
//-----------------------------

static Boolean LL_CaptureWaitEdge(volatile Int64U *pTimeMs, Boolean Falling, Int32U TimeoutMs, pInt16U pStamp)
{
	Int64U Deadline;

	if(!pTimeMs)
		return FALSE;

	if(Falling)
		TIM2->CCER |= TIM_CCER_CC2P;
	else
		TIM2->CCER &= ~TIM_CCER_CC2P;

	TIM2->SR &= ~(TIM_SR_CC2IF | TIM_SR_CC2OF);

	Deadline = *pTimeMs + (TimeoutMs ? TimeoutMs : 1);

	while(!(TIM2->SR & TIM_SR_CC2IF))
	{
		if(*pTimeMs >= Deadline)
			return FALSE;
	}

	*pStamp = (Int16U)TIM2->CCR2;
	TIM2->SR &= ~(TIM_SR_CC2IF | TIM_SR_CC2OF);
	return TRUE;
}
//-----------------------------

Boolean LL_CaptureMeasureHighPulseUs(volatile Int64U *pTimeMs, Int32U TimeoutMs, pInt32U pWidthUs)
{
	Int16U tRise, tFall;

	if(!pWidthUs || !pTimeMs)
		return FALSE;

	if(!LL_CaptureWaitEdge(pTimeMs, FALSE, TimeoutMs, &tRise))
		return FALSE;

	if(!LL_CaptureWaitEdge(pTimeMs, TRUE, TimeoutMs, &tFall))
		return FALSE;

	// TIM2 = 1 МГц → разница счётчика = мкс (для импульса < 65536 мкс)
	*pWidthUs = (Int32U)((Int16U)(tFall - tRise));
	return TRUE;
}
//-----------------------------

void LL_CaptureArmEdge(Boolean Falling)
{
	if(Falling)
		TIM2->CCER |= TIM_CCER_CC2P;
	else
		TIM2->CCER &= ~TIM_CCER_CC2P;

	TIM2->SR &= ~(TIM_SR_CC2IF | TIM_SR_CC2OF);
}
//-----------------------------

Boolean LL_CaptureEdgeReady(pInt16U pStamp)
{
	if(!(TIM2->SR & TIM_SR_CC2IF))
		return FALSE;

	if(pStamp)
		*pStamp = (Int16U)TIM2->CCR2;

	TIM2->SR &= ~(TIM_SR_CC2IF | TIM_SR_CC2OF);
	return TRUE;
}
//-----------------------------
