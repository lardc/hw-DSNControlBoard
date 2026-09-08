// Header
#include "Logic.h"
//
#include "MemLabel.h"
#include "DS2431.h"
#include "DS18B20.h"
#include "OneWire.h"
#include "DPN.h"
#include "Controller.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "IGBTGateDriver.h"
#include "LowLevel.h"
#include "Global.h"
#include "Board.h"

// Types
//
typedef enum __SubState
{
	SS_Idle = 0,

	SS_CfgScan,
	SS_CfgValidate,
	SS_CfgDpnStart,
	SS_CfgDpnWait,
	SS_CfgDrivers,

	SS_PulseT1,
	SS_PulsePause,
	SS_PulseT2
} SubState;

// Variables
//
LogicAdapterState LOGIC_Adapter = {0};
static SubState CurrentSubState = SS_Idle;
static Boolean Configured = false;
static Int64U CoolingDeadline = 0;
static Int64U Timeout = 0;
static DeviceState StateAfterOp = DS_SafetyActive;

// Forward
//
static void LOGIC_ClearAdapterState();
static void LOGIC_FillProbeFromLabels(LogicProbeInfo *Probe, const MemLabelEntry *Labels, Int8U LabelCount);
static Int16U LOGIC_GetDeviceKind(const MemLabelEntry *Labels, Int8U LabelCount);
static Boolean LOGIC_CheckTemperaturePresent();
static void LOGIC_SetSubState(Int16U Value);
static void LOGIC_FinishOk();
static void LOGIC_FinishProblem(Int16U Problem);
static void LOGIC_ProcessCooling();
static void LOGIC_ProcessOp();
static Boolean LOGIC_ValidateAdapterForTest();
static Int8U LOGIC_RequiredSwitchBoards(Int16U Scheme);
static void LOGIC_ApplyDrivers();
static void LOGIC_SetGateCtrl(Boolean On);
static Boolean LOGIC_PulseSafetyFault();
static Boolean LOGIC_IsPulseSubState(SubState State);

// Functions
//
static void LOGIC_ClearAdapterState()
{
	LOGIC_Adapter.Cached = false;
	LOGIC_Adapter.TemperaturePresent = false;
	LOGIC_Adapter.ProbeCount = 0;
	LOGIC_Adapter.GateDriverCount = 0;
	LOGIC_Adapter.SwitchBoardCount = 0;
	LOGIC_Adapter.Probe.Present = false;
	LOGIC_Adapter.Probe.Ds2431Index = 0;
	LOGIC_Adapter.Probe.AdapterCode = 0;
	LOGIC_Adapter.Probe.MaxVoltage = 0;
	LOGIC_Adapter.Probe.MaxCurrent = 0;
	LOGIC_Adapter.Probe.Serial = 0;
	LOGIC_Adapter.Probe.Version = 0;
}
//-----------------------------------------------

static Int16U LOGIC_GetDeviceKind(const MemLabelEntry *Labels, Int8U LabelCount)
{
	Int8U Index;

	for(Index = 0; Index < LabelCount; ++Index)
	{
		if(Labels[Index].Type == ML_Device)
			return Labels[Index].Value;
	}

	return 0xFFFF;
}
//-----------------------------------------------

static void LOGIC_FillProbeFromLabels(LogicProbeInfo *Probe, const MemLabelEntry *Labels, Int8U LabelCount)
{
	Int8U Index;

	for(Index = 0; Index < LabelCount; ++Index)
	{
		switch(Labels[Index].Type)
		{
			case ML_AdapterCode:
				Probe->AdapterCode = Labels[Index].Value;
				break;
			case ML_MaxVoltage:
				Probe->MaxVoltage = Labels[Index].Value;
				break;
			case ML_MaxCurrent:
				Probe->MaxCurrent = Labels[Index].Value;
				break;
			case ML_SerialNumber:
				Probe->Serial = Labels[Index].Value;
				break;
			case ML_Version:
				Probe->Version = Labels[Index].Value;
				break;
			default:
				break;
		}
	}
}
//-----------------------------------------------

