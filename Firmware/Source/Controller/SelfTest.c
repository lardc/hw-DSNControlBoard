// Header
#include "SelfTest.h"
//
#include "Controller.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Constraints.h"
#include "IGBTGateDriver.h"
#include "ZcRegistersDriver.h"
#include "LowLevel.h"
#include "Global.h"
#include "Delay.h"

// Variables
//
SelfTestSubState SELFTEST_SubState = STS_None;
static Int8U RgIndex = 0;
static DeviceState StateAfter = DS_Enabled;

// Forward
//
static void SELFTEST_Fail(Int16U Problem);
static void SELFTEST_Succeed();
static Boolean SELFTEST_DurationOk(Int32U PulseDuration, Int32U ExpectedDuration, Boolean CompareExpected);
static Boolean SELFTEST_RunDriverCheck(IGBTGD_Channel Channel, Boolean ToSwitchBoard, float OpenV, float CloseV,
		float RgOhm, pInt32U pPulseDuration);

// Functions
//
static void SELFTEST_Fail(Int16U Problem)
{
	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);
	ZcRD_WriteExpMask(0);
	IGBTGD_ResetAll();

	SELFTEST_SubState = STS_None;
	CONTROL_SwitchToFault(DF_SELF_TEST);
	DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_FAIL;
	DataTable[REG_PROBLEM] = Problem;
}
//-----------------------------------------------

static void SELFTEST_Succeed()
{
	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);
	ZcRD_WriteExpMask(0);
	IGBTGD_ResetAll();

	SELFTEST_SubState = STS_None;
	DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_OK;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	CONTROL_SetDeviceState(StateAfter);
}
//-----------------------------------------------

static Boolean SELFTEST_DurationOk(Int32U PulseDuration, Int32U ExpectedDuration, Boolean CompareExpected)
{
	Int32U Delta;
	Int32U MaxDuration = (Int32U)DataTable[REG_ST_PULSE_MAX_DURATION];
	Int32U Tol = (Int32U)DataTable[REG_ST_RG_DURATION_TOL];

	if(PulseDuration == 0 || PulseDuration > MaxDuration)
		return false;

	if(!CompareExpected)
		return true;

	Delta = (PulseDuration > ExpectedDuration) ? (PulseDuration - ExpectedDuration) : (ExpectedDuration - PulseDuration);
	return (Delta <= Tol);
}
//-----------------------------------------------

static Boolean SELFTEST_RunDriverCheck(IGBTGD_Channel Channel, Boolean ToSwitchBoard, float OpenV, float CloseV,
		float RgOhm, pInt32U pPulseDuration)
{
	Int32U PulseDuration = 0;
	Int32U CaptureTimeoutMs = (Int32U)DataTable[REG_ST_CAPTURE_TIMEOUT];

	IGBTGD_Configure(Channel, OpenV, CloseV, RgOhm, ToSwitchBoard, false);
	DELAY_MS((Int32U)DataTable[REG_ST_CHARGE_TIME]);

	if(Channel == IGBTGD_CH_H)
	{
		LL_SetGateCtrlL(false);
		LL_SetGateCtrlH(true);
	}
	else
	{
		LL_SetGateCtrlH(false);
		LL_SetGateCtrlL(true);
	}

	if(!LL_CaptureMeasureHighPulseUs(&CONTROL_TimeCounter, CaptureTimeoutMs, &PulseDuration))
	{
		LL_SetGateCtrlH(false);
		LL_SetGateCtrlL(false);
		if(pPulseDuration)
			*pPulseDuration = 0;
		return false;
	}

	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);

	if(pPulseDuration)
		*pPulseDuration = PulseDuration;

	return true;
}
//-----------------------------------------------

void SELFTEST_Stop()
{
	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);
	ZcRD_WriteExpMask(0);
	SELFTEST_SubState = STS_None;
}
//-----------------------------------------------

