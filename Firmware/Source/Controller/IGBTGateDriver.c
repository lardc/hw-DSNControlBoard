// Header
#include "IGBTGateDriver.h"

// Include
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Global.h"
#include "LowLevel.h"
#include "math.h"

// Variables
static const float IGBTGD_Rbits[8] = {200.0f, 100.0f, 51.0f, 24.0f, 12.0f, 6.2f, 3.0f, 1.5f}; // Сопротивления Rg Ом
static const float IGBTGD_Rs = 15.0f;

// Forward
//
static float IGBTGD_CodeToResistance(Int8U Code);
static Int8U IGBTGD_BlockedBits(Int8U Code);
static Int16U IGBTGD_ComposeBase(Int8U RgCode, bool MuxOn, bool FaultLed);
static float IGBTGD_GetOpenK(IGBTGD_Channel Channel);
static float IGBTGD_GetCloseK(IGBTGD_Channel Channel);
static void IGBTGD_ApplyBase(IGBTGD_Channel Channel, Int16U Base);

// Functions
//
static Int8U IGBTGD_BlockedBits(Int8U Code)
{
	if(Code >= 128)
		return 0x0F;
	if(Code >= 64)
		return 0x07;
	if(Code >= 32)
		return 0x03;
	if(Code >= 16)
		return 0x01;
	return 0;
}
//-----------------------------------------------

static float IGBTGD_CodeToResistance(Int8U Code)
{
	float Conductance = 1.0f / IGBTGD_Rs;
	Int8U i;

	for(i = 0; i < 8; ++i)
	{
		if(Code & (1u << i))
			Conductance += 1.0f / IGBTGD_Rbits[i];
	}

	return 1.0f / Conductance;
}
//-----------------------------------------------

Int8U IGBTGD_FindRgCode(float RgOhm)
{
	Int16U BestCode = 0;
	float BestErr = 1e9f;
	Int16U Code;

	if(RgOhm <= 0.0f)
		return 0;

	for(Code = 0; Code < 256; ++Code)
	{
		Int8U RgCode = (Int8U)Code;
		float Resistance, RelError;

		if(RgCode & IGBTGD_BlockedBits(RgCode))
			continue;

		Resistance = IGBTGD_CodeToResistance(RgCode);
		RelError = fabsf(Resistance - RgOhm) / RgOhm;
		if(RelError < BestErr)
		{
			BestErr = RelError;
			BestCode = RgCode;
		}
	}

	return (Int8U)BestCode;
}
//-----------------------------------------------

Int16U IGBTGD_VoltageToDac(float VoltageAbs, float Coef)
{
	float DacCode;

	if(VoltageAbs < 0.0f)
		VoltageAbs = -VoltageAbs;

	// OpenV/CloseV — в вольтах; DAC_VREF — в мВ
	DacCode = DAC_RESOLUTION_16 * (VoltageAbs * 1000.0f) * Coef / DAC_VREF;
	if(DacCode > DAC_RESOLUTION_16)
		DacCode = DAC_RESOLUTION_16;
	if(DacCode < 0.0f)
		DacCode = 0.0f;

	return (Int16U)(DacCode + 0.5f);
}
//-----------------------------------------------

static float IGBTGD_GetOpenK(IGBTGD_Channel Channel)
{
	return (Channel == IGBTGD_CH_H) ? DataTable[REG_DRVH_OPEN_K] : DataTable[REG_DRVL_OPEN_K];
}
//-----------------------------------------------

static float IGBTGD_GetCloseK(IGBTGD_Channel Channel)
{
	return (Channel == IGBTGD_CH_H) ? DataTable[REG_DRVH_CLOSE_K] : DataTable[REG_DRVL_CLOSE_K];
}
//-----------------------------------------------

