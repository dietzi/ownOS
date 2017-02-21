#include "includes.h"

uint32_t tss[32] = { 0, 0, 0x10 };

isr_handler_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t interrupt, isr_handler_t handler)
{
    interrupt_handlers[interrupt] = handler;
}

struct cpu_state* syscall(struct cpu_state* cpu)
{
    switch (cpu->eax) {
        case 0: /* time() */
            time();
            break;
    }
}

struct cpu_state* isr_handler(struct cpu_state* cpu)
{
	terminal_writestring("ISR");
	char *intnr;
	char *errornr;
	itoa2(cpu->intr,intnr,10);
	itoa2(cpu->error,errornr,10);
	terminal_writestring(intnr);
	terminal_writestring(errornr);
	if(cpu->intr == GENERAL_PROTECTION_FAULT)
    {
        terminal_writestring("General Protection Fault!");
		terminal_writestring('0' + (int)cpu->error);
    }
	if(cpu->intr == BAD_TSS)
    {
        terminal_writestring("BAD TSS");
		terminal_writestring('0' + (int)cpu->error);
    }
	
	if(cpu->intr == SYSCALL) {
		terminal_writestring("SYS");
		cpu=syscall(cpu);
	}
	if(cpu->intr == 16272832) {
		terminal_writestring("SYSCALL");
		cpu=syscall(cpu);
	}
	
	return cpu;
    /*if(interrupt_handlers[regs.int_no])
    {
        terminal_writestring("Handling ISR!");
        interrupt_handlers[regs.int_no](regs);
    }*/
}

struct cpu_state* handle_interrupt(struct cpu_state* cpu)
{
    struct cpu_state* new_cpu = cpu;

	if(cpu->intr != 0x20 && cpu->intr != 0x21) {
		terminal_writestring("IRQ");
		terminal_writestring('0' + cpu->intr);
	}
    if (cpu->intr == 0x20) {
		// terminal_writestring("Handling IRQ 0");
        if(MULTITASKING) new_cpu=schedule(cpu); //handle_multitasking(cpu);
		if(MULTITASKING) tss[1] = (uint32_t) (new_cpu + 1);
    }
	if(cpu->intr == 0x21) {
		terminal_writestring("Keyboard");
		if(KEYBOARD) kbd_irq_handler();
	}
	
	if (cpu->intr >= 0x20 && cpu->intr <= 0x2f) {
		// terminal_writestring("Resetting Master");
		if (cpu->intr >= 0x28) {
			// terminal_writestring("Resetting Slave");
			outb(SLAVE_COMMAND, PIC_RESET);
			// terminal_writestring("Slave resetted");
		}
		outb(MASTER_COMMAND, PIC_RESET);
		// terminal_writestring("Master resetted");
	}
	// terminal_writestring("Returning CPU-State");
	return new_cpu;
}

void noti(void) {
	terminal_writestring("Ruecksprung");
}