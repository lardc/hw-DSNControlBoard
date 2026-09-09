// Header
#include "Controller.h"
//
// Includes
#include "Board.h"
#include "Delay.h"
#include "DeviceProfile.h"
#include "Interrupts.h"
#include "LowLevel.h"
#include "SysConfig.h"
#include "DebugActions.h"
#include "BCCIxParams.h"
#include "ZcRegistersDriver.h"
#include "DPN.h"
#include "Logic.h"
#include "SelfTest.h"
#include "BCCIMHighLevel.h"
#include "SaveToFlash.h"
#include "DeviceObjectDictionary.h"
#include "DataTable.h"

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
static Boolean CycleActive = false;
volatile Int64U CONTROL_TimeCounter = 0;
static volatile bool SafetyFlushPending = false;
//
volatile Int16U CONTROL_DiagCounter = 0;
volatile float CONTROL_DiagData[VALUES_DIAG_SIZE];
volatile Boolean RequestSaveToFlash = false;
static Int64U CounterSaveTimer = 0;
//

// Forward functions
//
bool CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void CONTROL_UpdateWatchDog();
void CONTROL_ResetToDefaultState();
void CONTROL_SafetyCheck();
void CONTROL_SaveLastRequest(Int16U ActionID);
void CONTROL_InitStoragePointers();

// Functions
//
void CONTROL_Init()
{
	Int16U FEPIndexes[FEP_COUNT] = {EP_DiagData};
	Int16U FEPSized[FEP_COUNT] = {VALUES_DIAG_SIZE};
	pInt16U FEPCounters[FEP_COUNT] = {(pInt16U)&CONTROL_DiagCounter};
	pFloat32 FEPDatas[FEP_COUNT] = {(pFloat32)&CONTROL_DiagData};

	// Конфигурация сервиса работы Data-table и EPROM
	EPROMServiceConfig EPROMService = {(FUNC_EPROM_WriteValues)&NFLASH_WriteDT, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};
	// Инициализация data table
	DT_Init(EPROMService, false);
	DT_SaveFirmwareInfo(CAN_NID, 0);
	// Инициализация device profile
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitFEPService(FEPIndexes, FEPSized, FEPCounters, FEPDatas);

	CONTROL_InitStoragePointers();
	STF_LoadCounters();

	// Сброс значений
	DEVPROFILE_ResetControlSection();
	CONTROL_ResetToDefaultState();

	// Принудительная остановка DPN при старте
	DPN_RequestStop();
	CounterSaveTimer = CONTROL_TimeCounter;
}
//------------------------------------------

void CONTROL_Idle()
{
	CONTROL_SafetyCheck();
	LOGIC_Process();

	if(CONTROL_State != DS_InProcess)
	{
		if(RequestSaveToFlash)
		{
			RequestSaveToFlash = false;
			STF_SaveDiagData();
		}
		else if(DataTable[REG_CNT_ACTIVE] && (CONTROL_TimeCounter - CounterSaveTimer) >= COUNTER_SAVE_TIMEOUT_MS)
		{
			STF_SaveCounterData();
			CounterSaveTimer = CONTROL_TimeCounter;
		}
	}

	DEVPROFILE_ProcessRequests();
	CONTROL_UpdateWatchDog();
}
//------------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	if(Reason == DF_DPN_INTERFACE)
	{
		BHLError Error = BHL_GetError();
		DataTable[REG_EXT_UNIT_ERROR_CODE] = Error.ErrorCode;
		DataTable[REG_EXT_UNIT_FUNCTION] = Error.Func;
		DataTable[REG_EXT_UNIT_EXT_DATA] = Error.ExtData;
	}

	DPN_ResetFsm();
	LOGIC_Reset();
	SELFTEST_Stop();
	CONTROL_ResetOutputRegisters();

	CONTROL_SetDeviceState(DS_Fault);
	DataTable[REG_FAULT_REASON] = Reason;
	DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
}
//------------------------------------------

