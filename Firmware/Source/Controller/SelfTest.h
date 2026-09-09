#ifndef __SELFTEST_H
#define __SELFTEST_H

#include "stdinc.h"

typedef enum __SelfTestSubState
{
	STS_None = 0,

	STS_ProtectionBoard_H_DUT,
	STS_ProtectionBoard_L_DUT,
	STS_ProtectionBoard_H_SwitchBoard,
	STS_ProtectionBoard_L_SwitchBoard,

	STS_GateResistors_H,
	STS_GateResistors_L,

	STS_Amplitude_H_PM20,
	STS_Amplitude_H_M15_P20,
	STS_Amplitude_H_M20_P15,
	STS_Amplitude_L_PM20,
	STS_Amplitude_L_M15_P20,
	STS_Amplitude_L_M20_P15,

	STS_Finish
} SelfTestSubState;

extern SelfTestSubState SELFTEST_SubState;

void SELFTEST_Start();
void SELFTEST_Process();
void SELFTEST_Stop();

#endif // __SELFTEST_H
