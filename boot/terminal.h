void terminal_initialize(void);
void terminal_writestring(const char*);
static inline void outb(unsigned short, unsigned char);
void displaycursor(int,int);
void clearscreen();
void stopCPU();