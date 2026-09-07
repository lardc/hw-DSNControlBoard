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
bool LL_IsSafetyTrig();
//
// Аппаратный SPI3: SCK/DIN для SR и ЦАП IGBTGateDriver
void LL_SPI_WriteByte(Int8U Data);

// При Enable=true — сброс GateCtrl H/L (полный контур safety — следующие этапы)
void LL_Safety(bool Enable);

#endif //__LOWLEVEL_H