static Boolean LOGIC_CheckTemperaturePresent()
{
	Int16U FoundCount = 0;

	if(!OneWire_SearchFamily(DS18B20_FAMILY_CODE, &FoundCount, NULL) || FoundCount == 0)
	{
		LOGIC_Adapter.TemperaturePresent = false;
		return false;
	}

	LOGIC_Adapter.TemperaturePresent = true;
	return true;
}
//-----------------------------------------------

static void LOGIC_SetSubState(Int16U Value)
{
	DataTable[REG_SUB_STATE] = Value;
}
//-----------------------------------------------

static void LOGIC_FinishOk()
{
	CurrentSubState = SS_Idle;
	LOGIC_SetSubState(SS_Idle);
	DataTable[REG_OP_RESULT] = OPRESULT_OK;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	CONTROL_SetDeviceState(StateAfterOp);
}
//-----------------------------------------------

static void LOGIC_FinishProblem(Int16U Problem)
{
	CurrentSubState = SS_Idle;
	LOGIC_SetSubState(SS_Idle);
	CONTROL_FinishedWithProblem(Problem);
	CONTROL_SetDeviceState(StateAfterOp);
}
//-----------------------------------------------

static Int8U LOGIC_RequiredSwitchBoards(Int16U Scheme)
{
	switch(Scheme)
	{
		case SCHEME_HB:
			return 0;
		case SCHEME_HC:
		case SCHEME_LC:
		case SCHEME_SD:
			return 1;
		case SCHEME_DD:
			return 2;
		default:
			return 0xFF;
	}
}
//-----------------------------------------------

static Boolean LOGIC_ValidateAdapterForTest()
{
	Int16U Scheme = (Int16U)DataTable[REG_TEST_SCHEME];
	Int8U RequiredSw;

	if(!LOGIC_Adapter.Cached
			|| LOGIC_Adapter.ProbeCount != 1
			|| LOGIC_Adapter.GateDriverCount != LOGIC_REQ_GATE_DRIVERS
			|| !LOGIC_Adapter.TemperaturePresent)
	{
		LOGIC_FinishProblem(PROBLEM_OW_INVENTORY);
		return false;
	}

	RequiredSw = LOGIC_RequiredSwitchBoards(Scheme);
	if(RequiredSw == 0xFF || LOGIC_Adapter.SwitchBoardCount != RequiredSw)
	{
		LOGIC_FinishProblem(PROBLEM_VALIDATE_SCHEME);
		return false;
	}

	DataTable[REG_ADAPTER_MAX_VOLTAGE] = LOGIC_Adapter.Probe.MaxVoltage;
	DataTable[REG_ADAPTER_MAX_CURRENT] = LOGIC_Adapter.Probe.MaxCurrent;

	if(DataTable[REG_TEST_VOLTAGE] > LOGIC_Adapter.Probe.MaxVoltage)
	{
		LOGIC_FinishProblem(PROBLEM_VALIDATE_VOLTAGE);
		return false;
	}

	if(DataTable[REG_TEST_CURRENT] > LOGIC_Adapter.Probe.MaxCurrent)
	{
		LOGIC_FinishProblem(PROBLEM_VALIDATE_CURRENT);
		return false;
	}

	return true;
}
//-----------------------------------------------

static void LOGIC_ApplyDrivers()
{
	Int16U Scheme = (Int16U)DataTable[REG_TEST_SCHEME];
	Int16U Pos = (Int16U)DataTable[REG_TEST_POS];
	Boolean MuxH = false;
	Boolean MuxL = false;
	float OpenV = DataTable[REG_GATE_OPEN_V];
	float CloseV = DataTable[REG_GATE_CLOSE_V];

	switch(Scheme)
	{
		case SCHEME_HC:
			MuxL = true;
			break;
		case SCHEME_LC:
			MuxH = true;
			break;
		case SCHEME_SD:
			if(Pos == TEST_POS_H)
				MuxL = true;
			else
				MuxH = true;
			break;
		case SCHEME_HB:
		case SCHEME_DD:
		default:
			break;
	}

	IGBTGD_Configure(IGBTGD_CH_H, OpenV, CloseV, DataTable[REG_GATE_RG_H], MuxH, false);
	IGBTGD_Configure(IGBTGD_CH_L, OpenV, CloseV, DataTable[REG_GATE_RG_L], MuxL, false);
}
//-----------------------------------------------

