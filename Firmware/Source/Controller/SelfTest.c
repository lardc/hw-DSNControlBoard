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

// Types
//
typedef enum __SelfTestProcess
{
	STP_InProcess = 0,
	STP_Finished,
	STP_Failed
} SelfTestProcess;

typedef enum __CheckStages
{
	CHS_Init = 0,
	CHS_WaitCharge,
	CHS_CapRise,
	CHS_CapFall,
	CHS_CapRiseOff,
	CHS_CapFallOff,
	CHS_Finish
} CheckStages;

typedef enum __AmplitudeCompare
{
	AMP_Equal = 0,
	AMP_OnShorter,
	AMP_OnLonger
} AmplitudeCompare;

// Variables
//
SelfTestSubState SELFTEST_SubState = STS_None;
static CheckStages CheckStage = CHS_Init;
static DeviceState StateAfter = DS_Enabled;
static Int64U Timeout = 0;
static Int16U CapRiseStamp = 0;
static Int32U LastPulseUs = 0;
static Int32U PulseOnUs = 0;
static Int8U RgIndex = 0;
static Boolean AmplitudeRelayOn = false;
static SelfTestProcess CheckResult = STP_Finished;

// Forward
//
static void SELFTEST_Fail(Int16U Problem);
static void SELFTEST_Succeed();
static void SELFTEST_ResetCheck();
static void SELFTEST_SetRelayBit(Int16U Bit, Boolean On);
static void SELFTEST_GateOn(IGBTGD_Channel Channel);
static void SELFTEST_GateOff();
static Boolean SELFTEST_DurationOk(Int32U PulseDuration, Int32U ExpectedDuration, Boolean CompareExpected);
static Boolean SELFTEST_AmplitudeOk(Int32U PulseOn, Int32U PulseOff, AmplitudeCompare Mode);
static void SELFTEST_SwitchToNextState(SelfTestProcess Result, SelfTestSubState NextState);
static SelfTestProcess SELFTEST_PulseCheck(IGBTGD_Channel Channel, Int16U RelayBit, Boolean ToSwitchBoard,
		Boolean ClearMuxAfter, Boolean ScanAllRg);
static SelfTestProcess SELFTEST_AmplitudeCheck(IGBTGD_Channel Channel, Int16U RelayBit, float OpenV, float CloseV,
		AmplitudeCompare Mode, Boolean CloseRelayAtEnd);

// Functions
//
static void SELFTEST_ResetCheck()
{
	CheckStage = CHS_Init;
	CheckResult = STP_Finished;
	Timeout = 0;
	CapRiseStamp = 0;
	LastPulseUs = 0;
	PulseOnUs = 0;
	RgIndex = 0;
}
//-----------------------------------------------

static void SELFTEST_Fail(Int16U Problem)
{
	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);
	ZcRD_WriteExpMask(0);
	IGBTGD_ResetAll();

	AmplitudeRelayOn = false;
	SELFTEST_ResetCheck();
	SELFTEST_SubState = STS_None;
	CONTROL_SwitchToFault(DF_SELF_TEST);
	DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_FAIL;
	DataTable[REG_PROBLEM] = Problem;
	RequestSaveToFlash = true;
}
//-----------------------------------------------

static void SELFTEST_Succeed()
{
	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);
	ZcRD_WriteExpMask(0);
	IGBTGD_ResetAll();

	AmplitudeRelayOn = false;
	SELFTEST_ResetCheck();
	SELFTEST_SubState = STS_None;
	DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_OK;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	CONTROL_SetDeviceState(StateAfter);
	RequestSaveToFlash = true;
}
//-----------------------------------------------

static void SELFTEST_SetRelayBit(Int16U Bit, Boolean On)
{
	Int16U Mask = ZcRD_GetExpMask();

	if(On)
		Mask |= Bit;
	else
		Mask &= ~Bit;

	ZcRD_WriteExpMask(Mask);
}
//-----------------------------------------------

static void SELFTEST_GateOn(IGBTGD_Channel Channel)
{
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
}
//-----------------------------------------------

static void SELFTEST_GateOff()
{
	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);
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

