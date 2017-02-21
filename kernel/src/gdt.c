#include "includes.h"

#define GDT_FLAG_DATASEG 0x02
#define GDT_FLAG_CODESEG 0x0a
#define GDT_FLAG_TSS     0x09
 
#define GDT_FLAG_SEGMENT 0x10
#define GDT_FLAG_RING0   0x00
#define GDT_FLAG_RING3   0x60
#define GDT_FLAG_PRESENT 0x80
 
#define GDT_FLAG_4K_GRAN 0x800
#define GDT_FLAG_32_BIT  0x400

#define GDTENTRIES 6
// This structure contains the value of one GDT entry.
// We use the attribute 'packed' to tell GCC not to change
// any of the alignment in the structure.
struct gdt_entry_struct
{
    uint16_t limit_low;           // The lower 16 bits of the limit.
    uint16_t base_low;            // The lower 16 bits of the base.
    uint8_t  base_middle;         // The next 8 bits of the base.
    uint8_t  access;              // Access flags, determine what ring this segment can be used in.
    uint8_t  granularity;         // low 4 bits are high 4 bits of limit
    uint8_t  base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;

struct gdt_ptr_struct
{
    uint16_t limit;               // The upper 16 bits of all selector limits.
    uint32_t base;                // The address of the first gdt_entry_t struct.
}
    __attribute__((packed));
typedef struct gdt_ptr_struct gdt_ptr_t;

// Lets us access our ASM functions from our C code.
//extern void load_gdt(uint32_t);
extern void load_gdt(gdt_ptr_t * gdt_ptr);

// Internal function prototypes.
void gdt_set_gate(int32_t,uint32_t,uint32_t,uint8_t,uint8_t);

gdt_entry_t gdt_entries[GDTENTRIES];
gdt_ptr_t   gdt_ptr;
//idt_entry_t idt_entries[256];
//idt_ptr_t   idt_ptr;

static uint64_t gdt[GDTENTRIES];

void init_gdt()
{
    gdt_ptr.limit = (sizeof(gdt_entry_t)*GDTENTRIES) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    /*
      Pr  Priv  1   Ex  DC   RW   Ac
      0x9A == 1001 1010  == 1   00    1   1   0    1    0
      0x92 == 1001 0010  == 1   00    1   0   0    1    0
      0xFA == 1111 1010  == 1   11    1   1   0    1    0
      0xF2 == 1111 0010  == 1   11    1   0   0    1    0
      We have page-granularity and 32-bit mode
      G   D   0   Av
      0xCF == 1100 1111  == 1   1   0   0  ~
    */

    /*gdt_set_gate(0,0,0,0,0);                    //Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); //Code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); //Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); //User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); //User mode data segment

	set_entry(5, (uint32_t) tss, sizeof(tss), GDT_FLAG_TSS | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
		
    terminal_writestring("Flushing GDT");
    load_gdt(&gdt_ptr);*/
	
	set_entry(0, 0, 0, 0);
    set_entry(1, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);
    set_entry(2, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);
    set_entry(3, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
    set_entry(4, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
	set_entry(5, (uint32_t) tss, sizeof(tss), GDT_FLAG_TSS | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
		
	terminal_writestring("Flushing GDT");
	struct {
		uint16_t limit;
		void* pointer;
	} __attribute__((packed)) gdtp = {
		.limit = GDTENTRIES * 8 - 1,
		.pointer = gdt,
	};
	asm volatile("lgdt %0" : : "m" (gdtp));
	asm volatile("ltr %%ax" : : "a" (5 << 3));
}

void gdt_set_gate(int32_t entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt_entries[entry].base_low = (base & 0xFFFF);
    gdt_entries[entry].base_middle = (base >> 16) & 0xFF;
    gdt_entries[entry].base_high = (base >> 24) & 0xFF;

    gdt_entries[entry].limit_low = (limit & 0xFFFF);
    gdt_entries[entry].granularity = (limit >> 16) & 0x0F;

    gdt_entries[entry].granularity |= gran & 0xF0;
    gdt_entries[entry].access = access;
}

static void set_entry(int i, unsigned int base, unsigned int limit, int flags)
{
    gdt[i] = limit & 0xffffLL;
    gdt[i] |= (base & 0xffffffLL) << 16;
    gdt[i] |= (flags & 0xffLL) << 40;
    gdt[i] |= ((limit >> 16) & 0xfLL) << 48;
    gdt[i] |= ((flags >> 8 )& 0xffLL) << 52;
    gdt[i] |= ((base >> 24) & 0xffLL) << 56;
}