#include "memory.h"

uint64_t GetMemorySize(EFI_MEMORY_DESCRIPTOR* mMap,
                       uint64_t numEntries,
                       uint64_t mMapDescSize) {
  static uint64_t memSizeBytes =
      0;  // Static variable for total memory size, so that we don't need to
          // recalculate memory size for each call.
  if (memSizeBytes > 0)
    return memSizeBytes;

  for (int i = 0; i < numEntries; i++) {
    EFI_MEMORY_DESCRIPTOR* desc =
        (EFI_MEMORY_DESCRIPTOR*)((uint64_t)mMap + (i * mMapDescSize));

    memSizeBytes += desc->numPages * 4096;
  }

  return memSizeBytes;
}

void memset(void* start, uint8_t value, uint64_t num) {
  for (uint64_t i = 0; i < num; i++) {
    *(uint8_t*)((uint64_t)start + i) = value;
  }
}