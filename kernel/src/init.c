#include "includes.h"

void init(struct multiboot_info *mb_info)
{
	terminal_initialize();
	if(MULTITASKING) init_pmm(mb_info);
	 
    init_gdt();
	init_idt();
	
	terminal_writestring("Loaded IDT and GDT");
	if(DEBUG) asm volatile("xchg %bx, %bx");
	if(KEYBOARD) terminal_writestring("Initializing keyboard");
	if(KEYBOARD) keyboard_init();
	if(MULTITASKING) terminal_writestring("Initializing multitasking");
	if(MULTITASKING) init_multitasking(mb_info);
	terminal_writestring("Activating interrupts");
	
	if(DEBUG) asm volatile("xchg %bx, %bx");
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

	terminal_writestring(" ");
	terminal_writestring("#####################");
	terminal_writestring("#   ownOS started   #");
	time();
	terminal_writestring("#####################");
	
	/*if(DEBUG)*/ asm volatile("xchg %bx, %bx");
    asm volatile("sti");
	terminal_writestring("Interrupts activated");
	if(DEBUG) asm volatile("xchg %bx, %bx");
    while(1);
    stopCPU();
}