#ifndef __SELFTEST_H
#define __SELFTEST_H

#include "stdinc.h"

typedef enum __SelfTestSubState
{
	STS_None = 0,

	STS_ProtectionBoardHDut,
	STS_ProtectionBoardLDut,
	STS_ProtectionBoardHSwitchBoard,
	STS_ProtectionBoardLSwitchBoard,

	STS_GateResistorsH,
	STS_GateResistorsL,

	STS_AmplitudeHPm20,
	STS_AmplitudeHM15P20,
	STS_AmplitudeHM20P15,
	STS_AmplitudeLPm20,
	STS_AmplitudeLM15P20,
	STS_AmplitudeLM20P15,

	STS_Finish
} SelfTestSubState;

extern SelfTestSubState SELFTEST_SubState;

void SELFTEST_Start();
void SELFTEST_Process();
void SELFTEST_Stop();

#endif // __SELFTEST_H
