void terminal_initialize(void);
void terminal_writestring(const char*);
void outb(unsigned short, unsigned char);
void displaycursor(int,int);
void clearscreen();
void stopCPU();
char * itoa (int, char, int);
unsigned char inb(unsigned short);
bool request_ports(uint32_t, uint32_t);