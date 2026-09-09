#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
#include "DataTable.h"
#include "Global.h"
#include "DeviceObjectDictionary.h"

// Definitions
//
#define DPN_CAN_ID_DEF					100				//

#define DRV_K_DEF						1.0f			//
#define DRV_K_MAX						10.0f			//
//
#define TEMP_COOLING_MIN				0.0f			// °C
#define TEMP_COOLING_MAX				120.0f			// °C
#define TEMP_COOLING_DEF				50.0f			// °C
//
#define ADC_DIAG_K_DEF					1.0f			//
#define ADC_DIAG_K_MAX					1000.0f			//
#define ADC_DIAG_OFFSET_MIN				(-1000.0f)		//
#define ADC_DIAG_OFFSET_MAX				1000.0f			//
//
#define TEST_VOLTAGE_MAX				1800.0f			// В
#define TEST_CURRENT_MAX				1000.0f			// А
#define TEST_PULSE_MS_MAX				100000.0f		// мс
#define GATE_V_MAX						30.0f			// В
#define GATE_RG_MAX						20.0f			// Ом
#define GATE_OPEN_V_DEF					15.0f			// В
#define GATE_CLOSE_V_DEF				(-15.0f)		// В
#define GATE_RG_DEF						10.0f			// Ом
//
#define ST_OPEN_V_DEF					20.0f			// В
#define ST_CLOSE_V_DEF					(-20.0f)		// В
#define ST_CHARGE_TIME_DEF				2.0f			// мс
#define ST_CAPTURE_TIMEOUT_DEF			2.0f			// мс
#define ST_PULSE_MAX_DURATION_DEF		2000.0f			// мкс
#define ST_RG_DURATION_TOL_DEF			200.0f			// мкс
#define ST_RG_0_DURATION_DEF			500.0f			// мкс
#define ST_RG_1_DURATION_DEF			500.0f			// мкс
#define ST_RG_2_DURATION_DEF			500.0f			// мкс
#define ST_RG_3_DURATION_DEF			500.0f			// мкс
#define ST_RG_4_DURATION_DEF			500.0f			// мкс
#define ST_RG_5_DURATION_DEF			500.0f			// мкс
#define ST_RG_6_DURATION_DEF			500.0f			// мкс
#define ST_RG_7_DURATION_DEF			500.0f			// мкс
#define ST_OPEN_V_MAX					30.0f			// В
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