static void LOGIC_SetGateCtrl(Boolean On)
{
	Int16U Pos = (Int16U)DataTable[REG_TEST_POS];

	if(Pos == TEST_POS_H)
	{
		LL_SetGateCtrlH(On);
		LL_SetGateCtrlL(false);
	}
	else
	{
		LL_SetGateCtrlL(On);
		LL_SetGateCtrlH(false);
	}
}
//-----------------------------------------------

static Boolean LOGIC_IsPulseSubState(SubState State)
{
	return (State == SS_PulseT1 || State == SS_PulsePause || State == SS_PulseT2);
}
//-----------------------------------------------

static Boolean LOGIC_PulseSafetyFault()
{
	if(!LL_IsSafetyTrig())
		return false;

	LOGIC_StopProcess();
	CONTROL_SwitchToFault(DF_SAFETY);
	return true;
}
//-----------------------------------------------

static void LOGIC_ProcessCooling()
{
	float Temperature;

	if(CONTROL_TimeCounter < CoolingDeadline)
		return;

	CoolingDeadline = CONTROL_TimeCounter + TIME_COOLING_PERIOD_MS;

	if(!LOGIC_Adapter.TemperaturePresent || !LOGIC_ReadTemperature(&Temperature))
	{
		LL_SetFan(false);
		DataTable[REG_TEMPERATURE] = 0;
		return;
	}

	DataTable[REG_TEMPERATURE] = Temperature;
	LL_SetFan(Temperature >= DataTable[REG_TEMP_COOLING]);
}
//-----------------------------------------------

static void LOGIC_ProcessOp()
{
	switch(CurrentSubState)
	{
		case SS_Idle:
			break;

		case SS_CfgScan:
			LOGIC_SetSubState(SS_CfgScan);
			if(!LOGIC_ScanAdapter())
			{
				if(!LOGIC_Adapter.TemperaturePresent)
					LOGIC_FinishProblem(PROBLEM_OW_TEMP);
				else if(LOGIC_Adapter.ProbeCount == 0 && LOGIC_Adapter.GateDriverCount == 0)
					LOGIC_FinishProblem(PROBLEM_OW_BUS);
				else
					LOGIC_FinishProblem(PROBLEM_OW_INVENTORY);
			}
			else
				CurrentSubState = SS_CfgValidate;
			break;

		case SS_CfgValidate:
			LOGIC_SetSubState(SS_CfgValidate);
			if(LOGIC_ValidateAdapterForTest())
				CurrentSubState = SS_CfgDpnStart;
			break;

		case SS_CfgDpnStart:
			LOGIC_SetSubState(SS_CfgDpnStart);
			if(DPN_IsBusy())
				break;
			if(!DPN_RequestConfig())
				LOGIC_FinishProblem(PROBLEM_DPN_NOT_READY);
			else
				CurrentSubState = SS_CfgDpnWait;
			break;

		case SS_CfgDpnWait:
			LOGIC_SetSubState(SS_CfgDpnWait);
			if(DPN_IsBusy())
				break;

			switch(DPN_GetFsmResult())
			{
				case DPN_FR_Ok:
					CurrentSubState = SS_CfgDrivers;
					break;
				case DPN_FR_Fail:
					Configured = false;
					CurrentSubState = SS_Idle;
					LOGIC_SetSubState(SS_Idle);
					if(CONTROL_State != DS_Fault)
						CONTROL_SetDeviceState(StateAfterOp);
					break;
				default:
					LOGIC_FinishProblem(PROBLEM_DPN_NOT_READY);
					break;
			}
			break;

		case SS_CfgDrivers:
			LOGIC_SetSubState(SS_CfgDrivers);
			LOGIC_ApplyDrivers();
			Configured = true;
			LOGIC_FinishOk();
			break;

		case SS_PulseT1:
			LOGIC_SetSubState(SS_PulseT1);
			if(LOGIC_PulseSafetyFault())
				break;
			if(CONTROL_TimeCounter > Timeout)
			{
				LOGIC_SetGateCtrl(false);
				Timeout = CONTROL_TimeCounter + (Int32U)DataTable[REG_TEST_PULSE_PAUSE];
				CurrentSubState = SS_PulsePause;
			}
			break;

		case SS_PulsePause:
			LOGIC_SetSubState(SS_PulsePause);
			if(LOGIC_PulseSafetyFault())
				break;
			if(CONTROL_TimeCounter > Timeout)
			{
				LOGIC_SetGateCtrl(true);
				Timeout = CONTROL_TimeCounter + (Int32U)DataTable[REG_TEST_SECOND_PULSE];
				CurrentSubState = SS_PulseT2;
			}
			break;

		case SS_PulseT2:
			LOGIC_SetSubState(SS_PulseT2);
			if(LOGIC_PulseSafetyFault())
				break;
			if(CONTROL_TimeCounter > Timeout)
			{
				LOGIC_StopProcess();
				LOGIC_FinishOk();
			}
			break;

		default:
			CurrentSubState = SS_Idle;
			break;
	}
}
//-----------------------------------------------

