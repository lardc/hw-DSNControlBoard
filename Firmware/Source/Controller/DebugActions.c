// Header
#include "DebugActions.h"

// Include
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Logic.h"
#include "Controller.h"

// Functions
//
bool DBACT_HandleDiagnosticAction(uint16_t ActionID, uint16_t *pUserError)
{
	switch(ActionID)
	{
		case ACT_DBG_ONEWIRE_DISCOVER:
			{
				Boolean Ok = LOGIC_ScanAdapter();

				DataTable[REG_DBG] =
						(float)(LOGIC_Adapter.TemperaturePresent ? 1 : 0)
						+ (float)(LOGIC_Adapter.SwitchBoardCount * 10)
						+ (float)(LOGIC_Adapter.GateDriverCount * 100)
						+ (float)(LOGIC_Adapter.ProbeCount * 1000);

				if(!Ok)
				{
					if(!LOGIC_Adapter.TemperaturePresent)
						CONTROL_FinishedWithProblem(PROBLEM_OW_TEMP);
					else if(LOGIC_Adapter.ProbeCount == 0 && LOGIC_Adapter.GateDriverCount == 0)
						CONTROL_FinishedWithProblem(PROBLEM_OW_BUS);
					else
						CONTROL_FinishedWithProblem(PROBLEM_OW_INVENTORY);
				}
			}
			break;

		case ACT_DBG_READ_TEMP:
			{
				float Temperature;

				if(LOGIC_ReadTemperature(&Temperature))
					DataTable[REG_DBG] = Temperature;
				else
					CONTROL_FinishedWithProblem(PROBLEM_OW_TEMP);
			}
			break;

		case ACT_DBG_ADAPTER_LIMITS:
			{
				if(!LOGIC_Adapter.Probe.Present)
				{
					CONTROL_FinishedWithProblem(PROBLEM_OW_INVENTORY);
					break;
				}

				DataTable[REG_DBG] = (float)(LOGIC_Adapter.Probe.MaxVoltage * 1000
						+ LOGIC_Adapter.Probe.MaxCurrent);
			}
			break;

		default:
			return false;
	}

	return true;
}
//-------------------------------------
