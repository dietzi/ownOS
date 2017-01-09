#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "idt.h"
#include "console.h"

void handle_interrupt(struct cpu_state* cpu)
{
    if (cpu->intr <= 0x1f) {
        kprintf("Exception:");
		kprintf((char*)cpu->intr);
 
        // Hier den CPU-Zustand ausgeben
 
        while(1) {
            // Prozessor anhalten
            asm volatile("cli; hlt");
        }
    } else {
        // Hier den Hardwareinterrupt behandeln
    }
}