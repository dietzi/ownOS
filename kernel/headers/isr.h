#ifndef ISR_H
#define ISR_H

#include <stdint.h>

typedef struct registers
{
    uint32_t ds;                             // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;               // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss;   // Pushed by the processor automatically.
} registers_t;

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

// This intentionally accepts a *COPY* of the registers.
// It's slower, but it prevents service routines from messing with them.
// Maybe messing with them is useful, and we'll change this later.
typedef void (*isr_handler_t)(registers_t);
void register_interrupt_handler(uint8_t interrupt, isr_handler_t handler);

#define IRQ0 20
#define IRQ1 21
#define IRQ2 22
#define IRQ3 23
#define IRQ4 24
#define IRQ5 25
#define IRQ6 26
#define IRQ7 27
#define IRQ8 28
#define IRQ9 29
#define IRQ10 30
#define IRQ11 31
#define IRQ12 32
#define IRQ13 33
#define IRQ14 34
#define IRQ15 35

#endif