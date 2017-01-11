#include "includes.h"

void init(void)
{
	terminal_initialize();
    init_gdt();
    // Master-PIC initialisieren
    outb(0x20, 0x11); // Initialisierungsbefehl fuer den PIC
    outb(0x21, 0x20); // Interruptnummer fuer IRQ 0
    outb(0x21, 0x04); // An IRQ 2 haengt der Slave
    outb(0x21, 0x01); // ICW 4
    
    // Slave-PIC initialisieren
    outb(0xa0, 0x11); // Initialisierungsbefehl fuer den PIC
    outb(0xa1, 0x28); // Interruptnummer fuer IRQ 8
    outb(0xa1, 0x02); // An IRQ 2 haengt der Slave
    outb(0xa1, 0x01); // ICW 4
    
    // Alle IRQs aktivieren (demaskieren)
    outb(0x21, 0x0);
    outb(0xa1, 0x0);
	
	init_idt();
	
	//terminal_writestring("Loading IDT");
	
	//asm volatile("lidt %0" : : "m" (idtp));
	
	//terminal_writestring("IDT loaded");
	//terminal_writestring("IDT's:");
	//for(int i=0;i<10;i++) terminal_writestring(idt[i]);

	terminal_writestring("Activating interrupts");
    asm volatile("sti");
    //asm volatile("int $0x0");
    while(1);
    stopCPU();
}

void int_handler(void)
{
    terminal_writestring("Ein Interrupt!\n");
    while(1);
}