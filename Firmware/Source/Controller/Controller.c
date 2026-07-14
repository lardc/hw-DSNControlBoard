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

	STF_LoadCounters();
	CONTROL_InitStoragePointers();

	// Сброс значений
	DEVPROFILE_ResetControlSection();
	CONTROL_ResetToDefaultState();
}
//------------------------------------------

void CONTROL_Idle()
{
	CONTROL_SafetyCheck();

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
				if(DPN_Enable())
					CONTROL_SetDeviceState(DS_Enabled);
			}
			else if(CONTROL_State != DS_Enabled)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_DISABLE_POWER:
			if(CONTROL_State == DS_Enabled)
			{
				if(DPN_Disable())
					CONTROL_SetDeviceState(DS_None);
			}
			else if(CONTROL_State != DS_None)
					*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_CLR_FAULT:
			if (CONTROL_State == DS_Fault)
			{
				if(DPN_ClearFault() && DPN_Disable())
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
			if(CONTROL_State == DS_Enabled || CONTROL_State == DS_SafetyActive || CONTROL_State == DS_SafetyTrig)
			{
				DPN_ClearSafetyTrig();
				CONTROL_SetDeviceState(DS_Enabled);
			}
			else
				*pUserError = ERR_DEVICE_NOT_READY;
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
	// Добавить необходимые регистры для сохранения
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
