#ifndef TIMER_H
#define TIMER_H

void pit_init(void);
void register_timer(void* callback, uint32_t timeout, bool remove_after_event, void *arguments);
void handle_timer(void);
bool unregister_timer_by_arguments(void* arguments);
int timer_ticks;

struct timer {
	void* callback;
	uint32_t ticks;
	uint32_t timeout;
	bool remove_after_event;
	struct timer* next;
	uint32_t *arguments;
};

#endif