void CONTROL_SetDeviceState(DeviceState NewState)
{
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;
}
//------------------------------------------

void CONTROL_ResetToDefaultState()
{
	CONTROL_ResetOutputRegisters();

	CONTROL_SetDeviceState(DS_None);
}
//------------------------------------------

bool CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError)
{
	*pUserError = ERR_NONE;

	CONTROL_SafetyCheck();

	switch (ActionID)
	{
		case ACT_ENABLE_POWER:
			if(CONTROL_State == DS_None)
			{
				if(DPN_SubState != DPNS_Idle || LOGIC_SubState != SS_Idle || SELFTEST_SubState != STS_None)
					*pUserError = ERR_DEVICE_NOT_READY;
				else if(DPN_Enable())
					CONTROL_SetDeviceState(DS_Enabled);
			}
			else if(CONTROL_State != DS_Enabled)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_DISABLE_POWER:
			if(CONTROL_State == DS_Enabled || CONTROL_State == DS_SafetyActive)
			{
				if(DPN_SubState != DPNS_Idle || LOGIC_SubState != SS_Idle || SELFTEST_SubState != STS_None)
					*pUserError = ERR_DEVICE_NOT_READY;
				else
				{
					LOGIC_Deactivate();
					CONTROL_SetDeviceState(DS_None);
				}
			}
			else if(CONTROL_State != DS_None)
					*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_CLR_FAULT:
			if (CONTROL_State == DS_Fault)
			{
				if(DPN_ClearFault())
				{
					CONTROL_SetDeviceState(DS_None);
					DataTable[REG_FAULT_REASON] = DF_NONE;
				}
			}
			break;

		case ACT_CLR_WARNING:
			DPN_ClearWarning();
			DataTable[REG_WARNING] = WARNING_NONE;
			break;

		case ACT_SET_ACTIVE:
			if(CONTROL_State == DS_Enabled || CONTROL_State == DS_SafetyActive)
			{
				if(LL_IsSafetyTrig())
					CONTROL_SetDeviceState(DS_SafetyTrig);
				else
					CONTROL_SetDeviceState(DS_SafetyActive);
			}
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_SET_INACTIVE:
			if(CONTROL_State == DS_Enabled || CONTROL_State == DS_SafetyActive
					|| CONTROL_State == DS_SafetyTrig || CONTROL_State == DS_InProcess)
			{
				LOGIC_Deactivate();
				DPN_ClearSafetyTrig();
				CONTROL_SetDeviceState(DS_Enabled);
			}
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_CONFIGURE:
			if(CONTROL_State == DS_SafetyActive)
			{
				if(!LOGIC_Configure())
					*pUserError = ERR_DEVICE_NOT_READY;
			}
			else
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_START_PULSE:
			if(CONTROL_State == DS_SafetyActive && LOGIC_IsConfigured())
			{
				if(!LOGIC_StartPulse())
					*pUserError = ERR_DEVICE_NOT_READY;
			}
			else
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_STOP_PULSE:
			if(CONTROL_State == DS_InProcess)
			{
				LOGIC_StopProcess();
				CONTROL_SetDeviceState(DS_SafetyActive);
			}
			else
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_SELF_TEST:
			if(CONTROL_State == DS_Enabled || CONTROL_State == DS_SafetyActive)
			{
				if(LOGIC_SubState != SS_Idle || DPN_SubState != DPNS_Idle || SELFTEST_SubState != STS_None)
					*pUserError = ERR_DEVICE_NOT_READY;
				else
					SELFTEST_Start();
			}
			else
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		default:
			return DBACT_HandleDiagnosticAction(ActionID, pUserError);
	}
	return true;
}
//-----------------------------------------------

void CONTROL_SafetyCheck()
{
	if(SafetyFlushPending)
	{
		SafetyFlushPending = false;
		LL_Safety(true);
		LL_Safety(false);

		if(DataTable[REG_SAFETY_ACTIVE])
		{
			if(CONTROL_State == DS_SafetyActive)
				CONTROL_SetDeviceState(DS_SafetyTrig);
		}
	}
}
//-----------------------------------------------

