#include "includes.h"

#define IDT_FLAG_DATASEG 0x02
#define IDT_FLAG_CODESEG 0x0a
#define IDT_FLAG_TSS     0x09
#define IDT_FLAG_SEGMENT 0x10
#define IDT_FLAG_RING0   0x00
#define IDT_FLAG_RING3   0x60
#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_4K_GRAN 0x800
#define IDT_FLAG_32_BIT  0x400
#define IDT_ENTRIES 4

#define IDT_ENTRIES 255

static uint64_t idt[IDT_ENTRIES];

struct {
	uint16_t limit;
	void* pointer;
} __attribute__((packed)) idtp = {
	.limit = IDT_ENTRIES * 8 - 1,
	.pointer = idt,
};

static void set_idt_entry(int i, unsigned int base, unsigned int limit, int flags)
{
    idt[i] = limit & 0xffffLL;
    idt[i] |= (base & 0xffffffLL) << 16;
    idt[i] |= (flags & 0xffLL) << 40;
    idt[i] |= ((limit >> 16) & 0xfLL) << 48;
    idt[i] |= ((flags >> 8 )& 0xffLL) << 52;
    idt[i] |= ((base >> 24) & 0xffLL) << 56;
}

void load_idt()
{
	asm volatile("lidt %0" : : "m" (idtp));
	asm volatile("mov $0x10, %%ax;mov %%ax, %%ds;mov %%ax, %%es;mov %%ax, %%fs;mov %%ax, %%gs;mov %%ax, %%ss;" : : );
	asm volatile("ljmp $0x8, $.1;.1:;" : : );
	terminal_writestring("IDT loaded");
}

void init_idt(void)
{
    terminal_writestring("initializing IDT");
    set_idt_entry(0, 0, 0, 0);
    set_idt_entry(1, 0, 0xfffff, IDT_FLAG_SEGMENT | IDT_FLAG_32_BIT |
        IDT_FLAG_CODESEG | IDT_FLAG_4K_GRAN | IDT_FLAG_PRESENT);
    set_idt_entry(2, 0, 0xfffff, IDT_FLAG_SEGMENT | IDT_FLAG_32_BIT |
        IDT_FLAG_DATASEG | IDT_FLAG_4K_GRAN | IDT_FLAG_PRESENT);
    set_idt_entry(3, 0, 0xfffff, IDT_FLAG_SEGMENT | IDT_FLAG_32_BIT |
        IDT_FLAG_CODESEG | IDT_FLAG_4K_GRAN | IDT_FLAG_PRESENT | IDT_FLAG_RING3);
    set_idt_entry(4, 0, 0xfffff, IDT_FLAG_SEGMENT | IDT_FLAG_32_BIT |
        IDT_FLAG_DATASEG | IDT_FLAG_4K_GRAN | IDT_FLAG_PRESENT | IDT_FLAG_RING3);
    terminal_writestring("IDT initialized");
	terminal_writestring("Loading IDT");
    load_idt();
}

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