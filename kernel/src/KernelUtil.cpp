#include "KernelUtil.h"
#include "gdt/gdt.h"

// The kernelInfo to be passed back to the back
KernelInfo kernelInfo;

PageTableManager pageTableManager = NULL;

void PrepareMemory(BootInfo* bootInfo) {
  uint64_t mMapEntries = bootInfo->mMapSize / bootInfo->mMapDescSize;

  GlobalAllocator = PageFrameAllocator();
  GlobalAllocator.ReadEFIMemoryMap(bootInfo->Map, bootInfo->mMapSize,
                                   bootInfo->mMapDescSize);

  uint64_t kernelSize = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
  uint64_t kernelPages = (uint64_t)kernelSize / 4096 + 1;

  GlobalAllocator.LockPages(&_KernelStart,
                            kernelPages);  // Locks the kernel memory pages

  PageTable* PML4 = (PageTable*)GlobalAllocator.RequestPage();
  memset(PML4, 0, 0x1000);

  pageTableManager = PageTableManager(PML4);

  uint64_t systemMemorySize =
      GetMemorySize(bootInfo->Map, mMapEntries, bootInfo->mMapDescSize);

  for (uint64_t t = 0; t < systemMemorySize; t += 0x1000) {
    pageTableManager.MapMemory((void*)t, (void*)t);
  }

  uint64_t fbBase = (uint64_t)bootInfo->framebuffer->BaseAddress;
  uint64_t fbSize = (uint64_t)bootInfo->framebuffer->BufferSize + 0x1000;

  uint64_t totalAddrSize = fbBase + fbSize;

  GlobalAllocator.LockPages((void*)fbBase, fbSize / 0x1000 + 1);

  for (uint64_t t = fbBase; t < totalAddrSize; t += 4096) {
    pageTableManager.MapMemory((void*)t, (void*)t);
  }

  // Put the PML4 into the cr3 register.
  asm("mov %0, %%cr3" : : "r"(PML4));

  kernelInfo.pageTableManager = &pageTableManager;
}

KernelInfo InitializeKernel(BootInfo* bootInfo) {
  GDTDescriptor gdtDescriptor;
  gdtDescriptor.Size = sizeof(GDT) - 1;
  gdtDescriptor.Offset = (uint64_t)&DefaultGDT;
  // Load the GDT
  LoadGDT(&gdtDescriptor);

  // Prepares the memory for operations
  PrepareMemory(bootInfo);

  // Set the framebuffer to All black
  // Reason: On some devices with higher RAM displays
  // a weird rainbow line at the top of the screen
  memset(bootInfo->framebuffer->BaseAddress, 0,
         bootInfo->framebuffer->BufferSize);

  return kernelInfo;
}

void Test::testPageTableManager(BasicRenderer newRenderer,
                                PageTableManager pageTableManager) {
  newRenderer.NewLine();

  // newRenderer.Print("Successfully Identity Mapped the whole memory");

  pageTableManager.MapMemory((void*)0x600000000, (void*)0x80000);

  uint64_t* test = (uint64_t*)0x600000000;
  *test = 26;

  newRenderer.Print(to_string(*test));
}

void Test::testPageMapIndexer(BasicRenderer newRenderer) {
  PageMapIndexer pageIndexer = PageMapIndexer(0x1000 * 52 + 0x50000 * 7);

  newRenderer.NewLine();

  newRenderer.Print("PageIndexer data for 0x1000: ");

  newRenderer.Print(to_string(pageIndexer.P_i));
  newRenderer.Print(" - ");
  newRenderer.Print(to_string(pageIndexer.PT_i));
  newRenderer.Print(" - ");
  newRenderer.Print(to_string(pageIndexer.PD_i));
  newRenderer.Print(" - ");
  newRenderer.Print(to_string(pageIndexer.PDP_i));
}

void Test::testPageFrameAllocator(BasicRenderer newRenderer) {
  newRenderer.NewLine();

  newRenderer.Print("Available RAM: ");
  newRenderer.Print(to_string(GlobalAllocator.GetfreeRAM() / 1024));
  newRenderer.Print(" KB");
  newRenderer.NewLine();

  newRenderer.Print("In-Use RAM: ");
  newRenderer.Print(to_string(GlobalAllocator.GetInUseRAM() / 1024));
  newRenderer.Print(" KB");
  newRenderer.NewLine();

  newRenderer.Print("Reserved RAM: ");
  newRenderer.Print(to_string(GlobalAllocator.GetReservedRAM() / 1024));
  newRenderer.Print(" KB");
  newRenderer.NewLine();

  for (int i = 0; i < 20; i++) {
    void* address = GlobalAllocator.RequestPage();
    newRenderer.Print(to_hstring((uint64_t)address));
    newRenderer.NewLine();
  }
  return;
}

void Test::testToStringImplementations(BasicRenderer newRenderer) {
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