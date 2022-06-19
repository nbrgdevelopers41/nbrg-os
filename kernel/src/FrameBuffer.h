#pragma once

#include <stddef.h>

struct Framebuffer {
  void* BaseAddress;
  size_t BufferSize;
  unsigned int Width;
  unsigned int Height;
  unsigned int PixelsPerScanLine;
};

struct PSF1_HEADER {
  unsigned char magic[2];
  unsigned char mode;
  unsigned char charsize;
};

struct PSF1_FONT {
  PSF1_HEADER* psf1_Header;
  void* glyphBuffer;
};