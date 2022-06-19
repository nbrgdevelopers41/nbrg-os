#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <stddef.h>

// Custom Structs
typedef struct {
  void* BaseAddress;
  size_t BufferSize;
  unsigned int Width;
  unsigned int Height;
  unsigned int PixelsPerScanLine;
} Framebuffer;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
  unsigned char magic[2];
  unsigned char mode;
  unsigned char charsize;
} PSF1_HEADER;  // The header for the [PSF1_FONT]

typedef struct {
  PSF1_HEADER* psf1_Header;
  void* glyphBuffer;
} PSF1_FONT;  // The PSF1 font

// Prototypes
int memcmp(const void* aptr,
           const void* bptr,
           size_t n);  // Memory compares the given arguments.
EFI_FILE* LoadFile(EFI_FILE* Directory,
                   CHAR16* Path,
                   EFI_HANDLE ImageHandle,
                   EFI_SYSTEM_TABLE* SystemTable);  // Loads a file
Framebuffer* InitializeGOP();  // Initializes GOP (Graphics Output Protocol)

PSF1_FONT* LoadPSF1Font(EFI_FILE* Directory,
                        CHAR16* Path,
                        EFI_HANDLE ImageHandle,
                        EFI_SYSTEM_TABLE* SystemTable);  // Loads the PSF1_FONT

// The main EFI function/entrypoint
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
  InitializeLib(ImageHandle, SystemTable);  // Initialize the UEFI Libraries.

  EFI_FILE* KernelFile =
      LoadFile(NULL, L"kernel.elf", ImageHandle,
               SystemTable);  // Load the Kernel File, kernel.elf

  if (KernelFile == NULL) {
    Print(L"Could Not Load kernel\n\r");
  } else {
    Print(L"Kernel Read Completed. Continuing boot.\n\r");
  }

  // the header of the [KernelFile]
  Elf64_Ehdr header;
  // Read the file header in this block
  {
    UINTN FileInfoSize;      // the file info object's size
    EFI_FILE_INFO FileInfo;  // the file information

    KernelFile->GetInfo(KernelFile, &gEfiFileInfoGuid, &FileInfoSize, NULL);

    SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize,
                                            (void**)&FileInfo);

    UINTN size = sizeof(header);

    KernelFile->Read(KernelFile, &size, &header);
  }

  // Check if we got a valid Kernel file and not any random file.
  if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
      header.e_ident[EI_CLASS] != ELFCLASS64 ||
      header.e_ident[EI_DATA] != ELFDATA2LSB || header.e_type != ET_EXEC ||
      header.e_machine != EM_X86_64 || header.e_version != EV_CURRENT) {
    Print(L"Kernel format is bad.\n\r");
  } else {
    Print(L"Kernel header successfully verified.\n\r");
  }

  // The Elf64 headers
  Elf64_Phdr* phdrs;
  {
    KernelFile->SetPosition(KernelFile, header.e_phoff);
    UINTN size = header.e_phnum * header.e_phentsize;

    SystemTable->BootServices->AllocatePool(EfiLoaderData, size,
                                            (void**)&phdrs);
    KernelFile->Read(KernelFile, &size, phdrs);
  }

  for (Elf64_Phdr* phdr = phdrs;
       (char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize;
       phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)) {
    switch (phdr->p_type) {
      case PT_LOAD: {
        int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
        Elf64_Addr segment = phdr->p_paddr;
        SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData,
                                                 pages, &segment);

        KernelFile->SetPosition(KernelFile, phdr->p_offset);
        UINTN size = phdr->p_filesz;
        KernelFile->Read(KernelFile, &size, (void**)segment);
        break;
      }

      default:
        break;
    }
  }

  Print(L"Kernel Loaded Successfully. Continuing Boot.\n\r");

  // The reference to the Kernel
  void (*Kernel)(Framebuffer*, PSF1_FONT*) =
      ((__attribute__((sysv_abi)) void (*)(Framebuffer*,
                                           PSF1_FONT*))header.e_entry);

  // Load the console fonts
  PSF1_FONT* zapLight18Font =
      LoadPSF1Font(NULL, L"zap-light18.psf", ImageHandle, SystemTable);

  // Checking if the font is NULL, and if so, we end the boot.
  if (zapLight18Font == NULL) {
    Print(L"The Console Font is not valid or is not found\n\r");
  } else {
    Print(L"Console Font Found. Proceeding to Initialize GOP.\n\r");
  }

  Framebuffer* newBuffer =
      InitializeGOP();  // Getting the FrameBuffer and Initializing GOP.

  // Printing the value of base address, size, width & height of the screen and
  // the pixels per scan line.
  Print(
      L"Base: 0x%x\n\rSize: 0x%x\n\rWidth: %d\n\rHeight: "
      L"%d\n\rPixelsPerScanLine: %d\n\r",
      newBuffer->BaseAddress, newBuffer->BufferSize, newBuffer->Width,
      newBuffer->Height, newBuffer->PixelsPerScanLine);

  Kernel(newBuffer, zapLight18Font);

  return EFI_SUCCESS;  // EXIT THE Operating System.
}

