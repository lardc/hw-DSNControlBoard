// -----------------------------------------
// Commutation logic
// ----------------------------------------

// Header
#include "Commutator.h"
//
#include "ZcRegistersDriver.h"
#include "DeviceObjectDictionary.h"
#include "CommutationTable.h"
#include "Global.h"
#include "DataTable.h"
#include "Delay.h"
#include "PMXU.h"
#include "Constraints.h"
#include "LowLevel.h"

// Variables
//
CommutationState COMM_State = COMM_Def;
DeviceProcessState COMM_ProcessState = DPS_None;
static Int64U Timeout = 0;
static Int64U CANRequestPause = 0;

// Forward declarations
//
static void COMM_DischargeBeforeIcesOrIrrm();
bool COMM_ValidateIGBT(Int16U Position, ModuleTypes Module);
bool COMM_ValidateDiode(Int16U Position, ModuleTypes Module);

// Functions
//
void COMM_Default()
{
	ZcRD_RegisterReset();

	COMM_State = COMM_Def;
	COMM_ProcessState = DPS_None;
}
// ----------------------------------------

void COMM_ComposeDisconnectPE()
{
	ZcRD_OutputValuesCompose(PE_TO_EPOT1, TRUE);
	ZcRD_OutputValuesCompose(PE_TO_EPOT2, TRUE);
	ZcRD_OutputValuesCompose(PE_TO_CPOT1, TRUE);
	ZcRD_OutputValuesCompose(PE_TO_CPOT2, TRUE);
}
// ----------------------------------------

