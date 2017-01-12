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
    }

    if(interrupt_handlers[regs.int_no])
    {
        terminal_writestring("Handling!");
        interrupt_handlers[regs.int_no](regs);
    }
}


void irq_handler(registers_t regs)
{
	if(regs.int_no != 32) {
		terminal_writestring("IRQ");
		char *result;
		itoa(regs.int_no,*result,10);
		terminal_writestring(*result);
	}
	if(regs.int_no == 21 || regs.int_no == 32) {
		terminal_writestring("IRQ Keyboard");
		kbd_irq_handler();
	} else {
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
}