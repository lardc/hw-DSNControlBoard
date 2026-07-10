#ifndef __STORAGE_DESCRIPTION_H
#define __STORAGE_DESCRIPTION_H

// Include
#include "stdinc.h"
#include "SaveToFlash.h"

// Variables
extern RecordDescription StorageDescription[];
extern Int32U TablePointers[];
extern const Int16U StorageSize;

extern CounterDescription CounterStorageDescription[];
extern CounterData CounterTablePointers[];
extern const Int16U CounterStorageSize;

#endif // __STORAGE_DESCRIPTION_H
