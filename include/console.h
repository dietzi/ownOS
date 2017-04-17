#ifndef CONSOLE_H
#define CONSOLE_H

void clrscr(void);
void clrscr_color(char color);
int kprintf(const char* fmt, ...);
void set_color(char color);

#endif
