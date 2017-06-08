#ifndef TIMER_H
#define TIMER_H

void pit_init(void);
bool register_timer(void* callback, uint32_t timeout,  bool remove_after_event, void *arguments);
void handle_timer(void);
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