void COMM_Process()
{
	Int16U ActionID =  DataTable[REG_LAST_CMD];

	switch(COMM_ProcessState)
	{
		case DPS_Start:
			if(PMXU_InFault())
				CONTROL_SwitchToFault(DF_PMXU);
			else if(!PMXU_IsReady())
				CONTROL_FinishedWithProblem(PROBLEM_PMXU_NOT_READY);
			else
				COMM_ProcessState = DPS_CheckIcesAndDischarge;
			break;

		case DPS_CheckIcesAndDischarge:
			if(COMM_State == COMM_IcesOrIrrm && ActionID != ACT_COMM_ICES_OR_IRRM)
			{
				if(PMXU_SwitchCommutation(DataTable[REG_DUT_POSITION], DataTable[REG_DUT_CASE], DataTable[REG_DUT_SCHEME], ACT_PMXU_COMM_PE))
				{
					COMM_ProcessState = DPS_CheckStatusAfterDischarge;
					COMM_DischargeBeforeIcesOrIrrm();
					Timeout = CONTROL_TimeCounter + PMXU_WAIT_LONG;
					CANRequestPause = 0;
				}
			}
			else
				COMM_ProcessState = DPS_PMXUCommutate;
			break;

		case DPS_CheckStatusAfterDischarge:
			if(CONTROL_TimeCounter <= Timeout)
			{
				if(CONTROL_TimeCounter >= CANRequestPause)
				{
					CANRequestPause = CONTROL_TimeCounter + PMXU_CAN_REQUEST_DELAY;
					if(PMXU_CheckReady())
						if(PMXU_CheckOPResult(OPRESULT_OK))
							COMM_ProcessState = DPS_PMXUCommutate;
				}
			}
			else
				CONTROL_SwitchToFault(DF_PMXU_FAILED_TO_FINISH);

			break;

		case DPS_PMXUCommutate:
			{
				Int16U PMXU_Command;
				switch(ActionID)
				{
					case ACT_COMM_NONE:
						PMXU_Command = ACT_PMXU_COMM_PE;
						break;
					case ACT_COMM_UCE_SAT:
						PMXU_Command = ACT_PMXU_COMM_VCESAT;
						break;
					case ACT_COMM_UFW_CHOPPER_DIODE:
						PMXU_Command = ACT_PMXU_COMM_VF;
						break;
					case ACT_COMM_ICES_OR_IRRM:
						PMXU_Command = ACT_PMXU_COMM_ICES;
						break;
					default:
						PMXU_Command = ACT_PMXU_COMM_NO_PE;
						break;
				}
				if(PMXU_SwitchCommutation(DataTable[REG_DUT_POSITION], DataTable[REG_DUT_CASE], DataTable[REG_DUT_SCHEME], PMXU_Command))
				{
					Timeout = CONTROL_TimeCounter + PMXU_WAIT_LONG;
					CANRequestPause = 0;
					COMM_ProcessState = DPS_CheckStatusAfterCommutation;
				}
			}
			break;

		case DPS_CheckStatusAfterCommutation:
			if(CONTROL_TimeCounter <= Timeout)
			{
				if(CONTROL_TimeCounter >= CANRequestPause)
				{
					CANRequestPause = CONTROL_TimeCounter + PMXU_CAN_REQUEST_DELAY;
					if(PMXU_CheckReady())
						if(PMXU_CheckOPResult(OPRESULT_OK))
							COMM_ProcessState = DPS_MXUCommutate;
				}
			}
			else
				CONTROL_SwitchToFault(DF_PMXU_FAILED_TO_FINISH);
			break;

		case DPS_MXUCommutate:
			COMM_Commutate(ActionID);
			DataTable[REG_OP_RESULT] = OPRESULT_OK;
			COMM_ProcessState = DPS_None;
			break;

		default:
			break;
	}
}
// ----------------------------------------
void COMM_Commutate(Int16U ActionID)
{
	Int16U DUTPosition = DataTable[REG_DUT_POSITION];
	ModuleTypes ModuleType = COMM_CalcModuleType();
	switch(ActionID)
	{
		case ACT_COMM_NONE:
			COMM_Default();
			break;

		case ACT_COMM_IGES_POS_PULSE:
		case ACT_COMM_IGES_NEG_PULSE:
		case ACT_COMM_UGE_TH:
		case ACT_COMM_UCE_SAT:
			ZcRD_OutputValuesReset();
			COMM_ComposeDisconnectPE();
			switch(ActionID)
			{
				case ACT_COMM_IGES_POS_PULSE:
					COMM_State = COMM_Iges_Pos;
					ZcRD_OutputValuesCompose(GT_G_COMM, TRUE);
					ZcRD_OutputValuesCompose(GT_GE_COMM, TRUE);
					ZcRD_OutputValuesCompose(GT_G_TO_G, TRUE);
					ZcRD_OutputValuesCompose(GT_GE_TO_GE, TRUE);
					break;
				case ACT_COMM_IGES_NEG_PULSE:
					COMM_State = COMM_Iges_Neg;
					ZcRD_OutputValuesCompose(GT_G_COMM, TRUE);
					ZcRD_OutputValuesCompose(GT_GE_COMM, TRUE);
					ZcRD_OutputValuesCompose(GT_G_TO_GE, TRUE);
					ZcRD_OutputValuesCompose(GT_GE_TO_G, TRUE);
					break;
				case ACT_COMM_UGE_TH:
					COMM_State = COMM_Ugeth;
					ZcRD_OutputValuesCompose(GT_G_COMM, TRUE);
					ZcRD_OutputValuesCompose(GT_GE_COMM, TRUE);
					ZcRD_OutputValuesCompose(GT_GPOT_COMM, TRUE);
					ZcRD_OutputValuesCompose(GT_GEPOT_COMM, TRUE);
					ZcRD_OutputValuesCompose(GT_G_TO_G, TRUE);
					ZcRD_OutputValuesCompose(GT_GE_TO_GE, TRUE);
					ZcRD_OutputValuesCompose(GT_GPOT_TO_G, TRUE);
					ZcRD_OutputValuesCompose(GT_GEPOT_TO_EPOT, TRUE);
					ZcRD_OutputValuesCompose(G_TO_CPOT, TRUE);
					break;
				case ACT_COMM_UCE_SAT:
					COMM_State = COMM_Ucesat;
					ZcRD_OutputValuesCompose(SV_G_COMM, TRUE);
					ZcRD_OutputValuesCompose(SV_GE_COMM, TRUE);
					ZcRD_OutputValuesCompose(SV_POT_POS_COMM, TRUE);
					ZcRD_OutputValuesCompose(SV_POT_NEG_COMM, TRUE);
					ZcRD_OutputValuesCompose(SV_G_TO_G, TRUE);
					ZcRD_OutputValuesCompose(SV_GE_TO_GE, TRUE);
					ZcRD_OutputValuesCompose(SV_POT_POS_TO_CPOT, TRUE);
					ZcRD_OutputValuesCompose(SV_POT_NEG_TO_EPOT, TRUE);
					break;

			}
			if(DUTPosition == DUT_POSITION_1)
			{
				switch(ModuleType)
				{
					case MIAA_CE:
					case MIAA_HB:
					case MIAA_LC:
					case MIDA_HB:
					case MIFA_HB:
					case MIFA_LC:
					case MIHA_HB:
					case MIHA_LC:
					case MIHM_SS:
					case MIHV_SS:
					case MISM_CH:
					case MISM_DS:
					case MISM_SS:
					case MISV_SS:
					case MIXM_HB:
					case MIXM_LR_LRD:
					case MIXV_HB:
						if(ActionID == ACT_COMM_UGE_TH || ActionID == ACT_COMM_UCE_SAT)
							ZcRD_OutputValuesCompose(CPOT_TO_CPOT1, TRUE);
						else
							ZcRD_OutputValuesCompose(EPOT_TO_CPOT1, TRUE);
						ZcRD_OutputValuesCompose(EPOT_TO_EPOT1, TRUE);
						ZcRD_OutputValuesCompose(G_TO_G1, TRUE);
						ZcRD_OutputValuesCompose(GE_TO_GE1, TRUE);
						ZcRD_OutputValuesCompose(GE1_TO_G1, TRUE);
						break;
					default:
						break;
				}
			}
			else if(DUTPosition == DUT_POSITION_2)
			{
				switch(ModuleType)
				{
					case MIAA_CE:
						ZcRD_OutputValuesCompose(EPOT_TO_EPOT1, TRUE);
						ZcRD_OutputValuesCompose(GE_TO_GE2, TRUE);
						ZcRD_OutputValuesCompose(G_TO_G2, TRUE);
						if(ActionID == ACT_COMM_UGE_TH || ActionID == ACT_COMM_UCE_SAT)
							ZcRD_OutputValuesCompose(CPOT_TO_CPOT2, TRUE);
						else
							ZcRD_OutputValuesCompose(EPOT_TO_CPOT2, TRUE);
						break;
					case MIAA_HB:
					case MIAA_HC:
					case MIDA_HB:
					case MIFA_HB:
					case MIFA_HC:
					case MIHA_HB:
					case MIHA_HC:
					case MISM_DS:
					case MIXM_HB:
					case MIXV_HB:
						if(ActionID == ACT_COMM_UGE_TH || ActionID == ACT_COMM_UCE_SAT)
							ZcRD_OutputValuesCompose(CPOT_TO_CPOT2, TRUE);
						else
							ZcRD_OutputValuesCompose(EPOT_TO_CPOT2, TRUE);
						ZcRD_OutputValuesCompose(EPOT_TO_EPOT2, TRUE);
						ZcRD_OutputValuesCompose(G_TO_G2, TRUE);
						ZcRD_OutputValuesCompose(GE_TO_GE2, TRUE);
						ZcRD_OutputValuesCompose(GE2_TO_G2, TRUE);
						break;
					default:
						break;
				}
			}
			ZcRD_RegisterFlushWrite();
			break;

		case ACT_COMM_UFW_CHOPPER_DIODE:
			COMM_State = COMM_Uf;

			ZcRD_OutputValuesReset();
			COMM_ComposeDisconnectPE();
			ZcRD_OutputValuesCompose(SV_POT_POS_COMM, TRUE);
			ZcRD_OutputValuesCompose(SV_POT_NEG_COMM, TRUE);
			ZcRD_OutputValuesCompose(SV_POT_POS_TO_EPOT, TRUE);
			ZcRD_OutputValuesCompose(SV_POT_NEG_TO_CPOT, TRUE);

			if(DUTPosition == DUT_POSITION_1)
			{
				switch(ModuleType)
				{
					case MIFA_SD:
						ZcRD_OutputValuesCompose(EPOT_TO_EPOT2, TRUE);
						ZcRD_OutputValuesCompose(CPOT_TO_CPOT2, TRUE);
						break;
					case MDAA_DD:
					case MDDA_DD:
					case MDFA_DD:
					case MDSM_SD:
					case MDSV_SD:
					case MIAA_HC:
					case MIFA_HC:
					case MIHA_HC:

					case MIAA_CE:
					case MIAA_HB:
					case MIAA_LC:
					case MIDA_HB:
					case MIFA_HB:
					case MIFA_LC:
					case MIHA_HB:
					case MIHA_LC:
					case MIHM_SS:
					case MIHV_SS:
					case MISM_CH:
					case MISM_DS:
					case MISM_SS:
					case MISV_SS:
					case MIXM_HB:
					case MIXM_LR_LRD:
					case MIXV_HB:
						ZcRD_OutputValuesCompose(EPOT_TO_EPOT1, TRUE);
						ZcRD_OutputValuesCompose(CPOT_TO_CPOT1, TRUE);
						break;
					default:
						break;
				}
			}
			else if(DUTPosition == DUT_POSITION_2)
			{
				switch(ModuleType)
				{
					case MDSM_SD:
					case MDSV_SD:
					case MIXM_LR_LRD:
						ZcRD_OutputValuesCompose(CPOT_TO_EPOT2, TRUE);
						ZcRD_OutputValuesCompose(EPOT_TO_CPOT2, TRUE);
						break;
					case MIAA_CE:
						ZcRD_OutputValuesCompose(EPOT_TO_EPOT1, TRUE);
						ZcRD_OutputValuesCompose(CPOT_TO_CPOT2, TRUE);
						break;
					case MDAA_DD:
					case MDDA_DD:
					case MDFA_DD:
					case MIAA_LC:
					case MIFA_LC:
					case MIHA_LC:
					case MISM_CH:

					case MIAA_HB:
					case MIAA_HC:
					case MIDA_HB:
					case MIFA_HB:
					case MIFA_HC:
					case MIHA_HB:
					case MIHA_HC:
					case MISM_DS:
					case MIXM_HB:
					case MIXV_HB:
						ZcRD_OutputValuesCompose(EPOT_TO_EPOT2, TRUE);
						ZcRD_OutputValuesCompose(CPOT_TO_CPOT2, TRUE);
						break;
					default:
						break;
				}
			}
			ZcRD_RegisterFlushWrite();
			break;

		case ACT_COMM_ICES_OR_IRRM:
		case ACT_COMM_NO_PE:
			if(ActionID == ACT_COMM_ICES_OR_IRRM)
				COMM_State = COMM_IcesOrIrrm;
			else if(ActionID == ACT_COMM_NO_PE)
				COMM_State = COMM_NoPE;

			ZcRD_OutputValuesReset();
			COMM_ComposeDisconnectPE();
			ZcRD_RegisterFlushWrite();
			break;

		case ACT_COMM_THERMISTOR:
			COMM_State = COMM_Thermistor;

			ZcRD_OutputValuesReset();
			COMM_ComposeDisconnectPE();

			ZcRD_OutputValuesCompose(GT_G_T2, TRUE);
			ZcRD_OutputValuesCompose(GT_GE_T1, TRUE);
			ZcRD_OutputValuesCompose(GT_G_AND_GT_GPOT_TO_T2, TRUE);
			ZcRD_OutputValuesCompose(GT_GE_AND_GT_GEPOT_TO_T1, TRUE);
			ZcRD_OutputValuesCompose(GT_GPOT_TO_GT_G_T2, TRUE);
			ZcRD_OutputValuesCompose(GT_GEPOT_TO_GT_GE_T1, TRUE);

			ZcRD_RegisterFlushWrite();
			break;
	}
}
// ----------------------------------------

