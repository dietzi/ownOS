#include "includes.h"

struct timer* timers = NULL;

void (*timer_cb)(void *arguments);

void pit_init(void) {
	int freq=1000;
	int counter = 1193182 / freq;
	outb(0x43, 0x34);
	outb(0x40,counter & 0xFF);
	outb(0x40,counter >> 8);
}

bool register_timer(void* callback, uint32_t timeout, void *arguments) {
	struct timer* timer_temp;
	if(timers == NULL) {
		timers = pmm_alloc();
		timer_temp->callback = callback;
		timer_temp->ticks = 0;
		timer_temp->timeout = timeout;
		timer_temp->arguments = arguments;
		
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
	
	return true;
}

void unregister_timer(struct timer* timer) {
	struct timer* timer_temp = timers;
	if(timer_temp == timer) {
		pmm_free(timers->arguments);
		pmm_free(timers);
		timers = timers->next;
		return;
	}
	while(timer_temp != NULL) {
		if(timer_temp->next == timer) {
			pmm_free(timers->next->arguments);
			pmm_free(timers->next);
			timer_temp->next = timer_temp->next->next;
			return;
		}
	}
}

void handle_timer(void) {
	struct timer* timer_temp = timers;
	if(timer_temp != NULL) {
		while(timer_temp != NULL) {
			timer_temp->ticks++;
			if(timer_temp->ticks >= timer_temp->timeout) {
				timer_cb = timer_temp->callback;
				timer_cb(timer_temp->arguments);
				if(timer_temp->remove_after_event) {
					unregister_timer(timer_temp);
				} else {
					timer_temp->ticks = 0;
				}
			}
			timer_temp = timer_temp->next;
		}
	}
}