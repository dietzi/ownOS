#include "includes.h"

#define IDT_ENTRIES 255

static uint64_t idt[IDT_ENTRIES];

struct {
	uint16_t limit;
	void* pointer;
} __attribute__((packed)) idtp = {
	.limit = IDT_ENTRIES * 8 - 1,
	.pointer = idt,
};

void init(void)
{
	clearscreen();
    initScreen;
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
	
	kprintf("Loading IDT");
	
	asm volatile("lidt %0" : : "m" (idtp));
	
	kprintf("Generating testing stuff");
	
	int i;
	
	for(i=0;i<30;i++) {
		char c1[0];
		c1[0]=(char)i;
		kprintf(c1);
	}
	
	kprintf("Raising interrupt");
    //asm volatile("sti");
    //asm volatile("int $0x0");
    while(1);
    stopCPU();
}

void int_handler(void)
{
    kprintf("Ein Interrupt!\n");
    while(1);
}