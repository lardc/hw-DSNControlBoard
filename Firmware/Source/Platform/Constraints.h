#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
#include "DataTable.h"
#include "Global.h"

//Definitions
//
// в мВ
#define SFTST_ALLOWED_VOLTAGE_MIN		0
#define SFTST_ALLOWED_VOLTAGE_MAX		5000
#define SFTST_ALLOWED_VOLTAGE_DEF1		300
#define SFTST_ALLOWED_VOLTAGE_DEF2		3000
//
#define DUT_POSITION_1					1
#define DUT_POSITION_2					2
//
#define SFTY_DELAY_MIN					0
#define SFTY_DELAY_MAX					500
#define SFTY_DELAY_DEF					200
//
#define PMXU_CAN_ID_DEF					100
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
