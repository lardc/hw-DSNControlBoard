// -----------------------------------------
// Global definitions
// ----------------------------------------

// Header
#include "CommutationTable.h"

// Variables
//
Int64U CT_SaveTimer = 0;
Int32U CycleCounters[COMMUTATION_TABLE_SIZE] = {0};

/* Визуализация расположения индексов по ЭМ
 *
 * ===== IORelayBoard (BOARD_IO, SS3) — Position 1 =====
 * BOARD_IO, BIT0, REG_IO_1				// 0
 * BOARD_IO, BIT1, REG_IO_1				// 1
 * BOARD_IO, BIT2, REG_IO_1				// 2
 * BOARD_IO, BIT3, REG_IO_1				// 3
 * BOARD_IO, BIT4, REG_IO_1				// 4
 * BOARD_IO, BIT5, REG_IO_1				// 5
 * BOARD_IO, BIT6, REG_IO_1				// 6
 * BOARD_IO, BIT7, REG_IO_1				// 7
 *
 * BOARD_IO, BIT0, REG_IO_2				// 8
 * BOARD_IO, BIT1, REG_IO_2				// 9
 * BOARD_IO, BIT2, REG_IO_2				// 10
 * BOARD_IO, BIT3, REG_IO_2				// 11
 * BOARD_IO, BIT4, REG_IO_2				// 12
 * BOARD_IO, BIT5, REG_IO_2				// 13
 * BOARD_IO, BIT6, REG_IO_2				// 14
 * BOARD_IO, BIT7, REG_IO_2				// 15
 *
 * BOARD_IO, BIT0, REG_IO_3				// 16
 * BOARD_IO, BIT1, REG_IO_3				// 17
 * BOARD_IO, BIT2, REG_IO_3				// 18
 * BOARD_IO, BIT3, REG_IO_3				// 19
 * BOARD_IO, BIT4, REG_IO_3				// 20
 * BOARD_IO, BIT5, REG_IO_3				// 21
 * BOARD_IO, BIT6, REG_IO_3				// 22
 * BOARD_IO, BIT7, REG_IO_3				// 23
 *
 * BOARD_IO, BIT0, REG_IO_4				// 24
 * BOARD_IO, BIT1, REG_IO_4				// 25
 * BOARD_IO, BIT2, REG_IO_4				// 26
 * BOARD_IO, BIT3, REG_IO_4				// 27
 * BOARD_IO, BIT4, REG_IO_4				// 28
 * BOARD_IO, BIT5, REG_IO_4				// 29
 * BOARD_IO, BIT6, REG_IO_4				// 30
 * BOARD_IO, BIT7, REG_IO_4				// 31	Unused
 *
 * ===== OutThermRelayBoard (BOARD_THERM, SS2) =====
 * BOARD_THERM, BIT0, REG_THERM_1		// 32
 * BOARD_THERM, BIT1, REG_THERM_1		// 33
 * BOARD_THERM, BIT2, REG_THERM_1		// 34
 * BOARD_THERM, BIT3, REG_THERM_1		// 35
 * BOARD_THERM, BIT4, REG_THERM_1		// 36
 * BOARD_THERM, BIT5, REG_THERM_1		// 37
 * BOARD_THERM, BIT6, REG_THERM_1		// 38 Unused
 * BOARD_THERM, BIT7, REG_THERM_1		// 39 Unused
 *
 * ===== InputRelayBoard (BOARD_INPUT, SS1) =====
 * BOARD_INPUT, BIT0, REG_INPUT_1		// 40
 * BOARD_INPUT, BIT1, REG_INPUT_1		// 41
 * BOARD_INPUT, BIT2, REG_INPUT_1		// 42
 * BOARD_INPUT, BIT3, REG_INPUT_1		// 43
 * BOARD_INPUT, BIT4, REG_INPUT_1		// 44
 * BOARD_INPUT, BIT5, REG_INPUT_1		// 45
 * BOARD_INPUT, BIT6, REG_INPUT_1		// 46
 * BOARD_INPUT, BIT7, REG_INPUT_1		// 47
 *
 * BOARD_INPUT, BIT0, REG_INPUT_2		// 48
 * BOARD_INPUT, BIT1, REG_INPUT_2		// 49
 * BOARD_INPUT, BIT2, REG_INPUT_2		// 50
 * BOARD_INPUT, BIT3, REG_INPUT_2		// 51
 * BOARD_INPUT, BIT4, REG_INPUT_2		// 52
 * BOARD_INPUT, BIT5, REG_INPUT_2		// 53
 * BOARD_INPUT, BIT6, REG_INPUT_2		// 54
 * BOARD_INPUT, BIT7, REG_INPUT_2		// 55
 *
 * BOARD_INPUT, BIT0, REG_INPUT_3		// 56
 * BOARD_INPUT, BIT1, REG_INPUT_3		// 57
 * BOARD_INPUT, BIT2, REG_INPUT_3		// 58
 * BOARD_INPUT, BIT3, REG_INPUT_3		// 59
 * BOARD_INPUT, BIT4, REG_INPUT_3		// 60
 * BOARD_INPUT, BIT5, REG_INPUT_3		// 61
 * BOARD_INPUT, BIT6, REG_INPUT_3		// 62 Unused
 * BOARD_INPUT, BIT7, REG_INPUT_3		// 63 Unused
 */