void LOGIC_Process()
{
	LOGIC_ProcessCooling();
	DPN_Process();
	LOGIC_ProcessOp();
}
//-----------------------------------------------

Boolean LOGIC_ScanAdapter()
{
	Int16U DeviceCount;
	Int16U DeviceIndex;

	LOGIC_ClearAdapterState();

	DS2431_Init();
	DeviceCount = DS2431_GetDeviceCount();

	for(DeviceIndex = 0; DeviceIndex < DeviceCount; ++DeviceIndex)
	{
		MemLabelEntry Labels[MEM_LABEL_MAX_LABELS];
		Int8U LabelCount;
		Int16U DeviceKind;

		LabelCount = MemLabel_Read((Int8U)DeviceIndex, Labels, MEM_LABEL_MAX_LABELS);
		if(LabelCount == 0)
			continue;

		DeviceKind = LOGIC_GetDeviceKind(Labels, LabelCount);
		if(DeviceKind == 0xFFFF)
			continue;

		switch(DeviceKind)
		{
			case LOGIC_ML_PROBE:
				if(LOGIC_Adapter.ProbeCount == 0)
				{
					LOGIC_Adapter.Probe.Present = true;
					LOGIC_Adapter.Probe.Ds2431Index = (Int8U)DeviceIndex;
					LOGIC_FillProbeFromLabels(&LOGIC_Adapter.Probe, Labels, LabelCount);
					LOGIC_Adapter.ProbeCount = 1;
				}
				break;

			case LOGIC_ML_GATE_DRIVER:
				if(LOGIC_Adapter.GateDriverCount < LOGIC_REQ_GATE_DRIVERS)
					LOGIC_Adapter.GateDriverCount++;
				break;

			case LOGIC_ML_SWITCH_BOARD:
				if(LOGIC_Adapter.SwitchBoardCount < LOGIC_MAX_SWITCH_BOARDS)
					LOGIC_Adapter.SwitchBoardCount++;
				break;

			default:
				break;
		}
	}

	LOGIC_CheckTemperaturePresent();

	LOGIC_Adapter.Cached =
			(LOGIC_Adapter.ProbeCount == 1)
			&& (LOGIC_Adapter.GateDriverCount == LOGIC_REQ_GATE_DRIVERS)
			&& LOGIC_Adapter.TemperaturePresent
			&& (LOGIC_Adapter.SwitchBoardCount <= LOGIC_MAX_SWITCH_BOARDS);

	return LOGIC_Adapter.Cached;
}
//-----------------------------------------------