static Boolean SELFTEST_AmplitudeOk(Int32U PulseOn, Int32U PulseOff, AmplitudeCompare Mode)
{
	Int32U Tol = (Int32U)DataTable[REG_ST_RG_DURATION_TOL];
	Int32U MaxDuration = (Int32U)DataTable[REG_ST_PULSE_MAX_DURATION];
	Int32U Delta;

	if(PulseOn == 0 || PulseOff == 0 || PulseOn > MaxDuration || PulseOff > MaxDuration)
		return false;

	switch(Mode)
	{
		case AMP_Equal:
			Delta = (PulseOn > PulseOff) ? (PulseOn - PulseOff) : (PulseOff - PulseOn);
			return (Delta <= Tol);

		case AMP_OnShorter:
			return (PulseOn + Tol < PulseOff);

		case AMP_OnLonger:
			return (PulseOff + Tol < PulseOn);

		default:
			return false;
	}
}
//-----------------------------------------------

static void SELFTEST_SwitchToNextState(SelfTestProcess Result, SelfTestSubState NextState)
{
	if(Result == STP_Finished)
		SELFTEST_SubState = NextState;
	else if(Result == STP_Failed)
		SELFTEST_Fail(PROBLEM_SELFTEST);
}
//-----------------------------------------------

static SelfTestProcess SELFTEST_PulseCheck(IGBTGD_Channel Channel, Int16U RelayBit, Boolean ToSwitchBoard,
		Boolean ClearMuxAfter, Boolean ScanAllRg)
{
	Int16U Stamp = 0;
	float OpenV = DataTable[REG_ST_OPEN_V];
	float CloseV = DataTable[REG_ST_CLOSE_V];
	float RgOhm = IGBTGD_RgBitOhm[ScanAllRg ? RgIndex : 0];
	Int32U WaitMs;
	Boolean Ok;

	switch(CheckStage)
	{
		case CHS_Init:
			CheckResult = STP_Finished;
			SELFTEST_SetRelayBit(RelayBit, true);
			IGBTGD_Configure(Channel, OpenV, CloseV, RgOhm, ToSwitchBoard, false);
			WaitMs = COMM_DELAY_MS + (Int32U)DataTable[REG_ST_CHARGE_TIME];
			if(ScanAllRg && RgIndex > 0)
				WaitMs = (Int32U)DataTable[REG_ST_CHARGE_TIME];
			Timeout = CONTROL_TimeCounter + (WaitMs ? WaitMs : 1);
			CheckStage = CHS_WaitCharge;
			break;

		case CHS_WaitCharge:
			if(CONTROL_TimeCounter > Timeout)
			{
				SELFTEST_GateOn(Channel);
				LastPulseUs = 0;
				CapRiseStamp = 0;
				LL_CaptureArmEdge(FALSE);
				Timeout = CONTROL_TimeCounter + (Int32U)DataTable[REG_ST_CAPTURE_TIMEOUT];
				CheckStage = CHS_CapRise;
			}
			break;

		case CHS_CapRise:
			if(LL_CaptureEdgeReady(&Stamp))
			{
				CapRiseStamp = Stamp;
				LL_CaptureArmEdge(TRUE);
				Timeout = CONTROL_TimeCounter + (Int32U)DataTable[REG_ST_CAPTURE_TIMEOUT];
				CheckStage = CHS_CapFall;
			}
			else if(CONTROL_TimeCounter > Timeout)
			{
				CheckResult = STP_Failed;
				CheckStage = CHS_Finish;
			}
			break;

		case CHS_CapFall:
			if(LL_CaptureEdgeReady(&Stamp))
			{
				LastPulseUs = (Int32U)((Int16U)(Stamp - CapRiseStamp));
				SELFTEST_GateOff();

				if(ScanAllRg)
					Ok = SELFTEST_DurationOk(LastPulseUs, (Int32U)DataTable[REG_ST_RG_0_DURATION + RgIndex], true);
				else
					Ok = SELFTEST_DurationOk(LastPulseUs, 0, false);

				if(!Ok)
				{
					CheckResult = STP_Failed;
					CheckStage = CHS_Finish;
					break;
				}

				if(ScanAllRg)
				{
					RgIndex++;
					if(RgIndex < IGBTGD_RG_BIT_COUNT)
					{
						CheckStage = CHS_Init;
						break;
					}
					RgIndex = 0;
				}

				CheckResult = STP_Finished;
				CheckStage = CHS_Finish;
			}
			else if(CONTROL_TimeCounter > Timeout)
			{
				CheckResult = STP_Failed;
				CheckStage = CHS_Finish;
			}
			break;

		case CHS_Finish:
			SELFTEST_GateOff();
			SELFTEST_SetRelayBit(RelayBit, false);
			if(ClearMuxAfter)
				IGBTGD_SetMux(Channel, false);
			CheckStage = CHS_Init;
			return CheckResult;

		default:
			CheckStage = CHS_Init;
			break;
	}

	return STP_InProcess;
}
//-----------------------------------------------

