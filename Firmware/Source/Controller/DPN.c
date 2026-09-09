// Header
//
#include "DPN.h"

// Includes
//
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "BCCIMHighLevel.h"
#include "Controller.h"

// Variables
//
DPNSubState DPN_SubState = DPNS_Idle;
static DPN_FsmResult FsmResult = DPN_FR_None;
static Int64U WaitDeadline = 0;
static Int64U NextPollTime = 0;
static Boolean SawBusy = false;

// Function prototypes
//
bool DPN_CallAction(Int16U Action);
static Boolean DPN_FsmStart(DPNSubState FirstState);
static Boolean DPN_PollReadyAfterCommand(Boolean *OutReady, Boolean RequireSawBusy);

// Functions
//

bool DPN_StartSelfTest()
{
	return DPN_CallAction(ACT_DPN_SELF_TEST);
}
//--------------------------------------

bool DPN_Enable()
{
	return DPN_CallAction(ACT_DPN_ENABLE_POWER);
}
//--------------------------------------

bool DPN_Disable()
{
	return DPN_CallAction(ACT_DPN_DISABLE_POWER);
}
//--------------------------------------

bool DPN_IsReady()
{
	return (DataTable[REG_DPN_EMULATED]) ? true : DPN_CheckState(DPN_Ready);
}
//--------------------------------------

bool DPN_InFault()
{
	return (DataTable[REG_DPN_EMULATED]) ? false : DPN_CheckState(DPN_Fault);
}
//--------------------------------------

bool DPN_CheckReady()
{
	Int16U DPN_State = 0;

	if(DataTable[REG_DPN_EMULATED])
		return true;

	if(BHL_ReadRegister(DataTable[REG_DPN_CAN_ID], REG_DPN_DEV_STATE, &DPN_State))
	{
		if(DPN_State == DPN_Ready)
			return true;
		else if(DPN_State == DPN_Fault)
		{
			CONTROL_SwitchToFault(DF_DPN);
			return false;
		}
	}
	else
		CONTROL_SwitchToFault(DF_DPN_INTERFACE);

	return false;
}
//--------------------------------------

bool DPN_CheckState(DPNState State)
{
	Int16U DPN_State = 0;

	if(DataTable[REG_DPN_EMULATED])
		return true;

	if(BHL_ReadRegister(DataTable[REG_DPN_CAN_ID], REG_DPN_DEV_STATE, &DPN_State))
		return (DPN_State == State) ? true : false;
	else
		CONTROL_SwitchToFault(DF_DPN_INTERFACE);

	return false;
}
//--------------------------------------

bool DPN_CheckOPResult(Int16U OPResult)
{
	Int16U DPN_OPResult = 0;

	if(DataTable[REG_DPN_EMULATED])
		return true;

	if(BHL_ReadRegister(DataTable[REG_DPN_CAN_ID], REG_DPN_OP_RESULT, &DPN_OPResult))
	{
		if(DPN_OPResult == OPRESULT_FAIL)
		{
			Int16U problem = 0;
			DPN_ReadRegister(REG_DPN_PROBLEM, &problem);
			DataTable[REG_DBG] = problem;
			CONTROL_FinishedWithProblem(PROBLEM_DPN_FAILED_COMMAND);
			return false;
		}
		else
			return (DPN_OPResult == OPResult) ? true : false;
	}
	else
		CONTROL_SwitchToFault(DF_DPN_INTERFACE);

	return false;
}
//--------------------------------------

bool DPN_Config()
{
	return DPN_CallAction(ACT_DPN_CONFIG);
}
//--------------------------------------

bool DPN_ClearFault()
{
	return DPN_CallAction(ACT_DPN_CLR_FAULT);
}
//--------------------------------------

bool DPN_ClearWarning()
{
	return DPN_CallAction(ACT_DPN_CLR_WARNING);
}
//--------------------------------------

bool DPN_ClearSafetyTrig()
{
	return DPN_CallAction(ACT_DPN_SAFETY_TRIG_CLEAR);
}
//--------------------------------------

bool DPN_CallAction(Int16U Action)
{
	if(DataTable[REG_DPN_EMULATED])
		return true;

	if(BHL_Call(DataTable[REG_DPN_CAN_ID], Action))
		return true;
	else
	{
		CONTROL_SwitchToFault(DF_DPN_INTERFACE);
		return false;
	}
}
//--------------------------------------

