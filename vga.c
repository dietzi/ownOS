/**
@file vga.c
@brief Hier werden die Grafikfunktionen bereitgestellt
*/

#include "includes.h"
#include "fonts.h"

/** @brief int32() führt einen Bios-Interrupt im Real-Mode aus */
extern void int32(unsigned char intnum, regs16_t *regs);

/** @brief diese Funktion holt die VESA-Informationen */

uint16_t* get_vesa_modes(void) {
	kprintf("Disabling NIC\n");
	pci_bdf_t addr1 = {
		.bus=0,
		.dev=17,
		.func=0
	};	
	pci_bdf_t addr2 = {
		.bus=0,
		.dev=1,
		.func=0
	};
	pci_bdf_t addr = {
		.bus=0,
		.dev=18,
		.func=0
	};
	int base = 0;
	pci_write_register_16(addr,base,0x08, 0x04);
	pci_config_write_8(addr1,0x51,0x0d); //0x3d
	pci_config_write_8(addr2,0,0x04,pci_config_read_8(addr2,0,0x04) | 0x04);
	kprintf("NIC disabled\n");

	struct VESA_INFO *vesa=(VESA_INFO *)pmm_alloc();
	struct MODE_INFO *info=(MODE_INFO *)pmm_alloc();
	memcpy(vesa->VESASignature,"VBE2",4);
	regs16_t regs;
	regs.ax=0x4f00;
	regs.di=vesa;
	regs.es=0;
	int32(0x10,&regs);
	if(regs.ax!=0x004f) {
		kprintf("VBE Error\n");
		return false;
	}
	
	uint8_t major=(vesa->VESAVersion >> 8) & 0xff;
	uint8_t minor=vesa->VESAVersion & 0xff;
	
	kprintf("Signature: %c",vesa->VESASignature[0]);
	kprintf("%c",vesa->VESASignature[1]);
	kprintf("%c",vesa->VESASignature[2]);
	kprintf("%c\n",vesa->VESASignature[3]);
	kprintf("VBE-Version: %d.%d\n",major,minor);
	
	uint16_t *modes = ((vesa->VideoModePtr & 0xFFFF0000) >> 12) + (vesa->VideoModePtr & 0xFFFF);
	while(*modes!=0xffff) {
		kprintf("\nMode %x",*modes);
		regs.ax=0x4f01;
		regs.cx=*modes;
		regs.es=0;
		regs.di=info;
		int32(0x10,&regs);
		if(regs.ax !=0x004f) {
			kprintf(" Error\n");
			continue;
		}
		*modes++;
		
		uint16_t *addr=(uint8_t*) (uintptr_t)info->PhysBasePtr;
		kprintf(" --> %dx%dx%d Addr: %x",info->XResolution,info->YResolution,info->BitsPerPixel,(char*)info->PhysBasePtr);		
	}
	kprintf("\n");

	start_nic();
}

/** @brief diese Funktion holt die VESA-Mode-Informationen */

struct MODE_INFO *get_vesa_mode_info(uint16_t mode) {
	struct MODE_INFO *info=(MODE_INFO *)pmm_alloc();
	regs16_t regs;
	regs.ax=0x4f01;
	regs.cx=mode;
	regs.es=0;
	regs.di=info;
	int32(0x10,&regs);
	if(regs.ax !=0x004f) {
		kprintf("Error\n");
		return -1;
	} else {
		kprintf("Mode: %x\n",mode);
	}
	kprintf(" --> %dx%dx%d Addr: %x\n",info->XResolution,info->YResolution,info->BitsPerPixel,(char*)info->PhysBasePtr);		
	
	return info;
}

unsigned char *font=LSB;
extern getfont(unsigned char *font);
struct MODE_INFO *info_set;

/** @brief diese Funktion setzt den VESA-Mode */

void set_vesa_mode(uint16_t mode) {
	char_pos_x=0;
	char_pos_y=0;
	info_set=get_vesa_mode_info(mode);
	vga=(uint8_t*) (uintptr_t)info_set->PhysBasePtr;
	regs16_t regs;
	regs.ax = 0x4f02;
	regs.bx = mode;
	int32(0x10,&regs);
	
	draw_rectangle_filled(0,0,info_set->XResolution,info_set->YResolution,0x888888);
	/*draw_rectangle_filled(50,50,350,350,0xBBBBBB);
	draw_rectangle(400,50,700,350,1,0x000000);
	draw_rectangle(750,50,1050,350,10,0x000000);
	draw_circle(500,500,100,0x000000);
	draw_line(0,info_set->YResolution,info_set->XResolution,0,0x000000);
	draw_ellipse(500,700,100,50,0x000000);*/
	/*int r=0,g=0,b=0;
	bool dir=false;
	uint32_t color=0x000000;
	for(int y=0;y<info_set->YResolution;y++) {
		for(int x=0;x<info_set->XResolution;x++) {
			PutPixel(x,y,(uint32_t)(0x00 | (b << 4) | (g << 8) | (r << 16)));
			if(dir) {
				r++;g++;b++;
				if(r > 255) {
					r--;g--;b--;
					dir=false;
				}
			} else {
				r--;g--;b--;
				if(r < 0) {
					r++;g++;b++;
					dir=true;
				}
			}
			//color+=(0xffffff / (info_set->XResolution * info_set->YResolution));
		}
	}*/
	
	video_active=true;
}

