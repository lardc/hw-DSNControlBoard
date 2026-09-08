#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
#include "DataTable.h"
#include "Global.h"
#include "DeviceObjectDictionary.h"

//Definitions
//
#define DPN_CAN_ID_DEF					100
#define REG_DRV_K_DFLT					1.0f
#define REG_DRV_K_MAX					10.0f
//
#define REG_TEMP_COOLING_MIN			0.0f
#define REG_TEMP_COOLING_MAX			120.0f
#define REG_TEMP_COOLING_DFLT			50.0f
//
#define REG_ADC_DIAG_K_DFLT				1.0f
#define REG_ADC_DIAG_K_MAX				1000.0f
#define REG_ADC_DIAG_OFFSET_MIN			(-1000.0f)
#define REG_ADC_DIAG_OFFSET_MAX			1000.0f
//
#define REG_TEST_VOLTAGE_MAX			1800.0f
#define REG_TEST_CURRENT_MAX			1000.0f
#define REG_TEST_PULSE_US_MAX			100000.0f
#define REG_GATE_V_MAX					30.0f
#define REG_GATE_RG_MAX					20.0f
#define REG_GATE_OPEN_V_DFLT			15.0f
#define REG_GATE_CLOSE_V_DFLT			(-15.0f)
#define REG_GATE_RG_DFLT				10.0f
//
#define NO								false
#define YES								true

// Types
typedef struct __TableItemConstraint
{
	float Min;
	float Max;
	float Default;
} TableItemConstraint;

// Variables
extern const TableItemConstraint Constraint[];

#endif // __CONSTRAINTS_H
