#ifndef INTR_H
#define INTR_H

#include <stdint.h>
#include "multiboot.h"
#include "modes.h"

/** @brief Struktur für den CPU-Zustand */
struct cpu_state {
    // Von Hand gesicherte Register
    uint32_t   eax;
    uint32_t   ebx;
    uint32_t   ecx;
    uint32_t   edx;
    uint32_t   esi;
    uint32_t   edi;
    uint32_t   ebp;

    uint32_t   intr;
    uint32_t   error;

    // Von der CPU gesichert
    uint32_t   eip;
    uint32_t   cs;
    uint32_t   eflags;
    uint32_t   esp;
    uint32_t   ss;
};

void outb(uint16_t port, uint8_t data);

void init_gdt(void);
void init_intr(void);
void init_multitasking(struct multiboot_info* mb_info);
void sleep(int ms);

struct cpu_state* handle_interrupt(struct cpu_state* cpu);
struct cpu_state* schedule(struct cpu_state* cpu);

char* last_message;

v86_t v86regs;

#endif