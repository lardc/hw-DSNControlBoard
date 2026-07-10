// -----------------------------------------
// Driver for IOExpansion buffers via SPI
// ----------------------------------------

#ifndef __ZBIOEXPANSION_H
#define __ZBIOEXPANSION_H

// Include
#include "stdinc.h"

// Functions
//
// Reset output register to zero
void ZcRD_RegisterReset();
// Write cache to physical register
void ZcRD_RegisterFlushWrite();
// Compose cached values
void ZcRD_OutputValuesCompose(Int16U TableID, Boolean TurnOn);
// Reset cached values to zero
void ZcRD_OutputValuesReset();

#endif // __ZBIOEXPANSION_H