void SELFTEST_Start()
{
	DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_SUB_STATE] = 0;
	RgIndex = 0;
	ZcRD_WriteExpMask(0);

	StateAfter = (CONTROL_State == DS_SafetyActive) ? DS_SafetyActive : DS_Enabled;
	CONTROL_SetDeviceState(DS_InSelfTest);
	SELFTEST_SubState = STS_ProtectionBoard_H_DUT;
}
//-----------------------------------------------

void SELFTEST_Process()
{
	Int32U PulseDuration = 0;
	Boolean Ok;
	float OpenV = DataTable[REG_ST_OPEN_V];
	float CloseV = DataTable[REG_ST_CLOSE_V];

	if(SELFTEST_SubState == STS_None)
		return;

	DataTable[REG_SUB_STATE] = (float)SELFTEST_SubState;

	switch(SELFTEST_SubState)
	{
		case STS_ProtectionBoard_H_DUT:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, true);
			Ok = SELFTEST_RunDriverCheck(IGBTGD_CH_H, false, OpenV, CloseV, IGBTGD_RgBitOhm[0], &PulseDuration);
			ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, false);
			if(!Ok || !SELFTEST_DurationOk(PulseDuration, 0, false))
				SELFTEST_Fail(PROBLEM_SELFTEST);
			else
				SELFTEST_SubState = STS_ProtectionBoard_L_DUT;
			break;

		case STS_ProtectionBoard_L_DUT:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, true);
			Ok = SELFTEST_RunDriverCheck(IGBTGD_CH_L, false, OpenV, CloseV, IGBTGD_RgBitOhm[0], &PulseDuration);
			ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, false);
			if(!Ok || !SELFTEST_DurationOk(PulseDuration, 0, false))
				SELFTEST_Fail(PROBLEM_SELFTEST);
			else
				SELFTEST_SubState = STS_ProtectionBoard_H_SwitchBoard;
			break;

		case STS_ProtectionBoard_H_SwitchBoard:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_HE_BIT, true);
			Ok = SELFTEST_RunDriverCheck(IGBTGD_CH_H, true, OpenV, CloseV, IGBTGD_RgBitOhm[0], &PulseDuration);
			ZcRD_SetExpBit(ZCRD_ST_RELAY_HE_BIT, false);
			IGBTGD_SetMux(IGBTGD_CH_H, false);
			if(!Ok || !SELFTEST_DurationOk(PulseDuration, 0, false))
				SELFTEST_Fail(PROBLEM_SELFTEST);
			else
				SELFTEST_SubState = STS_ProtectionBoard_L_SwitchBoard;
			break;

		case STS_ProtectionBoard_L_SwitchBoard:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_LE_BIT, true);
			Ok = SELFTEST_RunDriverCheck(IGBTGD_CH_L, true, OpenV, CloseV, IGBTGD_RgBitOhm[0], &PulseDuration);
			ZcRD_SetExpBit(ZCRD_ST_RELAY_LE_BIT, false);
			IGBTGD_SetMux(IGBTGD_CH_L, false);
			if(!Ok || !SELFTEST_DurationOk(PulseDuration, 0, false))
				SELFTEST_Fail(PROBLEM_SELFTEST);
			else
			{
				RgIndex = 0;
				SELFTEST_SubState = STS_GateResistors_H;
			}
			break;

		case STS_GateResistors_H:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, true);
			Ok = SELFTEST_RunDriverCheck(IGBTGD_CH_H, false, OpenV, CloseV, IGBTGD_RgBitOhm[RgIndex], &PulseDuration);
			if(!Ok || !SELFTEST_DurationOk(PulseDuration, (Int32U)DataTable[REG_ST_RG_0_DURATION + RgIndex], true))
			{
				ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, false);
				SELFTEST_Fail(PROBLEM_SELFTEST);
				break;
			}
			RgIndex++;
			if(RgIndex >= IGBTGD_RG_BIT_COUNT)
			{
				ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, false);
				RgIndex = 0;
				SELFTEST_SubState = STS_GateResistors_L;
			}
			break;

		case STS_GateResistors_L:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, true);
			Ok = SELFTEST_RunDriverCheck(IGBTGD_CH_L, false, OpenV, CloseV, IGBTGD_RgBitOhm[RgIndex], &PulseDuration);
			if(!Ok || !SELFTEST_DurationOk(PulseDuration, (Int32U)DataTable[REG_ST_RG_0_DURATION + RgIndex], true))
			{
				ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, false);
				SELFTEST_Fail(PROBLEM_SELFTEST);
				break;
			}
			RgIndex++;
			if(RgIndex >= IGBTGD_RG_BIT_COUNT)
			{
				ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, false);
				SELFTEST_SubState = STS_Amplitude_H_PM20;
			}
			break;

		case STS_Amplitude_H_PM20:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, true);
			Ok = SELFTEST_RunDriverCheck(IGBTGD_CH_H, false, 20.0f, -20.0f, IGBTGD_RgBitOhm[0], &PulseDuration);
			ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, false);
			if(!Ok || !SELFTEST_DurationOk(PulseDuration, 0, false))
				SELFTEST_Fail(PROBLEM_SELFTEST);
			else
				SELFTEST_SubState = STS_Amplitude_H_M15_P20;
			break;

		case STS_Amplitude_H_M15_P20:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, true);
			Ok = SELFTEST_RunDriverCheck(IGBTGD_CH_H, false, 20.0f, -15.0f, IGBTGD_RgBitOhm[0], &PulseDuration);
			ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, false);
			if(!Ok || !SELFTEST_DurationOk(PulseDuration, 0, false))
				SELFTEST_Fail(PROBLEM_SELFTEST);
			else
				SELFTEST_SubState = STS_Amplitude_H_M20_P15;
			break;

		case STS_Amplitude_H_M20_P15:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, true);
			SELFTEST_RunDriverCheck(IGBTGD_CH_H, false, 15.0f, -20.0f, IGBTGD_RgBitOhm[0], &PulseDuration);
			ZcRD_SetExpBit(ZCRD_ST_RELAY_H_BIT, false);
			SELFTEST_SubState = STS_Amplitude_L_PM20;
			break;

		case STS_Amplitude_L_PM20:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, true);
			Ok = SELFTEST_RunDriverCheck(IGBTGD_CH_L, false, 20.0f, -20.0f, IGBTGD_RgBitOhm[0], &PulseDuration);
			ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, false);
			if(!Ok || !SELFTEST_DurationOk(PulseDuration, 0, false))
				SELFTEST_Fail(PROBLEM_SELFTEST);
			else
				SELFTEST_SubState = STS_Amplitude_L_M15_P20;
			break;

		case STS_Amplitude_L_M15_P20:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, true);
			SELFTEST_RunDriverCheck(IGBTGD_CH_L, false, 20.0f, -15.0f, IGBTGD_RgBitOhm[0], &PulseDuration);
			ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, false);
			SELFTEST_SubState = STS_Amplitude_L_M20_P15;
			break;

		case STS_Amplitude_L_M20_P15:
			ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, true);
			Ok = SELFTEST_RunDriverCheck(IGBTGD_CH_L, false, 15.0f, -20.0f, IGBTGD_RgBitOhm[0], &PulseDuration);
			ZcRD_SetExpBit(ZCRD_ST_RELAY_L_BIT, false);
			if(!Ok || !SELFTEST_DurationOk(PulseDuration, 0, false))
				SELFTEST_Fail(PROBLEM_SELFTEST);
			else
				SELFTEST_SubState = STS_Finish;
			break;

		case STS_Finish:
			SELFTEST_Succeed();
			break;

		default:
			SELFTEST_SubState = STS_None;
			break;
	}
}
//-----------------------------------------------
