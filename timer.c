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
	kprintf("timer.c: 18\n");
	struct timer* timer_temp;
	if(timers == NULL) {
		kprintf("timer.c: 21\n");
		timers = pmm_alloc();
		timer_temp = timers;
	} else {
		kprintf("timer.c: 25\n");
		timer_temp = timers;
		kprintf("timer.c: 27\n");
		while(timer_temp->next != NULL) {
			kprintf("timer.c: 29\n");
			timer_temp = timer_temp->next;
		}
		kprintf("timer.c: 32\n");
		timer_temp->next = pmm_alloc();
		timer_temp = timer_temp->next;
	}
	kprintf("timer.c: 36\n");
	timer_temp->callback = callback;
	timer_temp->ticks = 0;
	timer_temp->timeout = timeout;
	timer_temp->arguments = arguments;
	timer_temp->remove_after_event = remove_after_event;
	timer_temp->next = NULL;
	if(timer_temp->callback == NULL) {
		kprintf("Error setting callback\n");
	}
	kprintf("timer.c: 46\n");
}

void unregister_timer(struct timer* timer) {
	kprintf("timer.c: Unregistering Timer\n");
	struct timer* timer_temp = timers;
	if(timer_temp == timer) {
		pmm_free(timers->arguments);
		pmm_free(timers);
		timers = timers->next;
		kprintf("timer.c: Unregistering Timer done\n");
		return;
	}
	while(timer_temp->next != NULL) {
		if(timer_temp->next == timer) {
			pmm_free(timer_temp->next->arguments);
			pmm_free(timer_temp->next);
			timer_temp->next = timer_temp->next->next;
			kprintf("timer.c: Unregistering Timer done\n");
			return;
		}
	}
}

void unregister_timer_by_arguments(void* arguments) {
	kprintf("timer.c: Unregistering Timer by arguments\n");
	struct timer* timer_temp = timers;
	if(timer_temp->arguments == arguments) {
		pmm_free(timers->arguments);
		pmm_free(timers);
		timers = timers->next;
		kprintf("timer.c: Unregistering Timer by arguments done\n");
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
}

void handle_timer(void) {
	struct timer* timer_temp = timers;
	while(timer_temp != NULL) {
		timer_temp->ticks++;
		if(timer_temp->ticks >= timer_temp->timeout) {
			kprintf("Timer: 0x%x\n",timer_temp);
			timer_cb = timer_temp->callback;
			last_message = "timer callback";
			timer_cb(timer_temp->arguments);
			last_message = "timer callback done";
			if(timer_temp->remove_after_event) {
				last_message = "unregister timer";
				unregister_timer(timer_temp);
			} else {
				last_message = "reset timer";
				timer_temp->ticks = 0;
			}
		}
		last_message = "next timer";
		if(timer_temp->next == NULL) break;
		timer_temp = timer_temp->next;
	}
	last_message = "end timer_handle";
}