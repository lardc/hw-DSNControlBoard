#ifndef __LOGIC_H
#define __LOGIC_H

#include "stdinc.h"

// ML_Device (ТТ): 0 — Probe, 1 — IGBTGateDriver, 2 — ADAPTSwitchBoard
#define LOGIC_ML_PROBE				0
#define LOGIC_ML_GATE_DRIVER		1
#define LOGIC_ML_SWITCH_BOARD		2

#define LOGIC_REQ_GATE_DRIVERS		2
#define LOGIC_MAX_SWITCH_BOARDS		2

typedef enum __LogicSubState
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
} LogicSubState;

typedef struct __LogicProbeInfo
{
	Int8U Ds2431Index;
	Int16U AdapterCode;
	Int16U MaxVoltage;
	Int16U MaxCurrent;
	Int16U Serial;
	Int16U Version;
	Boolean Present;
} LogicProbeInfo;

typedef struct __LogicSwitchBoardInfo
{
	Int16U MaxVoltage;
	Int16U MaxCurrent;
} LogicSwitchBoardInfo;

typedef struct __LogicAdapterState
{
	Boolean Cached;
	Boolean TemperaturePresent;
	Int8U ProbeCount;
	Int8U GateDriverCount;
	Int8U SwitchBoardCount;
	LogicProbeInfo Probe;
	LogicSwitchBoardInfo SwitchBoard[LOGIC_MAX_SWITCH_BOARDS];
} LogicAdapterState;

extern LogicAdapterState LOGIC_Adapter;
extern LogicSubState LOGIC_SubState;

void LOGIC_Process();

// Поиск DS2431 + разбор MemLabel + наличие DS18B20
Boolean LOGIC_ScanAdapter();

// Повторное чтение кэшированного Probe; иначе LOGIC_ScanAdapter
Boolean LOGIC_RefreshAdapter();

Boolean LOGIC_IsAdapterReady();
Boolean LOGIC_ReadTemperature(pFloat32 Temperature);

// Старт операций (из Dispatch)
Boolean LOGIC_Configure();
Boolean LOGIC_StartPulse();
void LOGIC_StopProcess();
void LOGIC_Deactivate();
void LOGIC_Reset();

Boolean LOGIC_IsConfigured();

#endif // __LOGIC_H
