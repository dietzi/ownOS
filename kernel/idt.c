#include "includes.h"

struct idt_entry_struct
{
    uint16_t base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
    uint16_t sel;                 // Kernel segment selector.
    uint8_t  always0;             // This must always be zero.
    uint8_t  flags;               // More flags. See documentation.
    uint16_t base_hi;             // The upper 16 bits of the address to jump to.
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_struct
{
    uint16_t limit;
    uint32_t base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

void *memset(void *p, int c, size_t count)
{
    for(unsigned int i = 0; i < count; i++)
    {
        ((char *)p)[i] = c;
    }
    return p;
}

// These extern directives let us access the addresses of our ASM ISR handlers.
extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();
extern void isr33();
extern void isr34();
extern void isr35();
extern void isr36();
extern void isr37();
extern void isr38();
extern void isr39();
extern void isr40();
extern void isr41();
extern void isr42();
extern void isr43();
extern void isr44();
extern void isr45();
extern void isr46();
extern void isr47();
extern void isr48();
extern void isr49();
extern void isr50();
extern void isr51();
extern void isr52();
extern void isr53();
extern void isr54();
extern void isr55();
extern void isr56();
extern void isr57();
extern void isr58();
extern void isr59();
extern void isr60();
extern void isr61();
extern void isr62();
extern void isr63();
extern void isr64();
extern void isr65();
extern void isr66();
extern void isr67();
extern void isr68();
extern void isr69();
extern void isr70();
extern void isr71();
extern void isr72();
extern void isr73();
extern void isr74();
extern void isr75();
extern void isr76();
extern void isr77();
extern void isr78();
extern void isr79();
extern void isr80();
extern void isr81();
extern void isr82();
extern void isr83();
extern void isr84();
extern void isr85();
extern void isr86();
extern void isr87();
extern void isr88();
extern void isr89();
extern void isr90();
extern void isr91();
extern void isr92();
extern void isr93();
extern void isr94();
extern void isr95();
extern void isr96();
extern void isr97();
extern void isr98();
extern void isr99();
extern void isr100();


// IRQ Handlers
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

extern void load_idt(idt_ptr_t *);
static void idt_set_gate(uint8_t,uint32_t,uint16_t,uint8_t);

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;


void init_idt()
{
    idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entry_t)*256);

