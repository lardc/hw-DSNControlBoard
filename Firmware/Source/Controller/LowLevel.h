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
// Аппаратный SPI1: SCK/DIN для сдвигового регистра CS и для записи в ЦАП
void LL_SPI_WriteByte(Int8U Data);

void LL_Safety(bool Enable);

#endif //__LOWLEVEL_H
