#include "includes.h"

struct timer timers[1000];

void pit_init(void) {
	int freq=1000;
	int counter = 1193182 / freq;
	outb(0x43, 0x34);
	outb(0x40,counter & 0xFF);
	outb(0x40,counter >> 8);
}

void register_timer(void* callback, uint32_t arguments[]) {
	
}

void handle_timer(void) {
	
}