static Int16U IGBTGD_ComposeBase(Int8U RgCode, bool MuxOn, bool FaultLed)
{
	Int16U Mask = (Int16U)RgCode | ZCRD_CS_IDLE;

	if(MuxOn)
		Mask |= ZCRD_BIT_MUX;
	if(FaultLed)
		Mask |= ZCRD_BIT_FAULT_LED;

	return Mask;
}
//-----------------------------------------------

static void IGBTGD_ApplyBase(IGBTGD_Channel Channel, Int16U Base)
{
	ZcRD_SetDriverBaseMask(Channel, Base);
	ZcRD_DeselectDacs(Channel);
}
//-----------------------------------------------

void IGBTGD_SetVoltages(IGBTGD_Channel Channel, float OpenV, float CloseV)
{
	Int16U DacOpen = IGBTGD_VoltageToDac(OpenV, IGBTGD_GetOpenK(Channel));
	Int16U DacClose = IGBTGD_VoltageToDac(CloseV, IGBTGD_GetCloseK(Channel));

	// CS1 ↓ → запись ЦАП1 (отпирающее)
	ZcRD_SelectDac1(Channel);
	ZcRD_WriteDacCode((Int32U)DacOpen);
	// CS1 ↑, CS2 ↓ → запись ЦАП2 (запирающее)
	ZcRD_SelectDac2(Channel);
	ZcRD_WriteDacCode((Int32U)DacClose);
	// CS2 ↑
	ZcRD_DeselectDacs(Channel);
}
//-----------------------------------------------

void IGBTGD_SetGateResistance(IGBTGD_Channel Channel, float RgOhm)
{
	Int16U Mask = ZcRD_GetDriverMask(Channel);
	Int8U RgCode = IGBTGD_FindRgCode(RgOhm);

	Mask = (Mask & ~ZCRD_RG_MASK) | RgCode | ZCRD_CS_IDLE;
	Mask &= ~(ZCRD_BIT_CS1 | ZCRD_BIT_CS2);
	Mask |= ZCRD_CS_IDLE;
	IGBTGD_ApplyBase(Channel, Mask);
}
//-----------------------------------------------

void IGBTGD_SetMux(IGBTGD_Channel Channel, bool ToSwitchBoard)
{
	Int16U Mask = ZcRD_GetDriverMask(Channel);

	if(ToSwitchBoard)
		Mask |= ZCRD_BIT_MUX;
	else
		Mask &= ~ZCRD_BIT_MUX;

	IGBTGD_ApplyBase(Channel, Mask);
}
//-----------------------------------------------

void IGBTGD_SetFaultLed(IGBTGD_Channel Channel, bool On)
{
	Int16U Mask = ZcRD_GetDriverMask(Channel);

	if(On)
		Mask |= ZCRD_BIT_FAULT_LED;
	else
		Mask &= ~ZCRD_BIT_FAULT_LED;

	IGBTGD_ApplyBase(Channel, Mask);
}
//-----------------------------------------------

void IGBTGD_Configure(IGBTGD_Channel Channel, float OpenV, float CloseV, float RgOhm,
		bool MuxOn, bool FaultLed)
{
	Int8U RgCode = IGBTGD_FindRgCode(RgOhm);
	Int16U Base = IGBTGD_ComposeBase(RgCode, MuxOn, FaultLed);

	IGBTGD_ApplyBase(Channel, Base);
	IGBTGD_SetVoltages(Channel, OpenV, CloseV);
}
//-----------------------------------------------

void IGBTGD_Reset(IGBTGD_Channel Channel)
{
	IGBTGD_ApplyBase(Channel, ZCRD_CS_IDLE);
	IGBTGD_SetVoltages(Channel, 0.0f, 0.0f);
	IGBTGD_ApplyBase(Channel, ZCRD_CS_IDLE);
}
//-----------------------------------------------

void IGBTGD_ResetAll()
{
	IGBTGD_Reset(IGBTGD_CH_H);
	IGBTGD_Reset(IGBTGD_CH_L);
	ZcRD_WriteExpMask(0);
	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);
}
//-----------------------------------------------
