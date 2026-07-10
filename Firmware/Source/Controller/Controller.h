#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
//
#include "Global.h"
#include "Commutator.h"

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

typedef enum __DeviceSelfTestState
{
	STS_None,
	STS_Start,
	STS_InputBoardStage0,
	STS_InputBoardStage1,
	STS_InputBoardStage2,
	STS_InputBoardStage3,
	STS_ThermBoardStage0,
	STS_ThermBoardStage1,
	STS_HV1BboardStage0,
	STS_HV1BboardStage1,
	STS_HV1BboardStage2,
	STS_HV2BboardStage0,
	STS_HV2BboardStage1,
	STS_HV2BboardStage2,
	STS_Finish,
} DeviceSelfTestState;

typedef enum __ModuleType
{
	Module_None,
	MDAA_DD,
	MDDA_DD,
	MDFA_DD,
	MDSM_SD,
	MDSV_SD,
	MIAA_CE,
	MIAA_HB,
	MIAA_HC,
	MIAA_LC,
	MIFA_HB,
	MIFA_HC,
	MIFA_LC,
	MIFA_SD,
	MIDA_HB,
	MIHA_HB,
	MIHA_HC,
	MIHA_LC,
	MIHM_SS,
	MIHV_SS,
	MISM_CH,
	MISM_DS,
	MISM_SS,
	MISV_SS,
	MIXM_HB,
	MIXM_LR_LRD,
	MIXV_HB,
} ModuleTypes;

// Variables
//
extern volatile Int64U CONTROL_TimeCounter;
extern volatile DeviceState CONTROL_State;
extern volatile DeviceSelfTestState CONTROL_SubState;
extern Int64U CONTROL_LEDTimeout;
extern volatile Int16U CONTROL_DiagCounter;
//
extern volatile float CONTROL_DiagData[VALUES_DIAG_SIZE];

// Functions
//
void CONTROL_Init();
void CONTROL_Idle();
void CONTROL_SaveTestResult();
void CONTROL_SwitchToFault(Int16U Reason);
void CONTROL_SetDeviceState(DeviceState NewState);
void CONTROL_SetDeviceSubState(DeviceSelfTestState NewSubState);
void CONTROL_ResetToDefaultState();
bool CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void CONTROL_LogicProcess();
void CONTROL_UpdateWatchDog();
void CONTROL_ResetOutputRegisters();
void CONTROL_HandleFrontPanelLamp(CommutationState Commutation);
void CONTROL_SafetyIrqTick();
void CONTROL_FinishedWithProblem(Int16U Problem);

#endif // __CONTROLLER_H
