#ifndef TIMER_H
#define TIMER_H

void pit_init(void);
void register_timer(void* callback, uint32_t arguments[]);
void handle_timer(void);
int timer_ticks;

struct timer {
	void* callback;
	uint32_t ticks;
	bool enabled;
	struct timer* next;
	uint32_t arguments[];
};

#endif