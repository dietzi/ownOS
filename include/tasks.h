#ifndef TASKS_H
#define TASKS_H

enum task_state {
	RUNNING=1,
	EXIT=2
};

enum task_type {
	IDLE=1,
	V86=2,
	NORMAL=3
};


struct vmm_context {
    uint32_t* pagedir;
};


/**
@brief Struktur f�r einen Task

@param cpu_state gibt den aktuellen CPU-Zustand des Tasks zur�ck
@param next gibt den n�chsten Task zur�ck
@param pid ist die Task-ID
**/
struct task {
    struct cpu_state*   cpu_state;
    struct cpu_state*   last_cpu_state;
    struct task*        next;
	int					pid;
	enum task_type		type;
	enum task_state		state;
	char*				name;
	struct vmm_context* context;
};

void task_a(uint32_t arg);
void task_b(uint32_t arg);
void exit(void);
bool remove_task_by_pid(int pid);

#endif