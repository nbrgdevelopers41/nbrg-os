#include "BasicRenderer.h"

BasicRenderer::BasicRenderer(Framebuffer* targetFramebuffer,
                             PSF1_FONT* psf1_font) {
  TargetFramebuffer = targetFramebuffer;
  PSF1_Font = psf1_font;
  Color = 0xff00cc99;
  CursorPosition = {50, 50};
}

void BasicRenderer::Print(const char* str) {
  char* chr = (char*)str;
  unsigned int width = TargetFramebuffer->Width;

  while (*chr != 0) {
    PutChar(*chr, CursorPosition.X, CursorPosition.Y);
    CursorPosition.X += 8;
    if (CursorPosition.X >= (width * 0.93)) {
      CursorPosition.X = 50;
      CursorPosition.Y += 18;
    }
    chr++;
  }
}

void BasicRenderer::PutChar(char chr,
                            unsigned int xOffset,
                            unsigned int yOffset) {
  unsigned int* pixPtr = (unsigned int*)TargetFramebuffer->BaseAddress;
  char* fontPtr =
      (char*)PSF1_Font->glyphBuffer + (chr * PSF1_Font->psf1_Header->charsize);
  for (unsigned long y = yOffset; y < yOffset + 18; y++) {
    for (unsigned long x = xOffset; x < xOffset + 8; x++) {
      if ((*fontPtr & (0b10000000 >> (x - xOffset))) > 0) {
        // The bit is on.
        *(unsigned int*)(pixPtr + x +
                         (y * TargetFramebuffer->PixelsPerScanLine)) = Color;
      }
    }
    fontPtr++;
  }
}