int memcmp(const void* aptr, const void* bptr, size_t n) {
  const unsigned char* a = aptr;
  const unsigned char* b = bptr;

  for (size_t i = 0; i < n; i++) {
    if (a[i] < b[i]) {
      return -1;
    } else if (a[i] > b[i]) {
      return 1;
    }
  }

  return 0;
}

EFI_FILE* LoadFile(EFI_FILE* Directory,
                   CHAR16* Path,
                   EFI_HANDLE ImageHandle,
                   EFI_SYSTEM_TABLE* SystemTable) {
  EFI_FILE* LoadedFile;

  EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;

  SystemTable->BootServices->HandleProtocol(
      ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;

  SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle,
                                            &gEfiSimpleFileSystemProtocolGuid,
                                            (void**)&FileSystem);

  if (Directory == NULL) {
    FileSystem->OpenVolume(FileSystem, &Directory);
  }

  EFI_STATUS s = Directory->Open(Directory, &LoadedFile, Path,
                                 EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);

  if (s != EFI_SUCCESS) {
    return NULL;
  }
  return LoadedFile;
}
// The framebuffer object whose pointer will be returned by the
// [InitializeGop] Function
Framebuffer framebuffer;

Framebuffer* InitializeGOP() {
  EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
  EFI_STATUS status;

  status =
      uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);

  if (EFI_ERROR(status)) {
    Print(L"Error! Unable to locate GOP\n\r");
    return NULL;
  } else {
    Print(L"GOP Located, Starting Graphics\n\r");
  }

  framebuffer.BaseAddress = (void*)gop->Mode->FrameBufferBase;
  framebuffer.BufferSize = gop->Mode->FrameBufferSize;
  framebuffer.Width = gop->Mode->Info->HorizontalResolution;
  framebuffer.Height = gop->Mode->Info->VerticalResolution;
  framebuffer.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

  return &framebuffer;
}

// Loads the PSF1_FONT, which is a decoder for the PSF1 types of fonts.
PSF1_FONT* LoadPSF1Font(EFI_FILE* Directory,
                        CHAR16* Path,
                        EFI_HANDLE ImageHandle,
                        EFI_SYSTEM_TABLE* SystemTable) {
  EFI_FILE* font = LoadFile(Directory, Path, ImageHandle, SystemTable);
  if (font == NULL)
    return NULL;

  PSF1_HEADER* fontHeader;
  SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER),
                                          (void**)&fontHeader);
  UINTN size = sizeof(PSF1_HEADER);
  font->Read(font, &size, fontHeader);

  if (fontHeader->magic[0] != PSF1_MAGIC0 ||
      fontHeader->magic[1] != PSF1_MAGIC1) {
    return NULL;
  }

  UINTN glyphBufferSize = fontHeader->charsize * 256;

  if (fontHeader->mode == 1) {
    // 512 glyph mode
    glyphBufferSize = fontHeader->charsize * 512;
  }

  void* glyphBuffer;
  {
    font->SetPosition(font, sizeof(PSF1_HEADER));
    SystemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize,
                                            (void**)&glyphBuffer);
    font->Read(font, &glyphBufferSize, glyphBuffer);
  }

  PSF1_FONT* finishedFont;
  SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT),
                                          (void**)&finishedFont);
  finishedFont->psf1_Header = fontHeader;
  finishedFont->glyphBuffer = glyphBuffer;

  return finishedFont;
}