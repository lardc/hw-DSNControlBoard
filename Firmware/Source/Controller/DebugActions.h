#ifndef __DEBUGACTIONS_H
#define __DEBUGACTIONS_H

// Include
//
#include "ZwBase.h"

// Functions
//
void DBACT_ToggleFPLed();
void DBACT_WriteSPI();
void DBACT_SelfTestMeasure();
void DBACT_SDEN();
void DBACT_SftEnablePulse();
void DBACT_ReadSftIn();
void DBACT_SpiSSPulse();

#endif //__DEBUGACTIONS_H
