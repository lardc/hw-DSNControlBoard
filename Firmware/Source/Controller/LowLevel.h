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
void LL_SelfTestCurrentEnable(bool State);
float GetTestVoltage();
bool LL_IsSafetyTrig();
//
// Аппаратный SPI1 для сдвиговых регистров реле
void LL_SPI_WriteByte(Int8U Data);
// Импульс защёлки на CS выбранной платы (BoardIdx: 0=SS1 Input, 1=SS2 OutTherm, 2=SS3 IORelay)
void LL_SPI_LatchBoard(Int8U BoardIdx);
void LL_SPI_TestSS(Int8U BoardIdx);
//
// OE сдвиговых регистров через GPIO_SFT_ENABLE.
// Enable=false → пин притянут к GND (OE разрешён, коммутация активна)
// Enable=true  → пин в high-Z (OE запрещён, все выходы сдвиговых регистров отключены)
void LL_SafetyForceRelaysOff(bool Enable);

#endif //__LOWLEVEL_H
