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

void kprintf(const char string[], int row, int col)
{
    int i;
    
    char* video = (char*) 0xb8000;
 
    for (i = 0; string[i] != '\0'; i++) {
        int pos=row*80+col+i;
        // Zeichen i in den Videospeicher kopieren
        video[pos * 2] = string[i];
 
        // 0x07 = Hellgrau auf Schwarz
        video[pos * 2 + 1] = 0x07;
        displaycursor(row,col+i+1);
    }
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
      kprintf("Stopping CPU",20,0);
      asm volatile("cli; hlt");
    }
}

struct cpu_state {
    // Von Hand gesicherte Register
    uint32_t   eax;
    uint32_t   ebx;
    uint32_t   ecx;
    uint32_t   edx;
    uint32_t   esi;
    uint32_t   edi;
    uint32_t   ebp;
 
    uint32_t   intr;
    uint32_t   error;
 
    // Von der CPU gesichert
    uint32_t   eip;
    uint32_t   cs;
    uint32_t   eflags;
    uint32_t   esp;
    uint32_t   ss;
};