// Header
#include "Logic.h"
//
#include "MemLabel.h"
#include "DS2431.h"
#include "DS18B20.h"
#include "OneWire.h"
#include "DPN.h"

// Variables
//
LogicAdapterState LOGIC_Adapter = {0};

// Forward
//
static void LOGIC_ClearAdapterState();
static void LOGIC_FillProbeFromLabels(LogicProbeInfo *Probe, const MemLabelEntry *Labels, Int8U LabelCount);
static Int16U LOGIC_GetDeviceKind(const MemLabelEntry *Labels, Int8U LabelCount);
static Boolean LOGIC_CheckTemperaturePresent();

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

void LOGIC_Process()
{
	DPN_Process();
	// Cooling / config / pulse FSM — этап 7
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