bool DPN_WriteReg(Int16U RegAddress, Int16U RegData)
{
	if(DataTable[REG_DPN_EMULATED])
		return true;

	if(BHL_WriteRegister(DataTable[REG_DPN_CAN_ID], RegAddress, RegData))
		return true;
	else
	{
		CONTROL_SwitchToFault(DF_DPN_INTERFACE);
		return false;
	}
}
//--------------------------------------

bool DPN_WriteRegFloat(Int16U RegAddress, float RegData)
{
	if(DataTable[REG_DPN_EMULATED])
		return true;

	if(BHL_WriteRegisterFloat(DataTable[REG_DPN_CAN_ID], RegAddress, RegData))
		return true;
	else
	{
		CONTROL_SwitchToFault(DF_DPN_INTERFACE);
		return false;
	}
}
//--------------------------------------

void DPN_ReadRegister(Int16U RegAddress, Int16U* RegData)
{
	if(!BHL_ReadRegister(DataTable[REG_DPN_CAN_ID], RegAddress, RegData))
		CONTROL_SwitchToFault(DF_DPN_INTERFACE);
}
//--------------------------------------

bool DPN_ReadRegisterFloat(Int16U RegAddress, float* RegData)
{
	if(DataTable[REG_DPN_EMULATED])
		return true;

	if(BHL_ReadRegisterFloat(DataTable[REG_DPN_CAN_ID], RegAddress, RegData))
		return true;
	else
	{
		CONTROL_SwitchToFault(DF_DPN_INTERFACE);
		return false;
	}
}
//--------------------------------------

void DPN_ResetFsm()
{
	DPN_SubState = DPNS_Idle;
	// Нет завершённого результата (не Ok/Fail) — после abort/fault/reset
	FsmResult = DPN_FR_None;
	WaitDeadline = 0;
	NextPollTime = 0;
	SawBusy = false;
}
//--------------------------------------

DPN_FsmResult DPN_GetFsmResult()
{
	return FsmResult;
}
//--------------------------------------

static Boolean DPN_FsmStart(DPNSubState FirstState)
{
	if(DPN_SubState != DPNS_Idle)
		return false;

	FsmResult = DPN_FR_Busy;
	DPN_SubState = FirstState;
	WaitDeadline = 0;
	NextPollTime = 0;
	SawBusy = false;
	return true;
}
//--------------------------------------

Boolean DPN_RequestConfig()
{
	return DPN_FsmStart(DPNS_CfgWriteVoltage);
}
//--------------------------------------

Boolean DPN_RequestStop()
{
	return DPN_FsmStart(DPNS_StopCall);
}
//--------------------------------------

static Boolean DPN_PollReadyAfterCommand(Boolean *OutReady, Boolean RequireSawBusy)
{
	Int16U DPN_State = 0;

	*OutReady = false;

	if(DataTable[REG_DPN_EMULATED])
	{
		*OutReady = true;
		return true;
	}

	if(CONTROL_TimeCounter < NextPollTime)
		return true;

	NextPollTime = CONTROL_TimeCounter + DPN_CAN_REQUEST_DELAY_MS;

	if(!BHL_ReadRegister(DataTable[REG_DPN_CAN_ID], REG_DPN_DEV_STATE, &DPN_State))
	{
		CONTROL_SwitchToFault(DF_DPN_INTERFACE);
		FsmResult = DPN_FR_Fail;
		return false;
	}

	if(DPN_State == DPN_Fault)
	{
		CONTROL_SwitchToFault(DF_DPN);
		FsmResult = DPN_FR_Fail;
		return false;
	}

	if(DPN_State == DPN_InProcess || DPN_State == DPN_InSelfTest)
	{
		SawBusy = true;
		return true;
	}

	if(RequireSawBusy)
	{
		if(SawBusy && DPN_State == DPN_Ready)
			*OutReady = true;
	}
	else if(DPN_State == DPN_None || DPN_State == DPN_Disabled)
		*OutReady = true;

	return true;
}
//--------------------------------------