void CONTROL_SafetyIrqTick()
{
	static bool PrevSafetyTrig = false;
	bool SafetyTrig = LL_IsSafetyTrig();

	if(SafetyTrig && !PrevSafetyTrig)
	{
		SafetyFlushPending = true;
		LL_Safety(true);
	}

	PrevSafetyTrig = SafetyTrig;
}
//-----------------------------------------------

void CONTROL_UpdateWatchDog()
{
	if(BOOT_LOADER_VARIABLE != BOOT_LOADER_REQUEST)
		IWDG_Refresh();
}
//------------------------------------------

void CONTROL_ResetOutputRegisters()
{
	DataTable[REG_FAULT_REASON] = DF_NONE;
	DataTable[REG_DISABLE_REASON] = DF_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_OP_RESULT] = OPRESULT_NONE;
	//
	DEVPROFILE_ResetScopes(0);
	DEVPROFILE_ResetEPReadState();
}
//------------------------------------------

void CONTROL_FinishedWithProblem(Int16U Problem)
{
	DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
	DataTable[REG_PROBLEM] = Problem;
}
//------------------------------------------

void CONTROL_InitStoragePointers()
{
	Int16U i;

	for(i = 0; i < ZCRD_RELAY_COUNTERS; ++i)
		STF_AssignCounterPointer(i, (Int32U)&CycleCounters[i]);

	STF_AssignPointer(0, (Int32U)&DataTable[REG_DEV_STATE]);
	STF_AssignPointer(1, (Int32U)&DataTable[REG_FAULT_REASON]);
	STF_AssignPointer(2, (Int32U)&DataTable[REG_DISABLE_REASON]);
	STF_AssignPointer(3, (Int32U)&DataTable[REG_WARNING]);
	STF_AssignPointer(4, (Int32U)&DataTable[REG_PROBLEM]);
	STF_AssignPointer(5, (Int32U)&DataTable[REG_OP_RESULT]);
	STF_AssignPointer(6, (Int32U)&DataTable[REG_SELF_TEST_OP_RESULT]);
	STF_AssignPointer(7, (Int32U)&DataTable[REG_SUB_STATE]);
	STF_AssignPointer(8, (Int32U)&DataTable[REG_TEMPERATURE]);
	STF_AssignPointer(9, (Int32U)&DataTable[REG_SELECTED_INDUCTANCE]);
	STF_AssignPointer(10, (Int32U)&DataTable[REG_SELECTED_PULSE_WIDTH]);
	STF_AssignPointer(11, (Int32U)&DataTable[REG_ADAPTER_MAX_VOLTAGE]);
	STF_AssignPointer(12, (Int32U)&DataTable[REG_ADAPTER_MAX_CURRENT]);
	STF_AssignPointer(13, (Int32U)&DataTable[REG_EXT_UNIT_ERROR_CODE]);
	STF_AssignPointer(14, (Int32U)&DataTable[REG_EXT_UNIT_FUNCTION]);
	STF_AssignPointer(15, (Int32U)&DataTable[REG_EXT_UNIT_EXT_DATA]);
}
//------------------------------------------

void CONTROL_HandleFrontPanelLamp()
{
	static Int64U FPLampCounter = 0;

	if(CONTROL_State == DS_Fault)
	{
		if(++FPLampCounter > TIME_FP_LED_FAULT_BLINK)
		{
			LL_ToggleFPLed();
			FPLampCounter = 0;
		}
	}
	else if(CONTROL_State == DS_None)
	{
		LL_SetStateFPLed(false);
		FPLampCounter = 0;
	}
	else if(CONTROL_State != DS_SafetyTrig)
	{
		if(CONTROL_TimeCounter >= FPLampCounter)
			LL_SetStateFPLed(false);
	}
}
//-----------------------------------------------
