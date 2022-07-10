#pragma once

#include <stdint.h>
#include "../Bitmap.h"
#include "../efiMemory.h"
#include "../memory.h"

class PageFrameAllocator {
 public:
  void ReadEFIMemoryMap(EFI_MEMORY_DESCRIPTOR* mMap,
                        size_t mMapSize,
                        size_t mMapDescSize);

  // The bitmap which stores the system memory info.
  Bitmap PageBitmap;

  // Locks and returns a free memory page that can be used by the requester.
  void* RequestPage();

  // Legacy, Slower Implementation of RequestPage()
  void* RequestPageLegacy();

  // Locks a single page
  void LockPage(void* address);

  // Unlocks a single page
  void FreePage(void* address);

  // Locks multiple sequential pages
  void LockPages(void* address, uint64_t numPages);

  // Unlocks multiple sequential pages
  void FreePages(void* address, uint64_t numPages);

  /// Memory available for use
  uint64_t GetfreeRAM();

  /// Memory in-use
  uint64_t GetInUseRAM();

  /// Memory reserved for system use
  uint64_t GetReservedRAM();

 private:
  // Initialize the bitmap
  void InitializeBitmap(size_t bitmapSize, void* bufferAddr);

  // Reserves a single page
  void ReservePage(void* address);

  // Unreserves a single page
  void UnreservePage(void* address);

  // Reserves multiple sequential pages
  void ReservePages(void* address, uint64_t numPages);

  // Unreserves multiple sequential pages
  void UnreservePages(void* address, uint64_t numPages);
};
// The Global Instance of the PageFrameAllocator
extern PageFrameAllocator GlobalAllocator;