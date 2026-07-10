#ifndef CONTROLLER_PMXU_H_
#define CONTROLLER_PMXU_H_

// Includes
//
#include "stdinc.h"

// Types
//
typedef enum __PMXUState
{
	PS_None				= 0,
	PS_Fault			= 1,
	PS_Disabled			= 2,
	PS_Enabled			= 3,
	PS_SafetyActive		= 4,
	PS_SafetyTrig		= 5,
	PS_InProcess		= 7
} PMXUState;

// Variables
//

// Functions
//
bool PMXU_IsReady();
bool PMXU_InFault();
bool PMXU_ClearFault();
bool PMXU_ClearWarning();
bool PMXU_SwitchCommutation(Int16U Position, Int16U DevCase, Int16U Scheme, Int16U CommutationNumber);
bool PMXU_Enable();
bool PMXU_Disable();
bool PMXU_CheckReady();
bool PMXU_CheckState(PMXUState State);
bool PMXU_CheckOPResult(Int16U OPResult);
bool PMXU_SafetyActivate();
bool PMXU_SafetyDeactivate();
bool PMXU_StartSelfTest();

#endif /* CONTROLLER_PMXU_H_ */
