#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

// Include
#include "Board.h"
#include "stdinc.h"

// Functions
//
void LL_ToggleBoardLED();
void LL_SetStateFPLed(bool State);
void LL_ToggleFPLed();

// SPI3
void LL_SPI_WriteByte(Int8U Data);
void LL_SPI_LatchRckH();
void LL_SPI_LatchRckL();
void LL_SPI_LatchRckExp();

void LL_SetSpiSckMux(bool ToDac);

// Формирование и индикация
void LL_SetGateCtrlH(bool State);
void LL_SetGateCtrlL(bool State);
void LL_SetFan(bool State);
void LL_SetScopeSync(bool State);

// OE SR расширения
void LL_SetOeExp(bool OutputsEnabled);

// Диагностика затворной цепи
void LL_SetStDischarge(bool State);
void LL_SetStStart(bool State);

// Безопасность (Trig = низкий уровень на SFT_IN, см. ТТ)
bool LL_IsSafetyTrig();
void LL_SafetyIgnore(bool Ignore);

void LL_Safety(bool Enable);

// АЦП диагностики силовых линий: мВ на входе АЦП (далее ×K + Offset → REG_ADC_DIAG_RAW)
float LL_MeasureAdcDiag();

// TIM2 CH2 (PA1): длительность высокого импульса SelfTestResult, мкс
// pTimeMs — системный тик 1 мс (например &CONTROL_TimeCounter); TimeoutMs — таймаут фронтов
Boolean LL_CaptureMeasureHighPulseUs(volatile Int64U *pTimeMs, Int32U TimeoutMs, pInt32U pWidthUs);

// Неблокирующий capture (FSM SelfTest): арм фронта / опрос готовности
void LL_CaptureArmEdge(Boolean Falling);
Boolean LL_CaptureEdgeReady(pInt16U pStamp);

#endif //__LOWLEVEL_H
