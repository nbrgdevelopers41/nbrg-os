#include "KernelUtil.h"

extern "C" void _start(BootInfo* bootInfo) {
  BasicRenderer newRenderer =
      BasicRenderer(bootInfo->framebuffer, bootInfo->psf1_Font);

  newRenderer.Print("Initializing kernel...");

  KernelInfo kernelInfo = InitializeKernel(bootInfo);
  PageTableManager* pageTableManager = kernelInfo.pageTableManager;

  newRenderer.ResetCursorPosition();

  newRenderer.Print(
      "Hello User! NBRG Developers welcome you to Rinze-OS. Thank you for "
      "choosing Rinze-OS. We wish to see you around!");

  while (true)
    ;
}
