#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "idt.h"

void handle_interrupt(struct cpu_state* cpu)
{
    if (cpu->intr <= 0x1f) {
        terminal_writestring("Exception:");
		terminal_writestring((char*)cpu->intr);
 
        // Hier den CPU-Zustand ausgeben
 
        while(1) {
            // Prozessor anhalten
            asm volatile("cli; hlt");
        }
    } else {
        // Hier den Hardwareinterrupt behandeln
    }
}