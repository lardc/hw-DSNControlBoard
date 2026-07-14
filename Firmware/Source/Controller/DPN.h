#ifndef CONTROLLER_DPN_H_
#define CONTROLLER_DPN_H_

// Includes
//
#include "stdinc.h"

// Types
//
// Состояния DPN (совпадают с DeviceState на стороне DPN)
typedef enum __DPNState
{
	DPN_None			= 0,
	DPN_Fault			= 1,
	DPN_Disabled		= 2,
	DPN_Ready			= 3,
	DPN_InProcess		= 4,
	DPN_SafetyTrig		= 5,
	DPN_InSelfTest		= 6
} DPNState;

// Functions
//
bool DPN_IsReady();
bool DPN_InFault();
bool DPN_ClearFault();
bool DPN_ClearWarning();
bool DPN_Enable();
bool DPN_Disable();
bool DPN_CheckReady();
bool DPN_CheckState(DPNState State);
bool DPN_CheckOPResult(Int16U OPResult);
bool DPN_Config();
bool DPN_ClearSafetyTrig();
bool DPN_StartSelfTest();
bool DPN_WriteReg(Int16U RegAddress, Int16U RegData);
void DPN_ReadRegister(Int16U RegAddress, Int16U* RegData);

#endif /* CONTROLLER_DPN_H_ */
