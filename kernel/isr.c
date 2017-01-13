#include "includes.h"

isr_handler_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t interrupt, isr_handler_t handler)
{
    interrupt_handlers[interrupt] = handler;
}

void isr_handler(registers_t regs)
{
	terminal_writestring("ISR");
    char *result;
	itoa(regs.int_no,*result,10);
	terminal_writestring(*result);
	if(regs.int_no == GENERAL_PROTECTION_FAULT)
    {
        //printf("General Protection Fault. Code: %d", regs.err_code);
        terminal_writestring("General Protection Fault!");
		char *result;
		itoa(regs.err_code,*result,10);
		terminal_writestring(*result);
    }

    if(interrupt_handlers[regs.int_no])
    {
        terminal_writestring("Handling!");
        interrupt_handlers[regs.int_no](regs);
    }
}

struct cpu_state* handle_interrupt(struct cpu_state* cpu)
{
	terminal_writestring("Handling interrupt");
    struct cpu_state* new_cpu = cpu;

	//if(cpu->intr != 0x32 && cpu->intr != 0x33) {
		terminal_writestring("IRQ");
		char *result;
		itoa(cpu->intr,*result,10);
		terminal_writestring(*result);
	///}
    /*if (cpu->intr == 0x32) {
        new_cpu=handle_multitasking(cpu);
    }*/
	if(cpu->intr == 0x33) {
		kbd_irq_handler();
	}

	if (cpu->intr >= 0x20 && cpu->intr <= 0x2f) {
		if (cpu->intr >= 0x28) {
			outb(0xa0, 0x20);
		}
		outb(0x20, 0x20);
	}	
    return new_cpu;
}

void irq_handler(registers_t regs)
{
	if(regs.int_no != IRQ0 && regs.int_no != IRQ1) {
		terminal_writestring("IRQ");
		char *result;
		itoa(regs.int_no,*result,10);
		terminal_writestring(*result);
	}
	if(regs.int_no == IRQ0) {
		struct cpu_state * cpu=(struct cpu_state *)regs;

		handle_multitasking(cpu);
	}
	if(regs.int_no == IRQ1) {
		kbd_irq_handler();
	}
	//If int_no >= 40, we must reset the slave as well as the master
	if(regs.int_no >= 40)
	{
		//reset slave
		outb(SLAVE_COMMAND, PIC_RESET);
	}

	outb(MASTER_COMMAND, PIC_RESET);

	if(interrupt_handlers[regs.int_no])
	{
		interrupt_handlers[regs.int_no](regs);
	}
}