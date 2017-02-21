#ifndef MULTITASKING_H
#define MULTITASKING_H

void init_multitasking(struct multiboot_info*);
struct cpu_state* handle_multitasking(struct cpu_state*);
struct cpu_state* schedule(struct cpu_state*);
struct task {
    struct cpu_state*   cpu_state;
    struct task*        next;
};

#endif