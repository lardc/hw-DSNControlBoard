// Header
#include "ZcRegistersDriver.h"

// Include
#include "LowLevel.h"
#include "Global.h"
#include "Delay.h"

// Variables
//
Int32U CycleCounters[ZCRD_RELAY_COUNTERS] = {0};

static Int16U DriverMask[2] = {ZCRD_CS_IDLE, ZCRD_CS_IDLE};
static Int16U ExpMask = 0;
static Int16U PrevExpMask = 0;

// Forward
//
static void ZcRD_Shift16(Int16U Mask);
static void ZcRD_LatchDriver(ZcRD_DriverChannel Channel);
static void ZcRD_UpdateRelayCounters(Int16U NewMask);

// Functions
//
static void ZcRD_Shift16(Int16U Mask)
{
	LL_SetSpiSckMux(false);
	DELAY_US(TIME_SPI_DELAY_US);
	LL_SPI_WriteByte((Int8U)(Mask >> 8));
	LL_SPI_WriteByte((Int8U)(Mask & 0xFF));
}
//-----------------------------------------------

static void ZcRD_LatchDriver(ZcRD_DriverChannel Channel)
{
	if(Channel == ZcRD_DrvH)
		LL_SPI_LatchRckH();
	else
		LL_SPI_LatchRckL();
}
//-----------------------------------------------

void ZcRD_WriteDriverMask(ZcRD_DriverChannel Channel, Int16U Mask)
{
	DriverMask[Channel] = Mask;
	ZcRD_Shift16(Mask);
	ZcRD_LatchDriver(Channel);
}
//-----------------------------------------------

Int16U ZcRD_GetDriverMask(ZcRD_DriverChannel Channel)
{
	return DriverMask[Channel];
}
//-----------------------------------------------

void ZcRD_SetDriverBaseMask(ZcRD_DriverChannel Channel, Int16U BaseMask)
{
	// База без линий CS — CS всегда в idle, пока не выбран ЦАП
	DriverMask[Channel] = (BaseMask & ~(ZCRD_BIT_CS1 | ZCRD_BIT_CS2)) | ZCRD_CS_IDLE;
}
//-----------------------------------------------

void ZcRD_WriteDacCode(Int32U Code)
{
	LL_SetSpiSckMux(true);
	DELAY_US(TIME_SPI_DELAY_US);
	LL_SPI_WriteByte((Int8U)(Code >> 24));
	LL_SPI_WriteByte((Int8U)(Code >> 16));
	LL_SPI_WriteByte((Int8U)(Code >> 8));
	LL_SPI_WriteByte((Int8U)(Code & 0xFF));
}
//-----------------------------------------------

void ZcRD_SelectDac1(ZcRD_DriverChannel Channel)
{
	Int16U Mask = (DriverMask[Channel] & ~(ZCRD_BIT_CS1 | ZCRD_BIT_CS2)) | ZCRD_CS1_ACTIVE;
	ZcRD_WriteDriverMask(Channel, Mask);
}
//-----------------------------------------------

void ZcRD_SelectDac2(ZcRD_DriverChannel Channel)
{
	Int16U Mask = (DriverMask[Channel] & ~(ZCRD_BIT_CS1 | ZCRD_BIT_CS2)) | ZCRD_CS2_ACTIVE;
	ZcRD_WriteDriverMask(Channel, Mask);
}
//-----------------------------------------------

void ZcRD_DeselectDacs(ZcRD_DriverChannel Channel)
{
	Int16U Mask = (DriverMask[Channel] & ~(ZCRD_BIT_CS1 | ZCRD_BIT_CS2)) | ZCRD_CS_IDLE;
	ZcRD_WriteDriverMask(Channel, Mask);
}
//-----------------------------------------------

static void ZcRD_UpdateRelayCounters(Int16U NewMask)
{
	for(Int16U i = 0; i < ZCRD_RELAY_COUNTERS; ++i)
	{
		Int16U Bit = (Int16U)(1u << i);
		if((NewMask & Bit) && !(PrevExpMask & Bit))
			CycleCounters[i]++;
	}
	PrevExpMask = NewMask;
}
//-----------------------------------------------

void ZcRD_WriteExpMask(Int16U Mask)
{
	ZcRD_UpdateRelayCounters(Mask);
	ExpMask = Mask;

	LL_SetOeExp(true);
	ZcRD_Shift16(Mask);
	LL_SPI_LatchRckExp();
}
//-----------------------------------------------

Int16U ZcRD_GetExpMask()
{
	return ExpMask;
}
//-----------------------------------------------

void ZcRD_SetExpBit(Int16U Bit, Boolean On)
{
	Int16U Mask = ExpMask;

	if(On)
		Mask |= Bit;
	else
		Mask &= ~Bit;

	ZcRD_WriteExpMask(Mask);
	DELAY_MS(COMM_DELAY_MS);
}
//-----------------------------------------------

void ZcRD_ResetAll()
{
	DriverMask[ZcRD_DrvH] = ZCRD_CS_IDLE;
	DriverMask[ZcRD_DrvL] = ZCRD_CS_IDLE;
	ExpMask = 0;
	PrevExpMask = 0;

	ZcRD_WriteDriverMask(ZcRD_DrvH, ZCRD_CS_IDLE);
	ZcRD_WriteDriverMask(ZcRD_DrvL, ZCRD_CS_IDLE);
	ZcRD_WriteExpMask(0);
	LL_SetOeExp(false);
}
//-----------------------------------------------
