// Header
//
#include "SelfTest.h"

// Includes
//
#include "Controller.h"
#include "DataTable.h"
#include "Delay.h"
#include "LowLevel.h"
#include "CommutationTable.h"
#include "ZcRegistersDriver.h"
#include "Commutator.h"

// Types
typedef enum __CheckRelayStages
{
	CRS_Init,
	CRS_CheckClosedCircuit,
	CRS_RelaySwitch,
	CRS_Finish
} CheckRelayStages;
typedef enum __SelfTestProcess
{
	STP_InProcess,
	STP_Finished,
	STP_FailedClosedCheck,
	STP_FailedOpenedCheck
} SelfTestProcess;

// Macro
#define SELFTEST_RelayCheck_macro(arr, failed_pointer) \
	SELFTEST_RelayCheck((arr), sizeof(arr) / sizeof((arr)[0]), failed_pointer)

// Variables
//
CheckRelayStages RelayStages = CRS_Init;

const Int8U SelfTestInputBoard_Stage0[] = {GT_G_COMM, GT_GE_COMM, GT_G_COMM_TO_GE_COMM, TEST_IN_TO_GT_G, TEST_OUT_TO_GT_GE};
const Int8U SelfTestInputBoard_Stage1[] = {GT_GPOT_COMM, GT_GEPOT_COMM, GT_GPOT_COMM_TO_GEPOT_COMM, TEST_IN_TO_GT_GPOT, TEST_OUT_TO_GT_GEPOT};
const Int8U SelfTestInputBoard_Stage2[] = {SV_G_COMM, SV_GE_COMM, SV_G_COMM_TO_SV_GE_COMM, TEST_IN_TO_SV_G, TEST_OUT_TO_SV_GE};
const Int8U SelfTestInputBoard_Stage3[] = {SV_POT_POS_COMM, SV_POT_NEG_COMM, SV_POT_POS_COMM_TO_SV_POT_NEG_COMM,
											TEST_IN_TO_SV_POT_POS, TEST_OUT_TO_SV_POT_NEG};

const Int8U SelfTestThermBoard_Stage0[] = {TEST_IN_TO_GT_G, TEST_OUT_TO_GT_GEPOT, GT_G_TO_G, GT_GE_TO_GE, G_TO_G1, GE_TO_GE1, GE1_TO_G1,
											GT_GE_T1, GT_GEPOT_TO_GT_GE_T1};
const Int8U SelfTestThermBoard_Stage1[] = {TEST_IN_TO_GT_GPOT, TEST_OUT_TO_GT_GE, GT_G_TO_GE, GT_GE_TO_G, G_TO_G2, GE_TO_GE2, GE2_TO_G2,
											GT_G_T2, GT_GPOT_TO_GT_G_T2};

const Int8U SelfTestHV1Board_Stage0[] = {TEST_IN_TO_GT_G, TEST_OUT_TO_GT_GEPOT, GT_G_TO_G, GT_GEPOT_TO_EPOT, G_TO_CPOT, CPOT_TO_EPOT1, EPOT_TO_EPOT1};
const Int8U SelfTestHV1Board_Stage1[] = {TEST_IN_TO_SV_POT_POS, TEST_OUT_TO_SV_POT_NEG, SV_POT_POS_TO_EPOT, SV_POT_NEG_TO_CPOT, EPOT_TO_CPOT1,
										CPOT_TO_CPOT1};
const Int8U SelfTestHV1Board_Stage2[] = {TEST_IN_TO_GT_GPOT, TEST_OUT_TO_GT_GE, GT_GE_TO_GE, GT_GPOT_TO_G, G_TO_G1, GE1_TO_G1, GE_TO_GE1};

const Int8U SelfTestHV2Board_Stage0[] = {TEST_IN_TO_GT_GPOT, TEST_OUT_TO_GT_GEPOT, GT_GPOT_TO_G, GT_GEPOT_TO_EPOT, G_TO_CPOT, CPOT_TO_EPOT2,
											EPOT_TO_EPOT2};
const Int8U SelfTestHV2Board_Stage1[] = {TEST_IN_TO_SV_POT_POS, TEST_OUT_TO_SV_POT_NEG, SV_POT_POS_TO_CPOT, SV_POT_NEG_TO_EPOT, EPOT_TO_CPOT2,
											CPOT_TO_CPOT2};
const Int8U SelfTestHV2Board_Stage2[] = {TEST_IN_TO_SV_G, TEST_OUT_TO_SV_GE, SV_G_TO_G, SV_GE_TO_GE, G_TO_G2, GE2_TO_G2, GE_TO_GE2};

// Functions prototypes
//
SelfTestProcess SELFTEST_RelayCheck(const Int8U *RelaysArray, Int8U RelaysArrayCounter, pInt8U FailedIndex);
void SELFTEST_HandleFail(SelfTestProcess State, Int8U FailedIndex);
void SELFTEST_SwitchToNextState(SelfTestProcess CheckState, DeviceSelfTestState NextState, Int8U FailedIndex);

