#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "idt.h"

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