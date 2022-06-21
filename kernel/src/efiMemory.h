#pragma once

#include <stdint.h>

struct EFI_MEMORY_DESCRIPTOR {
  uint32_t type;       // Type of this memory section
  void* physicalAddr;  // Physical Address of this memory section
  void* virtualAddr;   // Virtual Address of this memory section
  uint64_t numPages;  // numPages * 4096 = no of bytes this memory section spans
  uint64_t attribs;   // Attributes this memory has
};

extern const char* EFI_MEMORY_TYPE_STRINGS[];