#pragma once

#include <stdint.h>

#include "BasicRenderer.h"
#include "Bitmap.h"
#include "cstr.h"
#include "efiMemory.h"
#include "memory.h"
#include "paging/PageFrameAllocator.h"
#include "paging/PageMapIndexer.h"
#include "paging/PageTableManager.h"
#include "paging/paging.h"

class Test {
  // tests
 public:
  void testToStringImplementations(BasicRenderer newRenderer);
  void testPageMapIndexer(BasicRenderer newRenderer);
  void testPageFrameAllocator(BasicRenderer newRenderer);
  void testPageTableManager(BasicRenderer newRenderer,
                            PageTableManager pageTableManager);
};

struct BootInfo {
  Framebuffer* framebuffer;
  PSF1_FONT* psf1_Font;
  EFI_MEMORY_DESCRIPTOR* Map;
  uint64_t mMapSize;
  uint64_t mMapDescSize;
};

extern uint64_t _KernelStart;
extern uint64_t _KernelEnd;

struct KernelInfo {
  PageTableManager* pageTableManager;
};

KernelInfo InitializeKernel(BootInfo* bootInfo);