#include "includes.h"

void init(void)
{
    clearscreen();
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
    
    idt_install();
    
    kprintf("Raising interrupt",3,0);
    asm volatile("sti");
    //asm volatile("int $0x0");
    while(1);
    stopCPU();
}

void int_handler(void)
{
    kprintf("Ein Interrupt!\n",4,0);
    while(1);
}

void handle_interrupt(struct cpu_state* cpu)
{
    if (cpu->intr <= 0x1f) {
        kprintf("Exception ",5,0);
        kprintf((char*)cpu->intr,5,11);
        kprintf("Kernel angehalten!\n", 6,0);
        
        // Hier den CPU-Zustand ausgeben
 
        while(1) {
            // Prozessor anhalten
            asm volatile("cli; hlt");
        }
    } else {
      if (cpu->intr >= 0x20 && cpu->intr <= 0x2f) {
        if (cpu->intr >= 0x28) {
            outb(0xa0, 0x20);
        }
        outb(0x20, 0x20);
      }
    }
}