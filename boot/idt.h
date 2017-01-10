#ifndef IDT_H
#define IDT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
void handle_interrupt(struct cpu_state* cpu)
{
    if (cpu->intr <= 0x1f) {
        terminal_writestring("Exception!");
		//terminal_writestring((char*)cpu->intr);
 
        // Hier den CPU-Zustand ausgeben
 
        while(1) {
            // Prozessor anhalten
            stopCPU();
        }
    } else {
        // Hier den Hardwareinterrupt behandeln
		if (cpu->intr >= 0x20 && cpu->intr <= 0x2f) {
			if (cpu->intr >= 0x28) {
				outb(0xa0, 0x20);
			}
			outb(0x20, 0x20);
		}
		terminal_writestring("Interrupt!");
    }
}
#endif