    idt_set_gate( 0, (uint32_t)isr0 , 0x08, 0x8E);
    idt_set_gate( 1, (uint32_t)isr1 , 0x08, 0x8E);
    idt_set_gate( 2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate( 3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate( 4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate( 5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate( 6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate( 7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate( 8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate( 9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
    idt_set_gate(32, (uint32_t)isr32, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)isr33, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)isr34, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)isr35, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)isr36, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)isr37, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)isr38, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)isr39, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)isr40, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)isr41, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)isr42, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)isr43, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)isr44, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)isr45, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)isr46, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)isr47, 0x08, 0x8E);
    idt_set_gate(48, (uint32_t)isr48, 0x08, 0x8E);
    idt_set_gate(49, (uint32_t)isr49, 0x08, 0x8E);
    idt_set_gate(50, (uint32_t)isr50, 0x08, 0x8E);
    idt_set_gate(51, (uint32_t)isr51, 0x08, 0x8E);
    idt_set_gate(52, (uint32_t)isr52, 0x08, 0x8E);
    idt_set_gate(53, (uint32_t)isr53, 0x08, 0x8E);
    idt_set_gate(54, (uint32_t)isr54, 0x08, 0x8E);
    idt_set_gate(55, (uint32_t)isr55, 0x08, 0x8E);
    idt_set_gate(56, (uint32_t)isr56, 0x08, 0x8E);
    idt_set_gate(57, (uint32_t)isr57, 0x08, 0x8E);
    idt_set_gate(58, (uint32_t)isr58, 0x08, 0x8E);
    idt_set_gate(59, (uint32_t)isr59, 0x08, 0x8E);
    idt_set_gate(60, (uint32_t)isr60, 0x08, 0x8E);
    idt_set_gate(61, (uint32_t)isr61, 0x08, 0x8E);
    idt_set_gate(62, (uint32_t)isr62, 0x08, 0x8E);
    idt_set_gate(63, (uint32_t)isr63, 0x08, 0x8E);
    idt_set_gate(64, (uint32_t)isr64, 0x08, 0x8E);
    idt_set_gate(65, (uint32_t)isr65, 0x08, 0x8E);
    idt_set_gate(66, (uint32_t)isr66, 0x08, 0x8E);
    idt_set_gate(67, (uint32_t)isr67, 0x08, 0x8E);
    idt_set_gate(68, (uint32_t)isr68, 0x08, 0x8E);
    idt_set_gate(69, (uint32_t)isr69, 0x08, 0x8E);
    idt_set_gate(70, (uint32_t)isr70, 0x08, 0x8E);
    idt_set_gate(71, (uint32_t)isr71, 0x08, 0x8E);
    idt_set_gate(72, (uint32_t)isr72, 0x08, 0x8E);
    idt_set_gate(73, (uint32_t)isr73, 0x08, 0x8E);
    idt_set_gate(74, (uint32_t)isr74, 0x08, 0x8E);
    idt_set_gate(75, (uint32_t)isr75, 0x08, 0x8E);
    idt_set_gate(76, (uint32_t)isr76, 0x08, 0x8E);
    idt_set_gate(77, (uint32_t)isr77, 0x08, 0x8E);
    idt_set_gate(78, (uint32_t)isr78, 0x08, 0x8E);
    idt_set_gate(79, (uint32_t)isr79, 0x08, 0x8E);
    idt_set_gate(80, (uint32_t)isr80, 0x08, 0x8E);
    idt_set_gate(81, (uint32_t)isr81, 0x08, 0x8E);
    idt_set_gate(82, (uint32_t)isr82, 0x08, 0x8E);
    idt_set_gate(83, (uint32_t)isr83, 0x08, 0x8E);
    idt_set_gate(84, (uint32_t)isr84, 0x08, 0x8E);
    idt_set_gate(85, (uint32_t)isr85, 0x08, 0x8E);
    idt_set_gate(86, (uint32_t)isr86, 0x08, 0x8E);
    idt_set_gate(87, (uint32_t)isr87, 0x08, 0x8E);
    idt_set_gate(88, (uint32_t)isr88, 0x08, 0x8E);
    idt_set_gate(89, (uint32_t)isr89, 0x08, 0x8E);
    idt_set_gate(90, (uint32_t)isr90, 0x08, 0x8E);
    idt_set_gate(91, (uint32_t)isr91, 0x08, 0x8E);
    idt_set_gate(92, (uint32_t)isr92, 0x08, 0x8E);
    idt_set_gate(93, (uint32_t)isr93, 0x08, 0x8E);
    idt_set_gate(94, (uint32_t)isr94, 0x08, 0x8E);
    idt_set_gate(95, (uint32_t)isr95, 0x08, 0x8E);
    idt_set_gate(96, (uint32_t)isr96, 0x08, 0x8E);
    idt_set_gate(97, (uint32_t)isr97, 0x08, 0x8E);
    idt_set_gate(98, (uint32_t)isr98, 0x08, 0x8E);
    idt_set_gate(99, (uint32_t)isr99, 0x08, 0x8E);
    idt_set_gate(100, (uint32_t)isr100, 0x08, 0x8E);


    // IRQ entries
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    terminal_writestring("Flushing IDT");
    load_idt(&idt_ptr);
}

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    // We must uncomment the OR below when we get to using user-mode.
    // It sets the interrupt gate's privilege level to 3.
    idt_entries[num].flags   = flags /* | 0x60 */;
}