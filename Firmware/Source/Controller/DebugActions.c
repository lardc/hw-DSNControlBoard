// Header
#include "DebugActions.h"

// Include
//
#include "LowLevel.h"
#include "Board.h"
#include "Delay.h"
#include "Controller.h"
#include "DataTable.h"
#include "ZcRegistersDriver.h"
#include "CommutationTable.h"

// Functions
//
// Send pulse to Front Panel LED
void DBACT_ToggleFPLed()
{
	LL_SetStateFPLed(true);
	DELAY_MS(1000);
	LL_SetStateFPLed(false);
}
//-----------------------

void DBACT_WriteSPI()
{
	// Чтение номера таблицы коммутации из отладочного регистра
	ZcRD_OutputValuesCompose(DataTable[REG_DBG], TRUE);
	// Коммутация выбранной комбинации
	ZcRD_RegisterFlushWrite();
}
//-----------------------

// Turn self-test voltage ON, measure voltage with ADC, compare result with DataTable constant
void DBACT_SelfTestMeasure()
{
	LL_SelfTestCurrentEnable(true);
	DELAY_MS(100);
	DataTable[REG_DBG] = GetTestVoltage();
	DELAY_MS(100);
	LL_SelfTestCurrentEnable(false);
}
//-----------------------

void DBACT_SDEN()
{
	LL_SelfTestCurrentEnable(true);
	DELAY_MS(100);
	LL_SelfTestCurrentEnable(false);
}
//-----------------------

// Импульс SFT_ENABLE: на 1 с переводим пин в high-Z (OE сдвиговых регистров запрещён),
// затем возвращаем активное состояние (OD к GND, OE разрешён).
void DBACT_SftEnablePulse()
{
	LL_SafetyForceRelaysOff(true);
	DELAY_MS(1000);
	LL_SafetyForceRelaysOff(false);
}
//-----------------------

void DBACT_ReadSftIn()
{
	DataTable[REG_DBG] = LL_IsSafetyTrig() ? 1 : 0;
}
//-----------------------

void DBACT_SpiSSPulse()
{
	Int16U Select = (Int16U)DataTable[REG_DBG];
	if (Select < 1 || Select > 3)
		return;
	LL_SPI_TestSS((Int8U)(Select - 1));
}
//-----------------------
