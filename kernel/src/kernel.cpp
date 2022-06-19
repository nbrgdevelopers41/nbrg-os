#include <stdint.h>

#include "BasicRenderer.h"
#include "cstr.h"

extern "C" void _start(Framebuffer* framebuffer, PSF1_FONT* psf1_Font) {
  BasicRenderer newRenderer = BasicRenderer(framebuffer, psf1_Font);

  newRenderer.Print(
      "Hello User! Welcome to NBRG-OS. This is a test line. Thank you.");

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

  return;
}
