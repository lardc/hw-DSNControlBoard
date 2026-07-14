// Header
#include "StorageDescription.h"
#include "Global.h"

// Variables
RecordDescription StorageDescription[] = {};

#define SIZE_OF_POINTERS (sizeof(StorageDescription) / sizeof(StorageDescription[0]))
Int32U TablePointers[SIZE_OF_POINTERS ? SIZE_OF_POINTERS : 1] = {0};
const Int16U StorageSize = SIZE_OF_POINTERS;

CounterDescription CounterStorageDescription[] =
{
	{"0. Заглушка"},
};
CounterData CounterTablePointers[sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0])] = {0};
const Int16U CounterStorageSize = sizeof(CounterStorageDescription) / sizeof(CounterStorageDescription[0]);
