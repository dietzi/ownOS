#ifndef IDT_H
#define IDT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mem.h"

/* Defines an IDT entry */
struct idt_entry{
    uint16_t base_lo;
    uint16_t sel;        /* Our kernel segment goes here! */
    uint8_t always0;     /* This will ALWAYS be set to 0! */
    uint8_t flags;       /* Set using the above table! */
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* This exists in 'start.asm', and is used to load our IDT */
extern void _idt_load();

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

void idt_install();

#endif