static SelfTestProcess SELFTEST_AmplitudeCheck(IGBTGD_Channel Channel, Int16U RelayBit, float OpenV, float CloseV,
		AmplitudeCompare Mode, Boolean CloseRelayAtEnd)
{
	Int16U Stamp = 0;
	Int32U WaitMs;

	switch(CheckStage)
	{
		case CHS_Init:
			CheckResult = STP_Finished;
			WaitMs = (Int32U)DataTable[REG_ST_CHARGE_TIME];
			if(!AmplitudeRelayOn)
			{
				SELFTEST_SetRelayBit(RelayBit, true);
				AmplitudeRelayOn = true;
				WaitMs += COMM_DELAY_MS;
			}
			IGBTGD_Configure(Channel, OpenV, CloseV, IGBTGD_RgBitOhm[0], false, false);
			Timeout = CONTROL_TimeCounter + (WaitMs ? WaitMs : 1);
			CheckStage = CHS_WaitCharge;
			break;

		case CHS_WaitCharge:
			if(CONTROL_TimeCounter > Timeout)
			{
				SELFTEST_GateOn(Channel);
				LastPulseUs = 0;
				CapRiseStamp = 0;
				LL_CaptureArmEdge(FALSE);
				Timeout = CONTROL_TimeCounter + (Int32U)DataTable[REG_ST_CAPTURE_TIMEOUT];
				CheckStage = CHS_CapRise;
			}
			break;

		case CHS_CapRise:
			if(LL_CaptureEdgeReady(&Stamp))
			{
				CapRiseStamp = Stamp;
				LL_CaptureArmEdge(TRUE);
				Timeout = CONTROL_TimeCounter + (Int32U)DataTable[REG_ST_CAPTURE_TIMEOUT];
				CheckStage = CHS_CapFall;
			}
			else if(CONTROL_TimeCounter > Timeout)
			{
				CheckResult = STP_Failed;
				CheckStage = CHS_Finish;
			}
			break;

		case CHS_CapFall:
			if(LL_CaptureEdgeReady(&Stamp))
			{
				PulseOnUs = (Int32U)((Int16U)(Stamp - CapRiseStamp));
				SELFTEST_GateOff();
				LastPulseUs = 0;
				CapRiseStamp = 0;
				LL_CaptureArmEdge(FALSE);
				Timeout = CONTROL_TimeCounter + (Int32U)DataTable[REG_ST_CAPTURE_TIMEOUT];
				CheckStage = CHS_CapRiseOff;
			}
			else if(CONTROL_TimeCounter > Timeout)
			{
				CheckResult = STP_Failed;
				CheckStage = CHS_Finish;
			}
			break;

		case CHS_CapRiseOff:
			if(LL_CaptureEdgeReady(&Stamp))
			{
				CapRiseStamp = Stamp;
				LL_CaptureArmEdge(TRUE);
				Timeout = CONTROL_TimeCounter + (Int32U)DataTable[REG_ST_CAPTURE_TIMEOUT];
				CheckStage = CHS_CapFallOff;
			}
			else if(CONTROL_TimeCounter > Timeout)
			{
				CheckResult = STP_Failed;
				CheckStage = CHS_Finish;
			}
			break;

		case CHS_CapFallOff:
			if(LL_CaptureEdgeReady(&Stamp))
			{
				LastPulseUs = (Int32U)((Int16U)(Stamp - CapRiseStamp));
				if(!SELFTEST_AmplitudeOk(PulseOnUs, LastPulseUs, Mode))
					CheckResult = STP_Failed;
				else
					CheckResult = STP_Finished;
				CheckStage = CHS_Finish;
			}
			else if(CONTROL_TimeCounter > Timeout)
			{
				CheckResult = STP_Failed;
				CheckStage = CHS_Finish;
			}
			break;

		case CHS_Finish:
			SELFTEST_GateOff();
			if(CloseRelayAtEnd || CheckResult == STP_Failed)
			{
				SELFTEST_SetRelayBit(RelayBit, false);
				AmplitudeRelayOn = false;
			}
			CheckStage = CHS_Init;
			return CheckResult;

		default:
			CheckStage = CHS_Init;
			break;
	}

	return STP_InProcess;
}
//-----------------------------------------------

void SELFTEST_Stop()
{
	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);
	ZcRD_WriteExpMask(0);
	AmplitudeRelayOn = false;
	SELFTEST_ResetCheck();
	SELFTEST_SubState = STS_None;
}
//-----------------------------------------------

