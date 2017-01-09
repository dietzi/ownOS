#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "idt.h"

/* Declare an IDT of 256 entries. Although we will only use the
*  first 32 entries in this tutorial, the rest exists as a bit
*  of a trap. If any undefined IDT entry is hit, it normally
*  will cause an "Unhandled Interrupt" exception. Any descriptor
*  for which the 'presence' bit is cleared (0) will generate an
*  "Unhandled Interrupt" exception */
struct idt_entry idt[256];
struct idt_ptr _idtp;

/* Use this function to set an entry in the IDT. Alot simpler
*  than twiddling with the GDT ;) */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags){
    /* We'll leave you to try and code this function: take the
    *  argument 'base' and split it up into a high and low 16-bits,
    *  storing them in idt[num].base_hi and base_lo. The rest of the
    *  fields that you must set in idt[num] are fairly self-
    *  explanatory when it comes to setup */
    idt[num].base_lo = base & 0xFFFF;
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].always0 = 0;
    idt[num].sel = sel;
    idt[num].flags = flags;
}

/* Installs the IDT */
void idt_install(){
    kprintf("Installing IDT\n",0,0);
    /* Sets the special IDT pointer up, just like in 'gdt.c' */
    _idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
    _idtp.base = (uint32_t) &idt;

    /* Clear out the entire IDT, initializing it to zeros */
    memset(&idt, 0, sizeof(struct idt_entry) * 256);
    
    uint32_t foo = idt[0].base_lo;
    if (!foo){
        kprintf("IDT Zeroed correctly\n",0,0);
    }
    
    /* Add any new ISRs to the IDT here using idt_set_gate */

    /* Points the processor's internal register to the new IDT */
    //_idt_load();
}