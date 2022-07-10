#include "PageFrameAllocator.h"

uint64_t freeMemory;      // Memory available for use
uint64_t usedMemory;      // Memory in-use
uint64_t reservedMemory;  // Memory reserved for system use

bool Initialized = false;

PageFrameAllocator GlobalAllocator;

void PageFrameAllocator::ReadEFIMemoryMap(EFI_MEMORY_DESCRIPTOR* mMap,
                                          size_t mMapSize,
                                          size_t mMapDescSize) {
  if (Initialized)
    return;
  Initialized = true;

  uint64_t mMapEntries = mMapSize / mMapDescSize;

  size_t largestFreeMemorySegmentSize = 0;
  void* largestFreeMemorySegmentAddr = NULL;

  // Getting the largest chunk of free memory(EfiConventionalMemory)
  for (int i = 0; i < mMapEntries; i++) {
    EFI_MEMORY_DESCRIPTOR* desc =
        (EFI_MEMORY_DESCRIPTOR*)((uint64_t)mMap + (i * mMapDescSize));

    if (desc->type == 7) {  // the memory is EfiConventionalMemory
      if ((desc->numPages * 4096) > largestFreeMemorySegmentSize) {
        largestFreeMemorySegmentSize = desc->numPages * 4096;
        largestFreeMemorySegmentAddr = desc->physicalAddr;
      }
    }
  }

  // Total size of the system memory
  uint64_t memorySize = GetMemorySize(mMap, mMapEntries, mMapDescSize);
  freeMemory = memorySize;
  uint64_t bitmapSize = memorySize / 4096 / 8 + 1;

  // Initialize the PageBitmap
  InitializeBitmap(bitmapSize, largestFreeMemorySegmentAddr);

  // Lock pages where the bitmap is stored
  LockPages(&PageBitmap, PageBitmap.Size / 4096 + 1);

  // Reserve pages of memory of other types, eg. EfiUnusableMemory,
  // EfiACPIReclaimMemory, etc.
  for (int i = 0; i < mMapEntries; i++) {
    EFI_MEMORY_DESCRIPTOR* desc =
        (EFI_MEMORY_DESCRIPTOR*)((uint64_t)mMap + (i * mMapDescSize));

    // Is not EfiConventionalMemory
    if (desc->type != 7) {
      ReservePages(desc->physicalAddr, desc->numPages);
    }
  }
}

// Initialized the [PageBitmap]
void PageFrameAllocator::InitializeBitmap(size_t bitmapSize, void* bufferAddr) {
  PageBitmap.Size = bitmapSize;
  PageBitmap.Buffer = (uint8_t*)bufferAddr;

  // Set all bytes in the buffer to false to mark them as free
  for (int i = 0; i < bitmapSize; i++) {
    *(uint8_t*)(PageBitmap.Buffer + i) = 0;
  }
}

void* PageFrameAllocator::RequestPageLegacy() {
  uint64_t count = PageBitmap.Size * 8;
  for (uint64_t index = 0; index < count; index++) {
    if (PageBitmap[index] == true)
      continue;
    void* address = (void*)(index * 4096);
    LockPage(address);
    return address;
  }

  return NULL;  // TODO: Do page frame swap to file
}

uint64_t pageBitmapIndex = 0;
void* PageFrameAllocator::RequestPage() {
  uint64_t count = PageBitmap.Size * 8;
  for (; pageBitmapIndex < count; pageBitmapIndex++) {
    if (PageBitmap[pageBitmapIndex] == true)
      continue;
    void* address = (void*)(pageBitmapIndex * 4096);
    LockPage(address);
    return address;
  }

  return NULL;  // TODO: Do page frame swap to file
}

void PageFrameAllocator::FreePage(void* address) {
  uint64_t index = (uint64_t)address / 4096;
  if (PageBitmap[index] == false)
    return;

  if (PageBitmap.Set(index, false)) {
    freeMemory += 4096;
    usedMemory -= 4096;
    if (pageBitmapIndex > index)
      pageBitmapIndex = index;
  }
}

void PageFrameAllocator::LockPage(void* address) {
  uint64_t index = (uint64_t)address / 4096;
  if (PageBitmap[index] == true)
    return;

  if (PageBitmap.Set(index, true)) {
    freeMemory -= 4096;
    usedMemory += 4096;
  }
}

void PageFrameAllocator::ReservePage(void* address) {
  uint64_t index = (uint64_t)address / 4096;
  if (PageBitmap[index] == true)
    return;

  if (PageBitmap.Set(index, true)) {
    freeMemory -= 4096;
    reservedMemory += 4096;
  }
}

void PageFrameAllocator::UnreservePage(void* address) {
  uint64_t index = (uint64_t)address / 4096;
  if (PageBitmap[index] == false)
    return;

  if (PageBitmap.Set(index, false)) {
    freeMemory += 4096;
    reservedMemory -= 4096;
    if (pageBitmapIndex > index)
      pageBitmapIndex = index;
  }
}

void PageFrameAllocator::FreePages(void* address, uint64_t numPages) {
  for (int i = 0; i < numPages; i++) {
    FreePage((void*)((uint64_t)address + (i * 4096)));
  }
}

void PageFrameAllocator::LockPages(void* address, uint64_t numPages) {
  for (int i = 0; i < numPages; i++) {
    LockPage((void*)((uint64_t)address + (i * 4096)));
  }
}

void PageFrameAllocator::UnreservePages(void* address, uint64_t numPages) {
  for (int i = 0; i < numPages; i++) {
    UnreservePage((void*)((uint64_t)address + (i * 4096)));
  }
}

void PageFrameAllocator::ReservePages(void* address, uint64_t numPages) {
  for (int i = 0; i < numPages; i++) {
    ReservePage((void*)((uint64_t)address + (i * 4096)));
  }
}

uint64_t PageFrameAllocator::GetfreeRAM() {
  return freeMemory;
}

uint64_t PageFrameAllocator::GetInUseRAM() {
  return usedMemory;
}

uint64_t PageFrameAllocator::GetReservedRAM() {
  return reservedMemory;
}