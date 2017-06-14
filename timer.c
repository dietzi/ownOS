#include "includes.h"

struct timer* timers;

void (*timer_cb)(void *arguments);

void pit_init(void) {
	int freq=1000;
	int counter = 1193182 / freq;
	outb(0x43, 0x34);
	outb(0x40,counter & 0xFF);
	outb(0x40,counter >> 8);
	timers = NULL;
	timers->next = NULL;
}

void register_timer(void* callback, uint32_t timeout, bool remove_after_event, void *arguments) {
	kprintf("Registering Timer\n");
	struct timer* timer_temp = timers;
	while(timer_temp != NULL) {
		timer_temp = timer_temp->next;
	}
	timer_temp = pmm_alloc();
	timer_temp->callback = callback;
	timer_temp->ticks = 0;
	timer_temp->timeout = timeout;
	timer_temp->arguments = arguments;
	timer_temp->remove_after_event = remove_after_event;
	timer_temp->next = NULL;
	if(timer_temp->callback == NULL) {
		kprintf("Error setting callback\n");
	}
	kprintf("Registering Timer done\n");
	kprintf("Timers: ");
	timer_temp = timers;
	while(timer_temp != NULL) {
		kprintf("0x%x ",timer_temp);
		timer_temp = timer_temp->next;
	}
	kprintf("\n");
}

void unregister_timer(struct timer* timer) {
	kprintf("Unregistering Timer\n");
	struct timer* timer_temp = timers;
	if(timer_temp == timer) {
		pmm_free(timers->arguments);
		pmm_free(timers);
		timers = timers->next;
		return;
	}
	while(timer_temp->next != NULL) {
		if(timer_temp->next == timer) {
			pmm_free(timers->next->arguments);
			pmm_free(timers->next);
			timer_temp->next = timer_temp->next->next;
			return;
		}
	}
	kprintf("Unregistering Timer done\n");
}

void unregister_timer_by_arguments(void* arguments) {
	kprintf("Unregistering Timer by arguments\n");
	struct timer* timer_temp = timers;
	if(timer_temp->arguments == arguments) {
		pmm_free(timers->arguments);
		pmm_free(timers);
		timers = timers->next;
		return;
	}
	while(timer_temp->next != NULL) {
		if(timer_temp->next->arguments == arguments) {
			pmm_free(timer_temp->next->arguments);
			pmm_free(timer_temp->next);
			timer_temp->next = timer_temp->next->next;
			return;
		}
		timer_temp = timer_temp->next;
	}
	kprintf("Unregistering Timer by arguments done\n");
}

void handle_timer(void) {
	struct timer* timer_temp = timers;
	if(timer_temp != NULL) {
		while(timer_temp != NULL) {
			timer_temp->ticks++;
			if(timer_temp->ticks >= timer_temp->timeout) {
				kprintf("a\n");
				timer_cb = timer_temp->callback;
				last_message = "timer callback";
				kprintf("b\n");
				timer_cb(timer_temp->arguments);
				last_message = "timer callback done";
				if(timer_temp->remove_after_event) {
					last_message = "unregister timer";
					unregister_timer(timer_temp);
				} else {
					last_message = "reset timer";
					timer_temp->ticks = 0;
				}
				kprintf("c\n");
			}
			last_message = "next timer";
			timer_temp = timer_temp->next;
		}
	}
	last_message = "end timer_handle";
}