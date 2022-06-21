#include <stdint.h>

#include "BasicRenderer.h"
#include "cstr.h"
#include "efiMemory.h"

struct BootInfo {
  Framebuffer* framebuffer;
  PSF1_FONT* psf1_Font;
  EFI_MEMORY_DESCRIPTOR* Map;
  uint64_t mMapSize;
  uint64_t mMapDescSize;
};

extern "C" void _start(BootInfo* bootInfo) {
  BasicRenderer newRenderer =
      BasicRenderer(bootInfo->framebuffer, bootInfo->psf1_Font);

  newRenderer.Print(
      "Hello User! Welcome to NBRG-OS. This is a test line. Thank you.");

  uint64_t numEntries = bootInfo->mMapSize / bootInfo->mMapDescSize;

  for (size_t i = 0; i < numEntries; i++) {
    EFI_MEMORY_DESCRIPTOR* desc =
        (EFI_MEMORY_DESCRIPTOR*)((uint64_t)bootInfo->Map +
                                 i * bootInfo->mMapDescSize);
    newRenderer.NewLine();
    newRenderer.Print(EFI_MEMORY_TYPE_STRINGS[desc->type]);
    newRenderer.Print(" ");
    newRenderer.Print(to_string(desc->numPages * 4096 / 1024));
    newRenderer.Print(" KB");
  }

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
