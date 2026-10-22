// Header
#include "StorageDescription.h"
#include "Global.h"

// Variables
RecordDescription StorageDescription[] =
{
	{"REG_DEV_STATE",					DT_Float, 1},
	{"REG_FAULT_REASON",				DT_Float, 1},
	{"REG_DISABLE_REASON",				DT_Float, 1},
	{"REG_WARNING",						DT_Float, 1},
	{"REG_PROBLEM",						DT_Float, 1},
	{"REG_OP_RESULT",					DT_Float, 1},
	{"REG_SELF_TEST_OP_RESULT",			DT_Float, 1},
	{"REG_SUB_STATE",					DT_Float, 1},
	{"REG_TEMPERATURE",					DT_Float, 1},
	{"REG_SELECTED_INDUCTANCE",			DT_Float, 1},
	{"REG_SELECTED_PULSE_WIDTH",		DT_Float, 1},
	{"REG_ADAPTER_MAX_VOLTAGE",			DT_Float, 1},
	{"REG_ADAPTER_MAX_CURRENT",			DT_Float, 1},
	{"REG_EXT_UNIT_ERROR_CODE",			DT_Float, 1},
	{"REG_EXT_UNIT_FUNCTION",			DT_Float, 1},
	{"REG_EXT_UNIT_EXT_DATA",			DT_Float, 1},
};

Int32U TablePointers[sizeof(StorageDescription) / sizeof(StorageDescription[0])] = {0};
const Int16U StorageSize = sizeof(StorageDescription) / sizeof(StorageDescription[0]);

CounterDescription CounterStorageDescription[] =
{
	{"0. ST_Relay_H"},
	{"1. ST_Relay_L"},
	{"2. ST_Relay_HE"},
	{"3. ST_Relay_LE"},
	{"4. CTRL_H- to POT_C"},
	{"5. CTRL_L- to POT_L"},
};
CounterData CounterTablePointers[sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0])] = {0};
const Int16U CounterStorageSize = sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0]);
