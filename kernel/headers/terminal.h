#ifndef TERMINAL_H
#define TERMINAL_H

void terminal_initialize(void);
void terminal_writestring(const char*);
void outb(unsigned short, unsigned char);
void displaycursor(int,int);
void clearscreen();
void stopCPU();
char * itoa (int, int);
unsigned char inb(unsigned short);
bool request_ports(uint32_t, uint32_t);
void terminal_key(char);
void *kmemset(void*, int, size_t);
void terminal_write(char*, size_t);
void terminal_key(char);
void terminal_putchar(char);
void terminal_putentryat(char, uint8_t, size_t, size_t);

#endif