void SELFTEST_Start()
{
	DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_SUB_STATE] = 0;
	AmplitudeRelayOn = false;
	SELFTEST_ResetCheck();
	ZcRD_WriteExpMask(0);

	StateAfter = (CONTROL_State == DS_SafetyActive) ? DS_SafetyActive : DS_Enabled;
	CONTROL_SetDeviceState(DS_InSelfTest);
	SELFTEST_SubState = STS_ProtectionBoardHDut;
}
//-----------------------------------------------

void SELFTEST_Process()
{
	SelfTestProcess Result;

	if(SELFTEST_SubState == STS_None)
		return;

	DataTable[REG_SUB_STATE] = (float)SELFTEST_SubState;

	switch(SELFTEST_SubState)
	{
		case STS_ProtectionBoardHDut:
			Result = SELFTEST_PulseCheck(IGBTGD_CH_H, ZCRD_ST_RELAY_H_BIT, false, false, false);
			SELFTEST_SwitchToNextState(Result, STS_ProtectionBoardLDut);
			break;

		case STS_ProtectionBoardLDut:
			Result = SELFTEST_PulseCheck(IGBTGD_CH_L, ZCRD_ST_RELAY_L_BIT, false, false, false);
			SELFTEST_SwitchToNextState(Result, STS_ProtectionBoardHSwitchBoard);
			break;

		case STS_ProtectionBoardHSwitchBoard:
			Result = SELFTEST_PulseCheck(IGBTGD_CH_H, ZCRD_ST_RELAY_HE_BIT, true, true, false);
			SELFTEST_SwitchToNextState(Result, STS_ProtectionBoardLSwitchBoard);
			break;

		case STS_ProtectionBoardLSwitchBoard:
			Result = SELFTEST_PulseCheck(IGBTGD_CH_L, ZCRD_ST_RELAY_LE_BIT, true, true, false);
			SELFTEST_SwitchToNextState(Result, STS_GateResistorsH);
			break;

		case STS_GateResistorsH:
			Result = SELFTEST_PulseCheck(IGBTGD_CH_H, ZCRD_ST_RELAY_H_BIT, false, false, true);
			SELFTEST_SwitchToNextState(Result, STS_GateResistorsL);
			break;

		case STS_GateResistorsL:
			Result = SELFTEST_PulseCheck(IGBTGD_CH_L, ZCRD_ST_RELAY_L_BIT, false, false, true);
			SELFTEST_SwitchToNextState(Result, STS_AmplitudeHPm20);
			break;

		case STS_AmplitudeHPm20:
			Result = SELFTEST_AmplitudeCheck(IGBTGD_CH_H, ZCRD_ST_RELAY_H_BIT, 20.0f, -20.0f, AMP_Equal, false);
			SELFTEST_SwitchToNextState(Result, STS_AmplitudeHM15P20);
			break;

		case STS_AmplitudeHM15P20:
			Result = SELFTEST_AmplitudeCheck(IGBTGD_CH_H, ZCRD_ST_RELAY_H_BIT, 20.0f, -15.0f, AMP_OnShorter, false);
			SELFTEST_SwitchToNextState(Result, STS_AmplitudeHM20P15);
			break;

		case STS_AmplitudeHM20P15:
			Result = SELFTEST_AmplitudeCheck(IGBTGD_CH_H, ZCRD_ST_RELAY_H_BIT, 15.0f, -20.0f, AMP_OnLonger, true);
			SELFTEST_SwitchToNextState(Result, STS_AmplitudeLPm20);
			break;

		case STS_AmplitudeLPm20:
			Result = SELFTEST_AmplitudeCheck(IGBTGD_CH_L, ZCRD_ST_RELAY_L_BIT, 20.0f, -20.0f, AMP_Equal, false);
			SELFTEST_SwitchToNextState(Result, STS_AmplitudeLM15P20);
			break;

		case STS_AmplitudeLM15P20:
			Result = SELFTEST_AmplitudeCheck(IGBTGD_CH_L, ZCRD_ST_RELAY_L_BIT, 20.0f, -15.0f, AMP_OnShorter, false);
			SELFTEST_SwitchToNextState(Result, STS_AmplitudeLM20P15);
			break;

		case STS_AmplitudeLM20P15:
			Result = SELFTEST_AmplitudeCheck(IGBTGD_CH_L, ZCRD_ST_RELAY_L_BIT, 15.0f, -20.0f, AMP_OnLonger, true);
			SELFTEST_SwitchToNextState(Result, STS_Finish);
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
