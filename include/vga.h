#ifndef VGA_H
#define VGA_H

/**
@brief Struktur für Informationen aus einem VESA-Modus
*/
typedef struct MODE_INFO
{
	unsigned short ModeAttributes       __attribute__ ((packed));
	unsigned char  WinAAttributes       __attribute__ ((packed));
	unsigned char  WinBAttributes       __attribute__ ((packed));
	unsigned short WinGranularity       __attribute__ ((packed));
	unsigned short WinSize              __attribute__ ((packed));
	unsigned short WinASegment          __attribute__ ((packed));
	unsigned short WinBSegment          __attribute__ ((packed));
	unsigned long  WinFuncPtr           __attribute__ ((packed));
	unsigned short BytesPerScanLine     __attribute__ ((packed));
	unsigned short XResolution          __attribute__ ((packed));
	unsigned short YResolution          __attribute__ ((packed));
	unsigned char  XCharSize            __attribute__ ((packed));
	unsigned char  YCharSize            __attribute__ ((packed));
	unsigned char  NumberOfPlanes       __attribute__ ((packed));
	unsigned char  BitsPerPixel         __attribute__ ((packed));
	unsigned char  NumberOfBanks        __attribute__ ((packed));
	unsigned char  MemoryModel          __attribute__ ((packed));
	unsigned char  BankSize             __attribute__ ((packed));
	unsigned char  NumberOfImagePages   __attribute__ ((packed));
	unsigned char  Reserved_page        __attribute__ ((packed));
	unsigned char  RedMaskSize          __attribute__ ((packed));
	unsigned char  RedMaskPos           __attribute__ ((packed));
	unsigned char  GreenMaskSize        __attribute__ ((packed));
	unsigned char  GreenMaskPos         __attribute__ ((packed));
	unsigned char  BlueMaskSize         __attribute__ ((packed));
	unsigned char  BlueMaskPos          __attribute__ ((packed));
	unsigned char  ReservedMaskSize     __attribute__ ((packed));
	unsigned char  ReservedMaskPos      __attribute__ ((packed));
	unsigned char  DirectColorModeInfo  __attribute__ ((packed));
	unsigned long  PhysBasePtr          __attribute__ ((packed));
	unsigned long  OffScreenMemOffset   __attribute__ ((packed));
	unsigned short OffScreenMemSize     __attribute__ ((packed));
	unsigned char  Reserved[206]        __attribute__ ((packed));
} MODE_INFO;

/**
@brief Struktur für die VESA-Informationen
*/
typedef struct VESA_INFO
{ 
	unsigned char  VESASignature[4]     __attribute__ ((packed));
	unsigned short VESAVersion          __attribute__ ((packed));
	unsigned long  OEMStringPtr         __attribute__ ((packed));
	unsigned char  Capabilities[4]      __attribute__ ((packed));
	unsigned long  VideoModePtr         __attribute__ ((packed));
	unsigned short TotalMemory          __attribute__ ((packed));
	unsigned short OemSoftwareRev       __attribute__ ((packed));
	unsigned long  OemVendorNamePtr     __attribute__ ((packed));
	unsigned long  OemProductNamePtr    __attribute__ ((packed));
	unsigned long  OemProductRevPtr     __attribute__ ((packed));
	unsigned char  Reserved[222]        __attribute__ ((packed));
	unsigned char  OemData[256]         __attribute__ ((packed));
} VESA_INFO;

typedef unsigned char  BYTE; // 1byte
typedef unsigned short  WORD; // 2bytes
typedef unsigned long  DWORD; //4bytes

long char_pos_x;
long char_pos_y;
uint8_t* vga;

uint16_t* get_vesa_modes(void);
struct MODE_INFO *get_vesa_mode_info(uint16_t mode);
void set_vesa_mode(uint16_t mode);
volatile void draw_char(char *c, int x, int y, uint32_t fgcolor, uint32_t bgcolor);
volatile void PutPixel(int x, int y, uint32_t color);
void draw_rectangle_filled(int x1, int y1, int x2, int y2, uint32_t color);
void draw_rectangle(int x1, int y1, int x2, int y2, int border, uint32_t color);
void draw_circle(int x0, int y0, int radius, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_ellipse(int xm, int ym, int a, int b, uint32_t color);
uint32_t makecol(uint8_t r, uint8_t g, uint8_t b);

#endif