Boolean LOGIC_RefreshAdapter()
{
	MemLabelEntry Labels[MEM_LABEL_MAX_LABELS];
	Int8U LabelCount;
	Int16U DeviceKind;

	if(!LOGIC_Adapter.Cached || !LOGIC_Adapter.Probe.Present)
		return LOGIC_ScanAdapter();

	LabelCount = MemLabel_Read(LOGIC_Adapter.Probe.Ds2431Index, Labels, MEM_LABEL_MAX_LABELS);
	if(LabelCount == 0)
		return LOGIC_ScanAdapter();

	DeviceKind = LOGIC_GetDeviceKind(Labels, LabelCount);
	if(DeviceKind != LOGIC_ML_PROBE)
		return LOGIC_ScanAdapter();

	LOGIC_FillProbeFromLabels(&LOGIC_Adapter.Probe, Labels, LabelCount);
	return true;
}
//-----------------------------------------------

Boolean LOGIC_IsAdapterReady()
{
	return LOGIC_Adapter.Cached;
}
//-----------------------------------------------

Boolean LOGIC_ReadTemperature(pFloat32 Temperature)
{
	float Value;

	if(!Temperature)
		return false;

	if(!DS18B20_ReadTemperature(&Value))
		return false;

	*Temperature = Value;
	return true;
}
//-----------------------------------------------

Boolean LOGIC_IsConfigured()
{
	return Configured;
}
//-----------------------------------------------

Boolean LOGIC_IsBusy()
{
	return (CurrentSubState != SS_Idle);
}
//-----------------------------------------------

Boolean LOGIC_Configure()
{
	if(CurrentSubState != SS_Idle || DPN_IsBusy())
		return false;

	Configured = false;
	DataTable[REG_OP_RESULT] = OPRESULT_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	StateAfterOp = DS_SafetyActive;
	CONTROL_SetDeviceState(DS_InProcess);
	CurrentSubState = SS_CfgScan;
	return true;
}
//-----------------------------------------------

Boolean LOGIC_StartPulse()
{
	if(CurrentSubState != SS_Idle || !Configured)
		return false;

	if(CONTROL_State != DS_SafetyActive)
		return false;

	DataTable[REG_OP_RESULT] = OPRESULT_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	StateAfterOp = DS_SafetyActive;
	CONTROL_SetDeviceState(DS_InProcess);

	LL_SetScopeSync(true);
	LOGIC_SetGateCtrl(true);
	Timeout = CONTROL_TimeCounter + (Int32U)DataTable[REG_SELECTED_PULSE_WIDTH];
	CurrentSubState = SS_PulseT1;
	LOGIC_SetSubState(SS_PulseT1);
	return true;
}
//-----------------------------------------------

void LOGIC_StopProcess()
{
	LL_SetGateCtrlH(false);
	LL_SetGateCtrlL(false);
	LL_SetScopeSync(false);

	if(LOGIC_IsPulseSubState(CurrentSubState)
			|| CurrentSubState == SS_CfgScan || CurrentSubState == SS_CfgValidate
			|| CurrentSubState == SS_CfgDpnStart || CurrentSubState == SS_CfgDpnWait
			|| CurrentSubState == SS_CfgDrivers)
	{
		CurrentSubState = SS_Idle;
		LOGIC_SetSubState(SS_Idle);
	}
}
//-----------------------------------------------

void LOGIC_Reset()
{
	LOGIC_StopProcess();
	CurrentSubState = SS_Idle;
	Configured = false;
	LOGIC_SetSubState(SS_Idle);
}
//-----------------------------------------------

void LOGIC_Deactivate()
{
	LOGIC_StopProcess();
	Configured = false;
	IGBTGD_ResetAll();
	LOGIC_SetSubState(SS_Idle);
	CurrentSubState = SS_Idle;

	if(DPN_IsBusy())
		DPN_ResetFsm();

	DPN_RequestStop();
}
//-----------------------------------------------