void DPN_Process()
{
	Boolean Ready = false;
	float Value = 0;

	switch(DPN_SubState)
	{
		case DPNS_Idle:
			break;

		case DPNS_CfgWriteVoltage:
			if(!DPN_CheckReady())
			{
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Fail;
				if(CONTROL_State != DS_Fault)
					CONTROL_FinishedWithProblem(PROBLEM_DPN_NOT_READY);
				break;
			}

			if(DPN_WriteRegFloat(REG_DPN_VOLTAGE, DataTable[REG_TEST_VOLTAGE]))
				DPN_SubState = DPNS_CfgWriteCurrent;
			else
			{
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DPNS_CfgWriteCurrent:
			if(DPN_WriteRegFloat(REG_DPN_CURRENT, DataTable[REG_TEST_CURRENT]))
				DPN_SubState = DPNS_CfgWritePosition;
			else
			{
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DPNS_CfgWritePosition:
			if(DPN_WriteReg(REG_DPN_DUT_POSITION, (Int16U)DataTable[REG_TEST_POS]))
				DPN_SubState = DPNS_CfgWriteMaxDuration;
			else
			{
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DPNS_CfgWriteMaxDuration:
			if(DPN_WriteRegFloat(REG_DPN_MAX_DURATION, DataTable[REG_TEST_SECOND_PULSE] * 1000.0f))
				DPN_SubState = DPNS_CfgCall;
			else
			{
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DPNS_CfgCall:
			if(DPN_Config())
			{
				SawBusy = false;
				WaitDeadline = CONTROL_TimeCounter + DPN_WAIT_MS;
				NextPollTime = CONTROL_TimeCounter + DPN_CAN_REQUEST_DELAY_MS;
				DPN_SubState = DPNS_CfgWaitReady;
			}
			else
			{
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DPNS_CfgWaitReady:
			if(CONTROL_TimeCounter > WaitDeadline)
			{
				CONTROL_SwitchToFault(DF_DPN_FAILED_TO_FINISH);
				FsmResult = DPN_FR_Fail;
				break;
			}

			if(!DPN_PollReadyAfterCommand(&Ready, true))
				break;

			if(Ready)
			{
				Int16U OpResult = 0;

				if(DataTable[REG_DPN_EMULATED])
				{
					DPN_SubState = DPNS_CfgReadInductance;
					break;
				}

				if(!BHL_ReadRegister(DataTable[REG_DPN_CAN_ID], REG_DPN_OP_RESULT, &OpResult))
				{
					CONTROL_SwitchToFault(DF_DPN_INTERFACE);
					FsmResult = DPN_FR_Fail;
					break;
				}

				if(OpResult == OPRESULT_FAIL)
				{
					Int16U Problem = 0;
					DPN_ReadRegister(REG_DPN_PROBLEM, &Problem);
					DataTable[REG_DBG] = Problem;
					CONTROL_FinishedWithProblem(PROBLEM_DPN_FAILED_COMMAND);
					DPN_SubState = DPNS_Idle;
					FsmResult = DPN_FR_Fail;
					SawBusy = false;
				}
				else
					DPN_SubState = DPNS_CfgReadInductance;
			}
			break;

		case DPNS_CfgReadInductance:
			if(DataTable[REG_DPN_EMULATED])
			{
				DataTable[REG_SELECTED_INDUCTANCE] = 0;
				DPN_SubState = DPNS_CfgReadDuration;
			}
			else if(DPN_ReadRegisterFloat(REG_DPN_INDUCTANCE, &Value))
			{
				DataTable[REG_SELECTED_INDUCTANCE] = Value;
				DPN_SubState = DPNS_CfgReadDuration;
			}
			else
			{
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DPNS_CfgReadDuration:
			if(DataTable[REG_DPN_EMULATED])
			{
				DataTable[REG_SELECTED_PULSE_WIDTH] = DataTable[REG_TEST_SECOND_PULSE];
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Ok;
				SawBusy = false;
			}
			else if(DPN_ReadRegisterFloat(REG_DPN_MEASURE_DURATION, &Value))
			{
				DataTable[REG_SELECTED_PULSE_WIDTH] = Value / 1000.0f;
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Ok;
				SawBusy = false;
			}
			else
			{
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DPNS_StopCall:
			if(DPN_Disable())
			{
				SawBusy = false;
				WaitDeadline = CONTROL_TimeCounter + DPN_WAIT_MS;
				NextPollTime = CONTROL_TimeCounter + DPN_CAN_REQUEST_DELAY_MS;
				DPN_SubState = DPNS_StopWait;
			}
			else
			{
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DPNS_StopWait:
			if(CONTROL_TimeCounter > WaitDeadline)
			{
				CONTROL_SwitchToFault(DF_DPN_FAILED_TO_FINISH);
				FsmResult = DPN_FR_Fail;
				break;
			}

			if(!DPN_PollReadyAfterCommand(&Ready, false))
				break;

			if(Ready)
			{
				DPN_SubState = DPNS_Idle;
				FsmResult = DPN_FR_Ok;
				SawBusy = false;
			}
			break;

		default:
			DPN_ResetFsm();
			break;
	}
}
//--------------------------------------
