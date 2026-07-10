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
#include "Diagnostic.h"
#include "BCCIxParams.h"
#include "CommutationTable.h"
#include "ZcRegistersDriver.h"
#include "PMXU.h"
#include "BCCIMHighLevel.h"
#include "SelfTest.h"
#include "SaveToFlash.h"

// Types
//
typedef void (*FUNC_AsyncDelegate)();

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSelfTestState CONTROL_SubState = STS_None;
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
void CONTROL_InitStoragePointers();
void CONTROL_SaveLastRequest(Int16U ActionID);

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
}
//------------------------------------------

void CONTROL_Idle()
{
	if (CONTROL_TimeCounter - CT_SaveTimer >= CT_SAVE_TIMEOUT)
	{
		STF_SaveCounterData();
		CT_SaveTimer = CONTROL_TimeCounter;
	}

	CONTROL_SafetyCheck();

	SELFTEST_Process();
	COMM_Process();

	DEVPROFILE_ProcessRequests();
	CONTROL_UpdateWatchDog();
}
//------------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	if(Reason == DF_PMXU_INTERFACE)
	{
		BHLError Error = BHL_GetError();
		DataTable[REG_EXT_UNIT_ERROR_CODE] = Error.ErrorCode;
		DataTable[REG_EXT_UNIT_FUNCTION] = Error.Func;
		DataTable[REG_EXT_UNIT_EXT_DATA] = Error.ExtData;
	}

	CONTROL_ResetOutputRegisters();
	COMM_Default();
	LL_SelfTestCurrentEnable(false);

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

void CONTROL_SetDeviceSubState(DeviceSelfTestState NewSubState)
{
	CONTROL_SubState = NewSubState;
	DataTable[REG_SUB_STATE] = NewSubState;
}
//------------------------------------------

void CONTROL_ResetToDefaultState()
{
	CONTROL_ResetOutputRegisters();
	COMM_Default();
	LL_SelfTestCurrentEnable(false);

	CONTROL_SetDeviceState(DS_None);
	CONTROL_SetDeviceSubState(STS_None);
}
//------------------------------------------

void CONTROL_InitStoragePointers()
{
	for (Int16U i = 0; i < COMMUTATION_TABLE_SIZE; ++i)
		STF_AssignCounterPointer(i, (Int32U)&CycleCounters[i]);
}
//------------------------------------------

bool CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError)
{
	*pUserError = ERR_NONE;
	
	CONTROL_SafetyCheck();
	CONTROL_SaveLastRequest(ActionID);

	switch (ActionID)
	{
		case ACT_ENABLE_POWER:
			if(CONTROL_State == DS_None)
			{
				if(PMXU_Enable())
				{
					DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_NONE;
					CONTROL_SetDeviceState(DS_InSelfTest);
					CONTROL_SetDeviceSubState(STS_Start);
				}
			}
			else if(CONTROL_State != DS_Enabled)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_DISABLE_POWER:
			if(CONTROL_State == DS_Enabled)
			{
				if(PMXU_Disable())
				{
					COMM_Default();
					CONTROL_SetDeviceState(DS_None);
				}
			}
			else if(CONTROL_State != DS_None)
					*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_CLR_FAULT:
			if (CONTROL_State == DS_Fault)
			{
				if(PMXU_ClearFault() && PMXU_Disable())
				{
					CONTROL_SetDeviceState(DS_None);
					CONTROL_SetDeviceSubState(STS_None);
					DataTable[REG_FAULT_REASON] = DF_NONE;
				}
			}
			break;

		case ACT_CLR_WARNING:
			PMXU_ClearWarning();
			DataTable[REG_WARNING] = WARNING_NONE;
			break;

		case ACT_SET_ACTIVE:
			if(CONTROL_State == DS_Enabled || CONTROL_State == DS_SafetyActive)
			{
				if(PMXU_SafetyActivate())
				{
					if(LL_IsSafetyTrig())
						CONTROL_SetDeviceState(DS_SafetyTrig);
					else
						CONTROL_SetDeviceState(DS_SafetyActive);
				}
			}
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_SET_INACTIVE:
			if(CONTROL_State == DS_Enabled || CONTROL_State == DS_SafetyActive || CONTROL_State == DS_SafetyTrig)
			{
				if(PMXU_SafetyDeactivate())
					CONTROL_SetDeviceState(DS_Enabled);
			}
			else
				*pUserError = ERR_DEVICE_NOT_READY;
			break;

		case ACT_SELFT_TEST:
			if(CONTROL_State == DS_Enabled)
			{
				if(PMXU_IsReady())
				{
					DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_NONE;
					CONTROL_SetDeviceState(DS_InSelfTest);
					CONTROL_SetDeviceSubState(STS_Start);
				}
				else
					*pUserError = ERR_DEVICE_NOT_READY;
			}
			else if(CONTROL_State != DS_Enabled)
				*pUserError = ERR_OPERATION_BLOCKED;
			break;

		case ACT_COMM_IGES_POS_PULSE:
		case ACT_COMM_IGES_NEG_PULSE:
		case ACT_COMM_UGE_TH:
		case ACT_COMM_UCE_SAT:
		case ACT_COMM_UFW_CHOPPER_DIODE:
		case ACT_COMM_ICES_OR_IRRM:
		case ACT_COMM_THERMISTOR:
		case ACT_COMM_NO_PE:
		case ACT_COMM_NONE:
			if (CONTROL_State == DS_Fault)
				*pUserError = ERR_OPERATION_BLOCKED;
			else if(CONTROL_State != DS_Enabled && CONTROL_State != DS_SafetyActive)
				*pUserError = ERR_DEVICE_NOT_READY;
			else
			{
				CONTROL_ResetOutputRegisters();

				bool Validation = COMM_ValidateRequest(ActionID, (Int16U)DataTable[REG_DUT_POSITION]);

				if(Validation)
					COMM_ProcessState = DPS_Start;
			}
			break;
		
		case ACT_SET_COUNTER:
			CycleCounters[(Int16U)DataTable[REG_CNT_NUMBER]] = DataTable[REG_CNT_VALUE];
			break;

		case ACT_SAVE_COUNTERS:
			STF_SaveCounterData();
			break;

		case ACT_ERASE_COUNTERS:
			NFLASH_Unlock();
			STF_EraseCounterDataSector();
			break;

		default:
			return DIAG_HandleDiagnosticAction(ActionID, pUserError);
	}
	return true;
}
//-----------------------------------------------

