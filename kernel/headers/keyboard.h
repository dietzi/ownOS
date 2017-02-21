#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_init(void);
uint8_t translate_scancode(int, uint16_t);
void kbd_irq_handler();

#endif