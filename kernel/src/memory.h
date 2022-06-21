#pragma once

#include <stdint.h>
#include "efiMemory.h"

uint64_t GetMemorySize(EFI_MEMORY_DESCRIPTOR* mMap,
                       uint64_t numEntries,
                       uint64_t mMapDescSize);