void CONTROL_SafetyCheck()
{
	if(SafetyFlushPending)
	{
		SafetyFlushPending = false;
		// Запретить OE выходов, затем «кадр нулей», затем вернуть OE активным.
		LL_SafetyForceRelaysOff(true);
		ZcRD_OutputValuesReset();
		ZcRD_RegisterFlushWrite();
		LL_SafetyForceRelaysOff(false);

		// COMM_Default() в ISR не вызываем — тяжёлый 20 мс delay.
		COMM_State = COMM_Def;

		// Fault/state только если safety-режим активирован пользователем
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
		LL_SafetyForceRelaysOff(true);
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
	COMM_Default();
}
//------------------------------------------

void CONTROL_SaveLastRequest(Int16U ActionID)
{
	DataTable[REG_LAST_CMD]  = ActionID;
	DataTable[REG_LAST_POS]  = (Int16U)DataTable[REG_DUT_POSITION];
	DataTable[REG_LAST_CASE] = (Int16U)DataTable[REG_DUT_CASE];
	DataTable[REG_LAST_TYPE] = (Int16U)DataTable[REG_DUT_SCHEME];
}
// ----------------------------------------

void CONTROL_HandleFrontPanelLamp(CommutationState Commutation)
{
	static Int64U FPLampCounter = 0;
	bool Forced = false;
	static CommutationState LastCommutation = 0;

	if(LastCommutation != Commutation)
	{
		LastCommutation = Commutation;
		Forced = true;
	}

	if(CONTROL_State == DS_Fault)
	{
		if(++FPLampCounter > TIME_FP_LED_FAULT_BLINK)
		{
			LL_ToggleFPLed();
			FPLampCounter = 0;
		}
	}
	else
	{
		if(CONTROL_State == DS_InSelfTest)
			LL_SetStateFPLed(true);
		else
		{
			if(CONTROL_State != DS_SafetyTrig)
			{
				if(CONTROL_State == DS_None && FPLampCounter)
				{
					LL_SetStateFPLed(false);
					FPLampCounter = 0;
				}

				if(CONTROL_State != DS_None)
				{
					if(Forced)
					{
						LL_SetStateFPLed(true);
						FPLampCounter = CONTROL_TimeCounter + TIME_FP_LED_ON_STATE;
					}
					else
					{
						if(CONTROL_TimeCounter >= FPLampCounter)
							LL_SetStateFPLed(false);
					}
				}
			}
		}
	}
}
//-----------------------------------------------
