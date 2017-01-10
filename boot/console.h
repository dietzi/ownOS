typedef __builtin_va_list       va_list;
#define va_start(ap, X)         __builtin_va_start(ap, X)
#define va_arg(ap, type)        __builtin_va_arg(ap, type)
#define va_end(ap)              __builtin_va_end(ap)

static inline void outb(unsigned short port, unsigned char data)
{
    asm volatile ("outb %0, %1" : : "a" (data), "Nd" (port));
}

void displaycursor(int row,int col)
{
  unsigned short tmp=row*80+col;
  outb(0x3D4,14);
  outb(0x3D5,tmp >> 8);
  outb(0x3D4,15);
  outb(0x3D5,tmp);
}

void initScreen() {
	int i;
    char* video = (char*) 0xb8000;
	const char iString[]="ownOS> ";

	for (i = 0; iString[i] != '\0'; i++) {
        int pos=0;
        // Zeichen i in den Videospeicher kopieren
        video[pos * 2] = iString[i];
 
        // 0x07 = Grün auf Schwarz
        video[pos * 2 + 1] = 0x02;
		col+=1;
    }
	displaycursor(row,col);
}

void clearscreen()
{

	char* video = (char*) 0xb8000;

	int i;

	//char* video = (char*) 0xb8000;

 
	// C-Strings haben ein Nullbyte als Abschluss

	for (i = 0; i<2000; i++) {


        	// Zeichen i in den Videospeicher kopieren

        	video[i * 2] = 0;

 
        	// 0x07 = Hellgrau auf Schwarz

        	video[i * 2 + 1] = 0x07;

    }


}

void stopCPU() {
    while(1) {
      // Prozessor anhalten
      terminal_writestring("Stopping CPU");
      asm volatile("cli; hlt");
    }
}