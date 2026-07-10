// Header
#include "Diagnostic.h"

// Includes
#include "DataTable.h"
#include "LowLevel.h"
#include "Controller.h"
#include "DebugActions.h"
#include "ZcRegistersDriver.h"

// Functions
bool DIAG_HandleDiagnosticAction(uint16_t ActionID, uint16_t *pUserError)
{
	switch(ActionID)
	{
		case ACT_DBG_FP_LED:
			DBACT_ToggleFPLed();
			break;

		case ACT_DBG_WRITE_SPI:
			DBACT_WriteSPI();
			break;

		case ACT_DBG_SPI_RST:
			ZcRD_RegisterReset();
			break;

		case ACT_DBG_SELF_TEST_MEASURE:
			DBACT_SelfTestMeasure();
			break;

		case ACT_DBG_SD_EN:
			DBACT_SDEN();
			break;

		case ACT_DBG_SFT_ENABLE:
			DBACT_SftEnablePulse();
			break;

		case ACT_DBG_SFT_IN:
			DBACT_ReadSftIn();
			break;

		case ACT_DBG_SPI_SS:
			DBACT_SpiSSPulse();
			break;

		default:
			return false;
	}

	return true;
}
//-------------------------------------