static void COMM_DischargeBeforeIcesOrIrrm()
{
	LL_SafetyForceRelaysOff(true);
	ZcRD_OutputValuesReset();
	ZcRD_RegisterFlushWrite();
	LL_SafetyForceRelaysOff(false);
}
// ----------------------------------------

Int32U COMM_CalcModuleType()
{
	const Int32U CaseShift = 100;

	return ((Int32U)DataTable[REG_DUT_CASE] * CaseShift + (Int32U)DataTable[REG_DUT_SCHEME]);
}
// ----------------------------------------

bool COMM_ValidateRequest(Int16U ActionID, Int16U Position)
{
	if(Position != DUT_POSITION_1 && Position != DUT_POSITION_2)
	{
		CONTROL_FinishedWithProblem(PROBLEM_INCORRECT_DUT);
		return false;
	}

	ModuleTypes ModuleType = COMM_CalcModuleType();

	switch(ActionID)
	{
		case ACT_COMM_NONE:
		case ACT_COMM_NO_PE:
			return true;	// допустимо без проверки корпуса

		case ACT_COMM_IGES_POS_PULSE:
		case ACT_COMM_IGES_NEG_PULSE:
		case ACT_COMM_UGE_TH:
		case ACT_COMM_UCE_SAT:
			if(COMM_ValidateIGBT(Position,ModuleType))
				return true;
			CONTROL_FinishedWithProblem(PROBLEM_INCORRECT_DUT);
			return false;

		case ACT_COMM_ICES_OR_IRRM:
		case ACT_COMM_UFW_CHOPPER_DIODE:
			if(COMM_ValidateIGBT(Position, ModuleType) || COMM_ValidateDiode(Position, ModuleType))
				return true;
			CONTROL_FinishedWithProblem(PROBLEM_INCORRECT_DUT);
			return false;

		case ACT_COMM_THERMISTOR:
			switch(ModuleType)
			{
				case MIDA_HB:
				case MIXM_LR_LRD:
				case MIXM_HB:
					return true;
				default:
					CONTROL_FinishedWithProblem(PROBLEM_INCORRECT_DUT);
					return false;
			}
			break;
	}

	CONTROL_FinishedWithProblem(PROBLEM_INCORRECT_COMMAND);
	return false;
}
// ----------------------------------------

