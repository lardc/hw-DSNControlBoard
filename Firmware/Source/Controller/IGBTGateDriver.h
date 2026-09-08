// -----------------------------------------
// Конфигурация IGBTGateDriver H/L
// ----------------------------------------

#ifndef __IGBT_GATE_DRIVER_H
#define __IGBT_GATE_DRIVER_H

#include "stdinc.h"
#include "ZcRegistersDriver.h"

typedef ZcRD_DriverChannel IGBTGD_Channel;

#define IGBTGD_CH_H		ZcRD_DrvH
#define IGBTGD_CH_L		ZcRD_DrvL

// Коды схемы включения
#define SCHEME_SD								17	// Single diode
#define SCHEME_HB								18	// Half-bridge
#define SCHEME_LC								19	// Low chopper
#define SCHEME_HC								20	// High chopper
#define SCHEME_DD								21	// Dual diode

#define TEST_POS_H								0	// Верхний ключ (H)
#define TEST_POS_L								1	// Нижний ключ (L)

// Подбор кода Rg (битмаска BIT0..7) по целевому сопротивлению, Ом
Int8U IGBTGD_FindRgCode(float RgOhm);

// Пересчёт напряжения затвора [В] в код ЦАП (16 бит) с коэффициентом K
Int16U IGBTGD_VoltageToDac(float VoltageAbs, float Coef);

// Запись отпирающего/запирающего напряжения (последовательность CS1/CS2 по ТТ)
void IGBTGD_SetVoltages(IGBTGD_Channel Channel, float OpenV, float CloseV);

// Установка Rg (Ом → код → SR)
void IGBTGD_SetGateResistance(IGBTGD_Channel Channel, float RgOhm);

// MUX: true → ADAPTSwitchBoard; false → затвор DUT
void IGBTGD_SetMux(IGBTGD_Channel Channel, bool ToSwitchBoard);

void IGBTGD_SetFaultLed(IGBTGD_Channel Channel, bool On);

// Полная конфигурация канала
void IGBTGD_Configure(IGBTGD_Channel Channel, float OpenV, float CloseV, float RgOhm,
		bool MuxOn, bool FaultLed);

// Сброс ЦАП и SR канала / обоих
void IGBTGD_Reset(IGBTGD_Channel Channel);
void IGBTGD_ResetAll();

#endif // __IGBT_GATE_DRIVER_H
