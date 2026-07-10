// -----------------------------------------
// Commutation table
// ----------------------------------------

#ifndef __COMMTABLE_H
#define __COMMTABLE_H

// Include
#include "stdinc.h"

// Constants
//
// MXU303: три физические платы за отдельными CS (SS1/SS2/SS3).
// IORelayBoard объединяет функционал прежних OutHVRelayBoard-1/2 в одной плате.
#define NUM_REGS_INPUT_BOARD		3	// InputRelayBoard — 3 сдвиговых регистра (24 бита) на SS1
#define NUM_REGS_THERM_BOARD		1	// OutThermRelayBoard — 1 сдвиговый регистр (8 бит) на SS2
#define NUM_REGS_IO_BOARD			4	// IORelayBoard — 4 сдвиговых регистра (32 бита) на SS3

#define NUM_REGS_TOTAL				(NUM_REGS_IO_BOARD + NUM_REGS_THERM_BOARD + NUM_REGS_INPUT_BOARD)

#define COMMUTATION_TABLE_SIZE		(NUM_REGS_TOTAL * 8)

//
// Commutation ID (индекс в CommutationTable[]).
// Распределение 64 слотов: 0..31 — IORelayBoard, 32..39 — OutThermRelayBoard,
// 40..63 — InputRelayBoard.
//
// ===== InputRelayBoard =====
#define GT_G_COMM					42 // Затвор
#define GT_GE_COMM					43 // Эмиттер управления
#define GT_GPOT_COMM				47 // Потенциальный затвор
#define GT_GEPOT_COMM				48 // Потенциальный эмиттер управления
#define GT_G_COMM_TO_GE_COMM		44
#define GT_GPOT_COMM_TO_GEPOT_COMM	49

#define SV_G_COMM								52 // Затвор
#define SV_GE_COMM								53 // Эмиттер Управления
#define SV_POT_POS_COMM							57 // Потенциальный +
#define SV_POT_NEG_COMM							58 // Потенциальный -
#define SV_G_COMM_TO_SV_GE_COMM					54
#define SV_POT_POS_COMM_TO_SV_POT_NEG_COMM		59

#define SV_POT_POS_TO_PMXU_POS		60
#define SV_POT_NEG_TO_PMXU_NEG		61
//62-63 - не заняты

#define TEST_IN_TO_GT_G				40
#define TEST_IN_TO_GT_GPOT			45
#define TEST_IN_TO_SV_G				50
#define TEST_IN_TO_SV_POT_POS		55

#define TEST_OUT_TO_GT_GE			41
#define TEST_OUT_TO_GT_GEPOT		46
#define TEST_OUT_TO_SV_GE			51
#define TEST_OUT_TO_SV_POT_NEG		56


// ===== IORelayBoard =====
// Main
#define GT_G_TO_G					18
#define GT_GE_TO_GE					19
#define GT_G_TO_GE					20
#define GT_GE_TO_G					21
#define GT_GEPOT_TO_EPOT			23
#define GT_GPOT_TO_G				22
#define	G_TO_CPOT					30
// 31 - Не занят

#define SV_G_TO_G					24
#define SV_GE_TO_GE					25
#define SV_POT_POS_TO_CPOT			28
#define SV_POT_POS_TO_EPOT			26
#define SV_POT_NEG_TO_CPOT			27
#define SV_POT_NEG_TO_EPOT			29

// OutHVRelayBoard-1
#define GE_TO_GE1					8
#define G_TO_G1						6
#define EPOT_TO_EPOT1				5
#define EPOT_TO_CPOT1				0
#define CPOT_TO_CPOT1				1
#define CPOT_TO_EPOT1				4

#define GE1_TO_G1					7	// Through 1kOhm resitor

#define PE_TO_EPOT1					3
#define PE_TO_CPOT1					2

// OutHVRelayBoard-2
#define GE_TO_GE2					17
#define G_TO_G2						15
#define EPOT_TO_EPOT2				14
#define EPOT_TO_CPOT2				9
#define CPOT_TO_EPOT2				13
#define CPOT_TO_CPOT2				10

#define GE2_TO_G2					16	// Through 1kOhm resitor

#define PE_TO_EPOT2					12
#define PE_TO_CPOT2					11

// ===== OutThermRelayBoard =====
#define GT_G_T2						34
#define GT_G_AND_GT_GPOT_TO_T2		32
#define GT_GPOT_TO_GT_G_T2			35
#define GT_GE_T1					36
#define GT_GE_AND_GT_GEPOT_TO_T1	33
#define GT_GEPOT_TO_GT_GE_T1		37
// 38,39 - не заняты

#define CT_SAVE_TIMEOUT				1800000
// Types
//
typedef struct __CommutationTableItem
{
	Int8U BoardNum;
	Int8U Bit;
	Int8U RegNum;
} CommutationTableItem;

// Variables
//
extern Int32U CycleCounters[COMMUTATION_TABLE_SIZE];
extern Int64U CT_SaveTimer;

#endif // __COMMTABLE_H
