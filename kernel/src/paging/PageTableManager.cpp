#include "PageTableManager.h"
#include <stdint.h>
#include "../memory.h"
#include "PageFrameAllocator.h"
#include "PageMapIndexer.h"
#include "paging.h"

PageTableManager::PageTableManager(PageTable* PML4Address) {
  this->PML4 = PML4Address;
}

void PageTableManager::MapMemory(void* virtualMemory, void* physicalMemory) {
  // Map the virtual memory address to the physical memory address
  PageMapIndexer indexer = PageMapIndexer((uint64_t)virtualMemory);
  PageDirectoryEntry PDE;

  /// Setting up the Page Directory Pointer Table entry.
  PDE = PML4->entries[indexer.PDP_i];
  PageTable* PDP;
  if (!PDE.GetFlag(PT_Flags::Present)) {
    PDP = (PageTable*)GlobalAllocator.RequestPage();
    memset(PDP, 0, 0x1000);
    PDE.SetAddress((uint64_t)PDP >> 12);
    PDE.SetFlag(PT_Flags::Present, true);
    PDE.SetFlag(PT_Flags::ReadWrite, true);
    PML4->entries[indexer.PDP_i] = PDE;
  } else {
    PDP = (PageTable*)((uint64_t)PDE.GetAddress() << 12);
  }

  // Setting up the Page Directory Table entry
  PDE = PDP->entries[indexer.PD_i];
  PageTable* PD;
  if (!PDE.GetFlag(PT_Flags::Present)) {
    PD = (PageTable*)GlobalAllocator.RequestPage();
    memset(PD, 0, 0x1000);
    PDE.SetAddress((uint64_t)PD >> 12);
    PDE.SetFlag(PT_Flags::Present, true);
    PDE.SetFlag(PT_Flags::ReadWrite, true);
    PDP->entries[indexer.PD_i] = PDE;
  } else {
    PD = (PageTable*)((uint64_t)PDE.GetAddress() << 12);
  }

  // Setting up the Page Table entry
  PDE = PD->entries[indexer.PT_i];
  PageTable* PT;
  if (!PDE.GetFlag(PT_Flags::Present)) {
    PT = (PageTable*)GlobalAllocator.RequestPage();
    memset(PT, 0, 0x1000);
    PDE.SetAddress((uint64_t)PT >> 12);
    PDE.SetFlag(PT_Flags::Present, true);
    PDE.SetFlag(PT_Flags::ReadWrite, true);
    PD->entries[indexer.PT_i] = PDE;
  } else {
    PT = (PageTable*)((uint64_t)PDE.GetAddress() << 12);
  }

  /// Set the Page entry.
  PDE = PT->entries[indexer.P_i];
  PDE.SetAddress((uint64_t)physicalMemory >> 12);
  PDE.SetFlag(PT_Flags::Present, true);
  PDE.SetFlag(PT_Flags::ReadWrite, true);
  PT->entries[indexer.P_i] = PDE;
}