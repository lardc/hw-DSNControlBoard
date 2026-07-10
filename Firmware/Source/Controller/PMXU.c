// Header
//
#include "PMXU.h"

// Includes
//
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "BCCIMHighLevel.h"
#include "LowLevel.h"
#include "Delay.h"

// Variables
//

// Function prototypes
//
bool PMXU_CallAction(Int16U Action);
bool PMXU_WriteReg(Int16U RegAddress, Int16U RegData);
void PMXU_ReadRegister(Int16U RegAddress, Int16U* RegData);


// Functions
//
bool PMXU_StartSelfTest()
{
	return PMXU_CallAction(ACT_PMXU_SELFTEST);
}
//--------------------------------------

bool PMXU_Enable()
{
	return PMXU_CallAction(ACT_PMXU_ENABLE_POWER);
}
//--------------------------------------

bool PMXU_Disable()
{
	return PMXU_CallAction(ACT_PMXU_DISABLE_POWER);
}
//--------------------------------------

bool PMXU_IsReady()
{
	return (DataTable[REG_PMXU_EMULATED]) ? true : (PMXU_CheckState(PS_Enabled) || PMXU_CheckState(PS_SafetyActive));
}
//--------------------------------------

bool PMXU_InFault()
{
	return (DataTable[REG_PMXU_EMULATED]) ? false : PMXU_CheckState(PS_Fault);
}
//--------------------------------------

bool PMXU_CheckReady()
{
	Int16U PMXU_State = 0;

	if(DataTable[REG_PMXU_EMULATED])
		return true;

	if(BHL_ReadRegister(DataTable[REG_PMXU_CAN_ID], REG_PMXU_DEV_STATE, &PMXU_State))
	{
		if(PMXU_State == PS_Enabled || PMXU_State == PS_SafetyActive)
			return true;
		else if(PMXU_State == PS_Fault)
		{
			CONTROL_SwitchToFault(DF_PMXU);
			return false;
		}
	}
	else
		CONTROL_SwitchToFault(DF_PMXU_INTERFACE);

	return false;
}
//--------------------------------------

bool PMXU_CheckState(PMXUState State)
{
	Int16U PMXU_State = 0;

	if(DataTable[REG_PMXU_EMULATED])
		return true;

	if(BHL_ReadRegister(DataTable[REG_PMXU_CAN_ID], REG_PMXU_DEV_STATE, &PMXU_State))
		return (PMXU_State == State) ? true : false;
	else
		CONTROL_SwitchToFault(DF_PMXU_INTERFACE);

	return false;
}
//--------------------------------------

bool PMXU_CheckOPResult(Int16U OPResult)
{
	Int16U PMXU_OPResult = 0;

	if(DataTable[REG_PMXU_EMULATED])
		return true;

	if(BHL_ReadRegister(DataTable[REG_PMXU_CAN_ID], REG_PMXU_OP_RESULT, &PMXU_OPResult))
	{
		if(PMXU_OPResult == OPRESULT_FAIL)
		{
			Int16U problem = 0;
			PMXU_ReadRegister(REG_PMXU_PROBLEM, &problem);
			DataTable[REG_DBG2] = problem;
			CONTROL_FinishedWithProblem(PROBLEM_PMXU_FAILED_COMMAND);
			return false;
		}
		else
			return (PMXU_OPResult == OPResult) ? true : false;
	}
	else
		CONTROL_SwitchToFault(DF_PMXU_INTERFACE);

	return false;
}
//--------------------------------------

bool PMXU_SwitchCommutation(Int16U Position, Int16U DevCase, Int16U Scheme,Int16U CommutationNumber)
{
	if(PMXU_WriteReg(REG_PMXU_DUT_POSITION, Position))
		if(PMXU_WriteReg(REG_PMXU_DEV_CASE, DevCase))
			return PMXU_CallAction(CommutationNumber);

	return false;
}
//--------------------------------------

bool PMXU_ClearFault()
{
	return PMXU_CallAction(ACT_PMXU_CLR_FAULT);
}
//--------------------------------------

bool PMXU_ClearWarning()
{
	return PMXU_CallAction(ACT_PMXU_CLR_WARNING);
}
//--------------------------------------

bool PMXU_SafetyActivate()
{
	if(PMXU_CallAction(ACT_PMXU_SET_ACTIVE))
	{
		if(PMXU_CheckState(DS_SafetyActive))
			return true;
	}

	return false;
}
//--------------------------------------

bool PMXU_SafetyDeactivate()
{
	if(PMXU_CallAction(ACT_PMXU_SET_INACTIVE))
	{
		if(PMXU_CheckState(DS_Enabled))
			return true;
	}

	return false;
}
//--------------------------------------

bool PMXU_CallAction(Int16U Action)
{
	if(DataTable[REG_PMXU_EMULATED])
		return true;

	if(BHL_Call(DataTable[REG_PMXU_CAN_ID], Action))
		return true;
	else
	{
		CONTROL_SwitchToFault(DF_PMXU_INTERFACE);
		return false;
	}
}
//--------------------------------------

bool PMXU_WriteReg(Int16U RegAddress, Int16U RegData)
{
	if(DataTable[REG_PMXU_EMULATED])
		return true;

	if(BHL_WriteRegister(DataTable[REG_PMXU_CAN_ID], RegAddress, RegData))
		return true;
	else
	{
		CONTROL_SwitchToFault(DF_PMXU_INTERFACE);
		return false;
	}
}
//--------------------------------------

void PMXU_ReadRegister(Int16U RegAddress, Int16U* RegData)
{
	if(!BHL_ReadRegister(DataTable[REG_PMXU_CAN_ID], RegAddress, RegData))
		CONTROL_SwitchToFault(DF_PMXU_INTERFACE);
}
