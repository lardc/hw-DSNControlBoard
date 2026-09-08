#ifndef __CONSTRAINTS_H
#define __CONSTRAINTS_H

// Include
#include "stdinc.h"
#include "DataTable.h"
#include "Global.h"

//Definitions
//
#define DPN_CAN_ID_DEF					100
#define REG_DRV_K_DFLT					1.0f
#define REG_DRV_K_MAX					10.0f
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