// Functions
//
void SELFTEST_Process()
{
	Int8U FailedIndex = 0;
	SelfTestProcess SelfTestState;

	if(CONTROL_State == DS_InSelfTest)
	{
		switch(CONTROL_SubState)
		{
			case STS_Start:
				RelayStages = CRS_Init;
				LL_SelfTestCurrentEnable(true);
				CONTROL_SetDeviceSubState(STS_InputBoardStage0);
				break;

			case STS_InputBoardStage0:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestInputBoard_Stage0, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_InputBoardStage1, FailedIndex);
				break;

			case STS_InputBoardStage1:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestInputBoard_Stage1, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_InputBoardStage2, FailedIndex);
				break;

			case STS_InputBoardStage2:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestInputBoard_Stage2, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_InputBoardStage3, FailedIndex);
				break;

			case STS_InputBoardStage3:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestInputBoard_Stage3, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_ThermBoardStage0, FailedIndex);
				break;

			case STS_ThermBoardStage0:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestThermBoard_Stage0, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_ThermBoardStage1, FailedIndex);
				break;

			case STS_ThermBoardStage1:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestThermBoard_Stage1, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_HV1BboardStage0, FailedIndex);
				break;

			case STS_HV1BboardStage0:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestHV1Board_Stage0, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_HV1BboardStage1, FailedIndex);
				break;

			case STS_HV1BboardStage1:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestHV1Board_Stage1, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_HV1BboardStage2, FailedIndex);
				break;

			case STS_HV1BboardStage2:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestHV1Board_Stage2, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_HV2BboardStage0, FailedIndex);
				break;

			case STS_HV2BboardStage0:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestHV2Board_Stage0, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_HV2BboardStage1, FailedIndex);
				break;

			case STS_HV2BboardStage1:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestHV2Board_Stage1, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_HV2BboardStage2, FailedIndex);
				break;

			case STS_HV2BboardStage2:
				SelfTestState = SELFTEST_RelayCheck_macro(SelfTestHV2Board_Stage2, &FailedIndex);
				SELFTEST_SwitchToNextState(SelfTestState, STS_Finish, FailedIndex);
				break;

			case STS_Finish:
				LL_SelfTestCurrentEnable(false);
				CONTROL_SetDeviceState(DS_Enabled);
				CONTROL_SetDeviceSubState(STS_None);
				break;

			default:
				break;
		}
	}
}
//-----------------------------------------------

void SELFTEST_SwitchToNextState(SelfTestProcess CheckState, DeviceSelfTestState NextState, Int8U FailedIndex)
{
	if(CheckState == STP_Finished)
		CONTROL_SetDeviceSubState(NextState);
	else if(CheckState != STP_InProcess)
		SELFTEST_HandleFail(CheckState, FailedIndex);
}
//-----------------------------------------------

void SELFTEST_HandleFail(SelfTestProcess State, Int8U FailedIndex)
{
	DataTable[REG_SELF_TEST_OP_RESULT] = OPRESULT_FAIL;
	DataTable[REG_SELF_TEST_FAILED_STATE] = State;
	DataTable[REG_SELF_TEST_FAILED_RELAY] = FailedIndex;
	CONTROL_SwitchToFault(DF_SELF_TEST);
}
//-----------------------------------------------

SelfTestProcess SELFTEST_RelayCheck(const Int8U *RelaysArray, Int8U RelaysArrayCounter, pInt8U FailedIndex)
{
	static Int8U TestCounter = 0;
	static SelfTestProcess Result = STP_Finished;

	switch(RelayStages)
	{
		case CRS_Init:
			ZcRD_OutputValuesReset();
			COMM_ComposeDisconnectPE();
			Result = STP_Finished;

			// Замыкание всех реле контура
			for(int i = 0; i < RelaysArrayCounter; i++)
				ZcRD_OutputValuesCompose(RelaysArray[i], true);
			ZcRD_RegisterFlushWrite();

			RelayStages = CRS_CheckClosedCircuit;
			break;

		case CRS_CheckClosedCircuit:
			if(GetTestVoltage() > DataTable[REG_SFTST_CLOSED_MAX_VOLTAGE])
			{
				Result = STP_FailedClosedCheck;
				RelayStages = CRS_Finish;
				if(FailedIndex)
					*FailedIndex = 0xFF;
			}
			else
			{
				RelayStages = CRS_RelaySwitch;
				TestCounter = 0;
			}
			break;

		case CRS_RelaySwitch:
			if(TestCounter < RelaysArrayCounter)
			{
				ZcRD_OutputValuesCompose(RelaysArray[TestCounter], false);
				ZcRD_RegisterFlushWrite();

				if(GetTestVoltage() < DataTable[REG_SFTST_OPENED_MIN_VOLTAGE])
				{
					Result = STP_FailedOpenedCheck;
					RelayStages = CRS_Finish;
					if(FailedIndex)
						*FailedIndex = RelaysArray[TestCounter];
				}
				else
				{
					ZcRD_OutputValuesCompose(RelaysArray[TestCounter], true);
					TestCounter++;
				}
			}
			else
				RelayStages = CRS_Finish;
			break;

		case CRS_Finish:
			ZcRD_OutputValuesReset();
			ZcRD_RegisterFlushWrite();
			RelayStages = CRS_Init;
			return Result;
	}

	return STP_InProcess;
}
//-----------------------------------------------