bool COMM_ValidateIGBT(Int16U Position, ModuleTypes Module)
{
	if(Position == DUT_POSITION_1)
	{
		switch(Module)
		{
			case MIAA_CE:
			case MIAA_HB:
			case MIAA_LC:
			case MIDA_HB:
			case MIFA_HB:
			case MIFA_LC:
			case MIHA_HB:
			case MIHA_LC:
			case MIHM_SS:
			case MIHV_SS:
			case MISM_CH:
			case MISM_DS:
			case MISM_SS:
			case MISV_SS:
			case MIXM_HB:
			case MIXM_LR_LRD:
			case MIXV_HB:
				return true;
			default:
				return false;
		}
	}
	else if(Position == DUT_POSITION_2)
	{
		switch(Module)
		{
			case MIAA_CE:
			case MIAA_HB:
			case MIAA_HC:
			case MIDA_HB:
			case MIFA_HB:
			case MIFA_HC:
			case MIHA_HB:
			case MIHA_HC:
			case MISM_DS:
			case MIXM_HB:
			case MIXV_HB:
				return true;
			default:
				return false;
		}
	}
	return false;
}
// ----------------------------------------

bool COMM_ValidateDiode(Int16U Position, ModuleTypes Module)
{
	if(Position == DUT_POSITION_1)
	{
		switch(Module)
		{
			case MDAA_DD:
			case MDDA_DD:
			case MDFA_DD:
			case MDSM_SD:
			case MDSV_SD:
			case MIAA_HC:
			case MIFA_HC:
			case MIFA_SD:
			case MIHA_HC:
				return true;
			default:
				return false;
		}
	}
	else if(Position == DUT_POSITION_2)
	{
		switch(Module)
		{
			case MDAA_DD:
			case MDDA_DD:
			case MDFA_DD:
			case MDSM_SD:
			case MDSV_SD:
			case MIAA_LC:
			case MIFA_LC:
			case MIHA_LC:
			case MISM_CH:
			case MIXM_LR_LRD:
				return true;
			default:
				return false;
		}
	}
	return false;
}
// ----------------------------------------
