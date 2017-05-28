#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEY_RESET 55555

void keyboard_init(void);
uint8_t translate_scancode(int, uint16_t);
struct cpu_state* kbd_irq_handler(struct cpu_state*);
char codeToChar(uint8_t,bool,bool,bool,bool,bool);
bool wait_for_any_key();
unsigned char inb(unsigned short port);

#endif