uint32_t makecol(uint8_t r, uint8_t g, uint8_t b) {
	uint32_t temp=0x00 | (b << 4) | (g << 8) | (r << 16);
	kprintf("%x\n",temp);
	return temp;
}

void draw_rectangle(int x1, int y1, int x2, int y2, int border, uint32_t color) {
	for(int i=y1;i<=y2;i++) {
		for(int j=1;j<=border;j++) {
			PutPixel(x1-1+j,i,color);
			PutPixel(x2+1-j,i,color);
		}
	}
	for(int i=x1;i<=x2;i++) {
		for(int j=1;j<=border;j++) {
			PutPixel(i,y1-1+j,color);
			PutPixel(i,y2+1-j,color);
		}
	}
}

void draw_rectangle_filled(int x1, int y1, int x2, int y2, uint32_t color) {
	for(int y=y1;y<y2;y++) {
		for(int x=x1;x<x2;x++) {
			PutPixel(x,y,color);
		}
	}
}

void draw_circle(int x0, int y0, int radius, uint32_t color) {
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	PutPixel(x0, y0 + radius, color);
	PutPixel(x0, y0 - radius, color);
	PutPixel(x0 + radius, y0, color);
	PutPixel(x0 - radius, y0, color);

	while(x < y) {
		if(f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;

		PutPixel(x0 + x, y0 + y, color);
		PutPixel(x0 - x, y0 + y, color);
		PutPixel(x0 + x, y0 - y, color);
		PutPixel(x0 - x, y0 - y, color);
		PutPixel(x0 + y, y0 + x, color);
		PutPixel(x0 - y, y0 + x, color);
		PutPixel(x0 + y, y0 - x, color);
		PutPixel(x0 - y, y0 - x, color);
	}
}

volatile void draw_char(char *c, int x, int y, uint32_t fgcolor, uint32_t bgcolor) {
	bool use_global_x=false;
	bool use_global_y=false;
	if(x==-1) {
		use_global_x=true;
	}
	if(y==-1) {
		use_global_y=true;
	}
	int cx,cy;
	int mask[8]={1,2,4,8,16,32,64,128};
	while(*c!=NULL && *c!='\0') {
		char cc=*c;
		char *glyph=LSB[((int)cc)];
		
		if(use_global_x) x=char_pos_x;
		if(use_global_y) y=char_pos_y;
		
		for(cy=0;cy<14;cy++){
			for(cx=0;cx<8;cx++){
				//if(glyph[cy]&mask[cx]) PutPixel(x+cx,y+cy,fgcolor);
				PutPixel(x+cx,y+cy/*-12*/,glyph[cy]&mask[cx]?fgcolor:bgcolor);
				//kprintf(glyph[cy]&mask[cx]?"#":" ");
			}
			//kprintf("\n");
		}
		if(use_global_x) char_pos_x+=8;
		if(char_pos_x>info_set->XResolution-8) {
			if(use_global_x) char_pos_x=0;
			if(use_global_y) char_pos_y+=14;
		}
		*c++;
	}
}

void draw_ellipse(int xm, int ym, int a, int b, uint32_t color) {
   int dx = 0, dy = b; /* im I. Quadranten von links oben nach rechts unten */
   long a2 = a * a, b2 = b * b;
   long err = b2-(2 * b-1) * a2, e2; /* Fehler im 1. Schritt */

   do {
       PutPixel(xm+dx, ym+dy,color); /* I. Quadrant */
       PutPixel(xm-dx, ym+dy,color); /* II. Quadrant */
       PutPixel(xm-dx, ym-dy,color); /* III. Quadrant */
       PutPixel(xm+dx, ym-dy,color); /* IV. Quadrant */

       e2 = 2 * err;
       if (e2 <  (2 * dx+1) * b2) { dx++; err += (2 * dx+1) * b2; }
       if (e2 > -(2 * dy-1) * a2) { dy--; err -= (2 * dy-1) * a2; }
   } while (dy >= 0);

   while (dx++ < a) { /* fehlerhafter Abbruch bei flachen Ellipsen (b=1) */
       PutPixel(xm+dx, ym,color); /* -> Spitze der Ellipse vollenden */
       PutPixel(xm-dx, ym,color);
   }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
  int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = dx+dy, e2; /* error value e_xy */

  while(1){
    PutPixel(x0,y0,color);
    if (x0==x1 && y0==y1) return;
    e2 = 2*err;
    if (e2 > dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 < dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}

/** @brief diese Funktion zeichnet einen Pixel */

volatile void PutPixel(int x, int y, uint32_t color) {
	if(color >= 0x000000 && color <= 0xFFFFFF) *(uint32_t *)(vga + y * info_set->BytesPerScanLine + x * (info_set->BitsPerPixel / 8)) = color;
}