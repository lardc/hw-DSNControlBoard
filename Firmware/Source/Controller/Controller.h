#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
//
#include "Global.h"
#include "stdinc.h"

// Types
//
typedef enum __DeviceState
{
	DS_None				= 0,
	DS_Fault			= 1,
	DS_Disabled			= 2,
	DS_Enabled			= 3,
	DS_SafetyActive		= 4,
	DS_SafetyTrig		= 5,
	DS_InSelfTest		= 6
} DeviceState;

// Variables
//
extern volatile Int64U CONTROL_TimeCounter;
extern volatile DeviceState CONTROL_State;
extern volatile Int16U CONTROL_DiagCounter;
//
extern volatile float CONTROL_DiagData[VALUES_DIAG_SIZE];

// Functions
//
void CONTROL_Init();
void CONTROL_Idle();
void CONTROL_SwitchToFault(Int16U Reason);
void CONTROL_SetDeviceState(DeviceState NewState);
void CONTROL_ResetToDefaultState();
bool CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void CONTROL_UpdateWatchDog();
void CONTROL_ResetOutputRegisters();
void CONTROL_HandleFrontPanelLamp();
void CONTROL_SafetyIrqTick();
void CONTROL_FinishedWithProblem(Int16U Problem);

#endif // __CONTROLLER_H
