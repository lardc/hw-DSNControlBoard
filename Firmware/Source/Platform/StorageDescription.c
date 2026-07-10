// Header
#include "StorageDescription.h"
#include "Global.h"

// Variables
RecordDescription StorageDescription[] = {};

#define SIZE_OF_POINTERS (sizeof(StorageDescription) / sizeof(StorageDescription[0]))
Int32U TablePointers[SIZE_OF_POINTERS ? SIZE_OF_POINTERS : 1] = {0};
const Int16U StorageSize = SIZE_OF_POINTERS;

// 64 счётчика соответствуют 64-слотной CommutationTable[].
// Расположение: 0..31 — IORelayBoard, 32..39 — OutThermRelayBoard, 40..63 — InputRelayBoard.
CounterDescription CounterStorageDescription[] =
{
	// ===== IORelayBoard (SS3, объединение прежних OutHVRelayBoard-1/2) =====
	{"0. IORelayBoard (OutHV-1): E_POT to C_POT1"},
	{"1. IORelayBoard (OutHV-1): C_POT to C_POT1"},
	{"2. IORelayBoard (OutHV-1): PE (GND) to C_POT1"},
	{"3. IORelayBoard (OutHV-1): PE (GND) to E_POT1"},
	{"4. IORelayBoard (OutHV-1): C_POT to E_POT1"},
	{"5. IORelayBoard (OutHV-1): E_POT to E_POT1"},
	{"6. IORelayBoard (OutHV-1): G to G1"},
	{"7. IORelayBoard (OutHV-1): GE1 to G1 (through 1k resistor)"},
	{"8. IORelayBoard (OutHV-1): GE to GE1"},
	{"9. IORelayBoard (OutHV-2): E_POT to C_POT2"},
	{"10. IORelayBoard (OutHV-2): C_POT to C_POT2"},
	{"11. IORelayBoard (OutHV-2): PE (GND) to C_POT2"},
	{"12. IORelayBoard (OutHV-2): PE (GND) to E_POT2"},
	{"13. IORelayBoard (OutHV-2): C_POT to E_POT2"},
	{"14. IORelayBoard (OutHV-2): E_POT to E_POT2"},
	{"15. IORelayBoard (OutHV-2): G to G2"},
	{"16. IORelayBoard (OutHV-2): GE2 to G2 (through 1k resistor)"},
	{"17. IORelayBoard (OutHV-2): GE to GE2"},
	{"18. IORelayBoard (Main): GT_G to G"},
	{"19. IORelayBoard (Main): GT_GE to GE"},
	{"20. IORelayBoard (Main): GT_G to GE"},
	{"21. IORelayBoard (Main): GT_GE to G"},
	{"22. IORelayBoard (Main): GT_G_POT to G"},
	{"23. IORelayBoard (Main): GT_GE_POT to E_POT"},
	{"24. IORelayBoard (Main): LSL_G to G"},
	{"25. IORelayBoard (Main): LSL_GE to GE"},
	{"26. IORelayBoard (Main): LSL_POT+ to E_POT"},
	{"27. IORelayBoard (Main): LSL_POT- to C_POT"},
	{"28. IORelayBoard (Main): LSL_POT+ to C_POT"},
	{"29. IORelayBoard (Main): LSL_POT- to E_POT"},
	{"30. IORelayBoard (Main): G to C_POT"},
	{"31. IORelayBoard: not used"},
	// ===== OutThermRelayBoard (SS2) =====
	{"32. OutThermRelayBoard: T2 to GT_G and GT_G_POT"},
	{"33. OutThermRelayBoard: T1 to GT_GE and GT_GE_POT"},
	{"34. OutThermRelayBoard: GT_G to T2"},
	{"35. OutThermRelayBoard: GT_G_POT to GT_G_T2"},
	{"36. OutThermRelayBoard: GT_GE to T1"},
	{"37. OutThermRelayBoard: GT_GE_POT to GT_GE_T1"},
	{"38. OutThermRelayBoard: not used"},
	{"39. OutThermRelayBoard: not used"},
	// ===== InputRelayBoard (SS1) =====
	{"40. InputRelayBoard: GT_G to TEST-IN"},
	{"41. InputRelayBoard: GT_GE to TEST-OUT"},
	{"42. InputRelayBoard: GT_G commutation (Open/Close)"},
	{"43. InputRelayBoard: GT_GE commutation (Open/Close)"},
	{"44. InputRelayBoard: GT_G to GT_GE"},
	{"45. InputRelayBoard: GT_G_POT to TEST-IN"},
	{"46. InputRelayBoard: GT_GE_POT to TEST-OUT"},
	{"47. InputRelayBoard: GT_G_POT commutation (Open/Close)"},
	{"48. InputRelayBoard: GT_GE_POT commutation (Open/Close)"},
	{"49. InputRelayBoard: GT_G_POT to GT_GE_POT"},
	{"50. InputRelayBoard: LSL_G to TEST-IN"},
	{"51. InputRelayBoard: LSL_GE to TEST-OUT"},
	{"52. InputRelayBoard: LSL_G commutation (Open/Close)"},
	{"53. InputRelayBoard: LSL_GE commutation (Open/Close)"},
	{"54. InputRelayBoard: LSL_G to LSL_GE"},
	{"55. InputRelayBoard: LSL_POT+ to TEST-IN"},
	{"56. InputRelayBoard: LSL_POT- to TEST-OUT"},
	{"57. InputRelayBoard: LSL_POT+ commutation (Open/Close)"},
	{"58. InputRelayBoard: LSL_POT- commutation (Open/Close)"},
	{"59. InputRelayBoard: LSL_POT+ to LSL_POT-"},
	{"60. InputRelayBoard: LSL_POT+ to PMXU+"},
	{"61. InputRelayBoard: LSL_POT- to PMXU-"},
	{"62. InputRelayBoard: not used"},
	{"63. InputRelayBoard: not used"}
};
CounterData CounterTablePointers[sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0])] = {0};
const Int16U CounterStorageSize = sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0]);
