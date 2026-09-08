#ifndef CONTROLLER_DPN_H_
#define CONTROLLER_DPN_H_

// Includes
//
#include "stdinc.h"

// Timeouts
//
#define DPN_WAIT_MS					5000	// Ожидание Ready после команды, мс
#define DPN_CAN_REQUEST_DELAY_MS	20		// Пауза между опросами состояния, мс

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

// Результат последней асинхронной операции FSM
typedef enum __DPN_FsmResult
{
	DPN_FR_None			= 0,
	DPN_FR_Busy			= 1,
	DPN_FR_Ok			= 2,
	DPN_FR_Fail			= 3
} DPN_FsmResult;

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
bool DPN_WriteRegFloat(Int16U RegAddress, float RegData);
void DPN_ReadRegister(Int16U RegAddress, Int16U* RegData);
bool DPN_ReadRegisterFloat(Int16U RegAddress, float* RegData);

// Асинхронная FSM (один шаг CAN за вызов DPN_Process)
void DPN_Process();
Boolean DPN_IsBusy();
DPN_FsmResult DPN_GetFsmResult();
void DPN_ResetFsm();

// Старт операций (false — FSM занята)
Boolean DPN_RequestConfig();
Boolean DPN_RequestStop();

#endif /* CONTROLLER_DPN_H_ */
