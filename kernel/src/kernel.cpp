#include <stdint.h>

#include "BasicRenderer.h"
#include "Bitmap.h"
#include "PageFrameAllocator.h"
#include "cstr.h"
#include "efiMemory.h"
#include "memory.h"

struct BootInfo {
  Framebuffer* framebuffer;
  PSF1_FONT* psf1_Font;
  EFI_MEMORY_DESCRIPTOR* Map;
  uint64_t mMapSize;
  uint64_t mMapDescSize;
};

uint8_t testBuffer[20];

extern "C" void _start(BootInfo* bootInfo) {
  BasicRenderer newRenderer =
      BasicRenderer(bootInfo->framebuffer, bootInfo->psf1_Font);

  newRenderer.Print(
      "Hello User! Welcome to NBRG-OS. This is a test line. Thank you.");

  PageFrameAllocator pageFrameAllocator;
  pageFrameAllocator.ReadEFIMemoryMap(bootInfo->Map, bootInfo->mMapSize,
                                      bootInfo->mMapDescSize);

  newRenderer.NewLine();

  newRenderer.Print("Free RAM: ");
  newRenderer.Print(to_string(pageFrameAllocator.GetfreeRAM() / 1024));
  newRenderer.Print(" KB");
  newRenderer.NewLine();

  newRenderer.Print("In-Use RAM: ");
  newRenderer.Print(to_string(pageFrameAllocator.GetInUseRAM() / 1024));
  newRenderer.Print(" KB");
  newRenderer.NewLine();

  newRenderer.Print("Reserved RAM: ");
  newRenderer.Print(to_string(pageFrameAllocator.GetReservedRAM() / 1024));
  newRenderer.Print(" KB");
  newRenderer.NewLine();

  return;
}

void printTest(BasicRenderer newRenderer) {
  newRenderer.Print("Today's Date is: ");
  newRenderer.Print(to_string((uint64_t)17062022));
  newRenderer.Print(" And A random number is: ");
  newRenderer.Print(to_string((int64_t)-30785));
  newRenderer.Print(" And A random floating number is: ");
  newRenderer.Print(to_string((double)-13.14));
  newRenderer.Print(" And The Hex Color is: ");
  newRenderer.Print(to_hstring((uint64_t)0xFF00cc99));
  newRenderer.Print(" And The shortened Hex Color is: ");
  newRenderer.Print(to_hstring((uint32_t)0xFF00cc99));
  newRenderer.Print(" And The Hex Color more shortened is: ");
  newRenderer.Print(to_hstring((uint16_t)0xFF00cc99));
  newRenderer.Print(" And The Hex Color very shortened is: ");
  newRenderer.Print(to_hstring((uint8_t)0x00cc99));
  newRenderer.Print(".");
}
