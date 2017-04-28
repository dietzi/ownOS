#include "includes.h"

// tell compiler our int32 function is external
extern void int32(unsigned char intnum, regs16_t *regs);
unsigned char* VGA1 = (unsigned char*) 0xA0000;
extern int x;
extern int y;

// int32 test
void change_to_video()
{
    regs16_t regs;
	//            x   y  color
    // switch to 320x200x256 graphics mode
    regs.ax = 0x0013;
    int32(0x10, &regs);
    
	video_active = true;
	
	  // full screen with blue color (1)
    //memset((char *)0xA0000, 1, (320*200));
     
    // draw horizontal line from 100,80 to 100,240 in multiple colors
    //for(y = 0; y < 200; y++)
        //memset((char *)0xA0000 + (y*320+80), y, 160);
	
    // wait for key
    /*regs.ax = 0x0000;
    int32(0x16, &regs);
     
    // switch to 80x25x16 text mode
    regs.ax = 0x0003;
    int32(0x10, &regs);*/
}

void change_to_text() {
    regs16_t regs;

    regs.ax = 0x0003;
    int32(0x10, &regs);
	x=0;
	y=0;
	
	video_active = false;
}