// Header
//
#include "DPN.h"

// Includes
//
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "BCCIMHighLevel.h"
#include "Controller.h"

// Function prototypes
//
bool DPN_CallAction(Int16U Action);

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

void DPN_ReadRegister(Int16U RegAddress, Int16U* RegData)
{
	if(!BHL_ReadRegister(DataTable[REG_DPN_CAN_ID], RegAddress, RegData))
		CONTROL_SwitchToFault(DF_DPN_INTERFACE);
}
//--------------------------------------
