#include "includes.h"

struct timer* timers = NULL;

void pit_init(void) {
	int freq=1000;
	int counter = 1193182 / freq;
	outb(0x43, 0x34);
	outb(0x40,counter & 0xFF);
	outb(0x40,counter >> 8);
}

bool register_timer(void* callback, uint32_t timeout, uint32_t *arguments) {
	struct timer* timer_temp;
	if(timers == NULL) {
		timers = pmm_alloc();
		
		return true;
	}
	timer_temp = timers;
	while(timer_temp->next != NULL) {
		timer_temp = timer_temp->next;
	}
	timer_temp->next = pmm_alloc();
	timer_temp = timer_temp->next;
	timer_temp->callback = callback;
	timer_temp->ticks = 0;
	timer_temp->timeout = timeout;
	timer_temp->arguments = arguments;
}

void handle_timer(void) {
	
}