// Header
//
#include "DPN.h"

// Includes
//
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "BCCIMHighLevel.h"
#include "Controller.h"

// Types
//
typedef enum __DPN_FsmState
{
	DFS_Idle = 0,

	DFS_CfgWriteVoltage,
	DFS_CfgWriteCurrent,
	DFS_CfgWritePosition,
	DFS_CfgWriteMaxDuration,
	DFS_CfgCall,
	DFS_CfgWaitReady,
	DFS_CfgReadInductance,
	DFS_CfgReadDuration,

	DFS_StopCall,
	DFS_StopWait
} DPN_FsmState;

// Variables
//
static DPN_FsmState FsmState = DFS_Idle;
static DPN_FsmResult FsmResult = DPN_FR_None;
static Int64U WaitDeadline = 0;
static Int64U NextPollTime = 0;

// Function prototypes
//
bool DPN_CallAction(Int16U Action);
static void DPN_FsmFailInterface();
static void DPN_FsmFailDevice();
static void DPN_FsmFailTimeout();
static void DPN_FsmFailProblem();
static void DPN_FsmSucceed();
static Boolean DPN_FsmStart(DPN_FsmState FirstState);
static Boolean DPN_PollReadyAfterCommand(Boolean *OutReady);

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
	FsmState = DFS_Idle;
	FsmResult = DPN_FR_None;
	WaitDeadline = 0;
	NextPollTime = 0;
}
//--------------------------------------

Boolean DPN_IsBusy()
{
	return (FsmState != DFS_Idle);
}
//--------------------------------------

DPN_FsmResult DPN_GetFsmResult()
{
	return FsmResult;
}
//--------------------------------------

static Boolean DPN_FsmStart(DPN_FsmState FirstState)
{
	if(FsmState != DFS_Idle)
		return false;

	FsmResult = DPN_FR_Busy;
	FsmState = FirstState;
	WaitDeadline = 0;
	NextPollTime = 0;
	return true;
}
//--------------------------------------

Boolean DPN_RequestConfig()
{
	return DPN_FsmStart(DFS_CfgWriteVoltage);
}
//--------------------------------------

Boolean DPN_RequestStop()
{
	return DPN_FsmStart(DFS_StopCall);
}
//--------------------------------------

static void DPN_FsmSucceed()
{
	FsmState = DFS_Idle;
	FsmResult = DPN_FR_Ok;
}
//--------------------------------------

static void DPN_FsmFailInterface()
{
	CONTROL_SwitchToFault(DF_DPN_INTERFACE);
	FsmResult = DPN_FR_Fail;
}
//--------------------------------------

static void DPN_FsmFailDevice()
{
	CONTROL_SwitchToFault(DF_DPN);
	FsmResult = DPN_FR_Fail;
}
//--------------------------------------

static void DPN_FsmFailTimeout()
{
	CONTROL_SwitchToFault(DF_DPN_FAILED_TO_FINISH);
	FsmResult = DPN_FR_Fail;
}
//--------------------------------------

static void DPN_FsmFailProblem()
{
	FsmState = DFS_Idle;
	FsmResult = DPN_FR_Fail;
}
//--------------------------------------

static Boolean DPN_PollReadyAfterCommand(Boolean *OutReady)
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
		DPN_FsmFailInterface();
		return false;
	}

	if(DPN_State == DPN_Fault)
	{
		DPN_FsmFailDevice();
		return false;
	}

	if(DPN_State == DPN_Ready || DPN_State == DPN_Disabled)
		*OutReady = true;

	return true;
}
//--------------------------------------

void DPN_Process()
{
	Boolean Ready = false;
	float Value = 0;

	switch(FsmState)
	{
		case DFS_Idle:
			break;

		case DFS_CfgWriteVoltage:
			if(DPN_WriteRegFloat(REG_DPN_VOLTAGE, DataTable[REG_TEST_VOLTAGE]))
				FsmState = DFS_CfgWriteCurrent;
			else
			{
				FsmState = DFS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DFS_CfgWriteCurrent:
			if(DPN_WriteRegFloat(REG_DPN_CURRENT, DataTable[REG_TEST_CURRENT]))
				FsmState = DFS_CfgWritePosition;
			else
			{
				FsmState = DFS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DFS_CfgWritePosition:
			if(DPN_WriteReg(REG_DPN_DUT_POSITION, (Int16U)DataTable[REG_TEST_POS]))
				FsmState = DFS_CfgWriteMaxDuration;
			else
			{
				FsmState = DFS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DFS_CfgWriteMaxDuration:
			if(DPN_WriteRegFloat(REG_DPN_MAX_DURATION, DataTable[REG_TEST_SECOND_PULSE]))
				FsmState = DFS_CfgCall;
			else
			{
				FsmState = DFS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DFS_CfgCall:
			if(DPN_Config())
			{
				WaitDeadline = CONTROL_TimeCounter + DPN_WAIT_MS;
				NextPollTime = 0;
				FsmState = DFS_CfgWaitReady;
			}
			else
			{
				FsmState = DFS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DFS_CfgWaitReady:
			if(CONTROL_TimeCounter > WaitDeadline)
			{
				DPN_FsmFailTimeout();
				break;
			}

			if(!DPN_PollReadyAfterCommand(&Ready))
				break;

			if(Ready)
			{
				if(DataTable[REG_DPN_EMULATED] || DPN_CheckOPResult(OPRESULT_OK))
					FsmState = DFS_CfgReadInductance;
				else
					DPN_FsmFailProblem();
			}
			break;

		case DFS_CfgReadInductance:
			if(DataTable[REG_DPN_EMULATED])
			{
				DataTable[REG_SELECTED_INDUCTANCE] = 0;
				FsmState = DFS_CfgReadDuration;
			}
			else if(DPN_ReadRegisterFloat(REG_DPN_INDUCTANCE, &Value))
			{
				DataTable[REG_SELECTED_INDUCTANCE] = Value;
				FsmState = DFS_CfgReadDuration;
			}
			else
			{
				FsmState = DFS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DFS_CfgReadDuration:
			if(DataTable[REG_DPN_EMULATED])
			{
				DataTable[REG_SELECTED_PULSE_WIDTH] = DataTable[REG_TEST_SECOND_PULSE];
				DPN_FsmSucceed();
			}
			else if(DPN_ReadRegisterFloat(REG_DPN_MEASURE_DURATION, &Value))
			{
				DataTable[REG_SELECTED_PULSE_WIDTH] = Value;
				DPN_FsmSucceed();
			}
			else
			{
				FsmState = DFS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DFS_StopCall:
			if(DPN_Disable())
			{
				WaitDeadline = CONTROL_TimeCounter + DPN_WAIT_MS;
				NextPollTime = 0;
				FsmState = DFS_StopWait;
			}
			else
			{
				FsmState = DFS_Idle;
				FsmResult = DPN_FR_Fail;
			}
			break;

		case DFS_StopWait:
			if(CONTROL_TimeCounter > WaitDeadline)
			{
				DPN_FsmFailTimeout();
				break;
			}

			if(!DPN_PollReadyAfterCommand(&Ready))
				break;

			if(Ready)
				DPN_FsmSucceed();
			break;

		default:
			DPN_ResetFsm();
			break;
	}
}
//--------------------------------------
