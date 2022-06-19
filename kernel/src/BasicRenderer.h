#pragma once
#include "FrameBuffer.h"
#include "RenderHelper.h"

class BasicRenderer {
 public:
  BasicRenderer(Framebuffer* targetFramebuffer, PSF1_FONT* psf1_font);
  void Print(const char* str);
  Coordinate CursorPosition;
  Framebuffer* TargetFramebuffer;
  PSF1_FONT* PSF1_Font;
  unsigned int Color;
  void PutChar(char chr, unsigned int xOffset, unsigned int yOffset);
};