#ifndef __ZBIOEXPANSION_H
#define __ZBIOEXPANSION_H

#include "stdinc.h"

// Канал драйвера
typedef enum __ZcRD_DriverChannel
{
	ZcRD_DrvH = 0,
	ZcRD_DrvL = 1
} ZcRD_DriverChannel;

// Битовые маски SR драйвера (ТТ табл. 3)
#define ZCRD_BIT_R0				(1u << 0)	// 200 Ом
#define ZCRD_BIT_R1				(1u << 1)	// 100 Ом
#define ZCRD_BIT_R2				(1u << 2)	// 51 Ом
#define ZCRD_BIT_R3				(1u << 3)	// 24 Ом
#define ZCRD_BIT_R4				(1u << 4)	// 12 Ом
#define ZCRD_BIT_R5				(1u << 5)	// 6.2 Ом
#define ZCRD_BIT_R6				(1u << 6)	// 3 Ом
#define ZCRD_BIT_R7				(1u << 7)	// 1.5 Ом
#define ZCRD_BIT_FAULT_LED		(1u << 8)
#define ZCRD_BIT_MUX			(1u << 9)	// MUX на ADAPTSwitchBoard
#define ZCRD_BIT_CS1			(1u << 10)	// CS ЦАП1 (0 = выбран)
#define ZCRD_BIT_CS2			(1u << 11)	// CS ЦАП2 (0 = выбран)

#define ZCRD_RG_MASK			(0x00FFu)
#define ZCRD_CS_IDLE			(ZCRD_BIT_CS1 | ZCRD_BIT_CS2)
#define ZCRD_CS1_ACTIVE			(ZCRD_BIT_CS2)			// CS1=0, CS2=1
#define ZCRD_CS2_ACTIVE			(ZCRD_BIT_CS1)			// CS1=1, CS2=0

// Счётчики ресурса (ТТ): 0..3 — ST_Relay на EXP; 4..5 — MUX-реле драйверов H/L
#define ZCRD_RELAY_COUNTERS		6
#define ZCRD_ST_RELAY_COUNT		4
#define ZCRD_ST_RELAY_H_BIT		(1u << 0)
#define ZCRD_ST_RELAY_L_BIT		(1u << 1)
#define ZCRD_ST_RELAY_HE_BIT	(1u << 2)
#define ZCRD_ST_RELAY_LE_BIT	(1u << 3)
#define ZCRD_CNT_MUX_H			4	// CTRL_H- на POT_C (MUX IGBTGateDriverH)
#define ZCRD_CNT_MUX_L			5	// CTRL_L- на POT_L (MUX IGBTGateDriverL)

extern Int32U CycleCounters[ZCRD_RELAY_COUNTERS];

// Запись 16-bit маски в каскад SR драйвера 
void ZcRD_WriteDriverMask(ZcRD_DriverChannel Channel, Int16U Mask);

// Текущая «базовая» маска канала 
Int16U ZcRD_GetDriverMask(ZcRD_DriverChannel Channel);
void ZcRD_SetDriverBaseMask(ZcRD_DriverChannel Channel, Int16U BaseMask);

// Запись кода ЦАП (32 бит, MSB first) 
void ZcRD_WriteDacCode(Int32U Code);

// CS1/CS2: выбрать / снять с учётом базовой маски канала
void ZcRD_SelectDac1(ZcRD_DriverChannel Channel);
void ZcRD_SelectDac2(ZcRD_DriverChannel Channel);
void ZcRD_DeselectDacs(ZcRD_DriverChannel Channel);

// SR расширения портов 
void ZcRD_WriteExpMask(Int16U Mask);
Int16U ZcRD_GetExpMask();
void ZcRD_SetExpBit(Int16U Bit, Boolean On);

// Обнуление масок драйверов и EXP
void ZcRD_ResetAll();

#endif // __ZBIOEXPANSION_H
