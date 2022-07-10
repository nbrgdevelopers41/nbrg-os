#pragma once

#include <stdint.h>

enum PT_Flags {
  // The PDE is present and can be accessed by the Memory Management Unit (MMU)
  Present = 0,

  // The PDE is available for Read as well as Write
  ReadWrite = 1,

  // The PDE needs to be accessed by the Super/Privilleged user or not.
  UserSuper = 2,

  WriteThrough = 3,

  // If the caching for this PDE should be disabled or not.
  CacheDisabled = 4,

  // If this PDE is currently being accessed.
  Accessed = 5,

  // Turns the next PDE into a Page of the size of all the PDEs
  // that could have been held by it.
  //
  // If this is set, then this would be pointing to a Page Table,
  // which will act as a Page of size 4096 * 512
  LargerPages = 7,

  // Equivalent to the 1st Available Bit.
  Custom0 = 9,

  // Equivalent to the 2nd Available Bit.
  Custom1 = 10,

  // Equivalent to the 3rd Available Bit.
  Custom2 = 11,

  NX = 63  // Only Supported on some systems.
};

struct PageDirectoryEntry {
  uint64_t Value;

  void SetFlag(PT_Flags flag, bool enabled);

  bool GetFlag(PT_Flags flag);

  void SetAddress(uint64_t address);

  uint64_t GetAddress();
};

struct PageTable {
  PageDirectoryEntry entries[512];
} __attribute__((aligned(0x1000)));