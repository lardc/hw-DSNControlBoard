// -----------------------------------------
// Commutation controller
// ----------------------------------------

#ifndef __COMMUTATOR_H
#define __COMMUTATOR_H

// Include
#include "stdinc.h"

// Defines
//
#define PMXU_WAIT_LONG		500 // в мс
#define PMXU_CAN_REQUEST_DELAY	20
// Types
//
typedef enum __CommutationState
{
	COMM_Def		= 0,
	COMM_NoPE,
	COMM_Iges_Pos,
	COMM_Iges_Neg,
	COMM_Ugeth,
	COMM_Ucesat,
	COMM_Uf,
	COMM_IcesOrIrrm,
	COMM_Thermistor
} CommutationState;

typedef enum __DeviceProcessState
{
	DPS_None = 0,
	DPS_Start,
	DPS_CheckIcesAndDischarge,
	DPS_CheckStatusAfterDischarge,
	DPS_PMXUCommutate,
	DPS_CheckStatusAfterCommutation,
	DPS_MXUCommutate,
} DeviceProcessState;

// Variables
//
extern CommutationState COMM_State;
extern DeviceProcessState COMM_ProcessState;

// Functions
//
void COMM_ComposeDisconnectPE();
void COMM_Commutate(Int16U ActionID);
void COMM_Default();
void COMM_Process();
bool COMM_ValidateRequest(Int16U ActionID, Int16U Position);
Int32U COMM_CalcModuleType();

#endif // __COMMUTATOR_H
