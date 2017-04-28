#include "includes.h"

/*
 * Dieser Speicherkontext wird nur waehrend der Initialisierung verwendet.
 * Spaeter laeuft der Kernel immer im Kontext des aktuellen Prozesses.
 */
struct vmm_context* kernel_context;


struct vmm_context* vmm_create_context(void)
{
    struct vmm_context* context = pmm_alloc();
    int i;

    /* Page Directory anlegen und mit Nullen initialisieren */
    context->pagedir = pmm_alloc();
    for (i = 0; i < 1024; i++) {
        context->pagedir[i] = 0;
    }

    return context;
}

struct vmm_context* vmm_create_context_user(void)
{
    struct vmm_context* context = pmm_alloc();
    int i;

    /* Page Directory anlegen und mit Nullen initialisieren */
    context->pagedir = pmm_alloc();
    for (i = 0; i < 1024; i++) {
        context->pagedir[i] = 0 | PTE_USER;
    }

    return context;
}

int vmm_map_page_user(struct vmm_context* context, uintptr_t virt, uintptr_t phys)
{
    uint32_t page_index = virt / 0x1000;
    uint32_t pd_index = page_index / 1024;
    uint32_t pt_index = page_index % 1024;

    uint32_t* page_table;
    int i;

    /* Wir brauchen 4k-Alignment */
    if ((virt & 0xFFF) || (phys & 0xFFF)) {
        return -1;
    }

    /* Page Table heraussuchen bzw. anlegen */
    if (context->pagedir[pd_index] & PTE_PRESENT & PTE_USER) {
        /* Page Table ist schon vorhanden */
        page_table = (uint32_t*) (context->pagedir[pd_index] & ~0xFFF);
    } else {
        /* Neue Page Table muss angelegt werden */
        page_table = pmm_alloc();
        for (i = 0; i < 1024; i++) {
            page_table[i] = 0;
        }
        context->pagedir[pd_index] =
            (uint32_t) page_table | PTE_PRESENT | PTE_WRITE | PTE_USER;
    }

    /* Neues Mapping in the Page Table eintragen */
    page_table[pt_index] = phys | PTE_PRESENT | PTE_WRITE | PTE_USER;
    asm volatile("invlpg %0" : : "m" (*(char*)virt));

    return 0;
}

int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys)
{
    uint32_t page_index = virt / 0x1000;
    uint32_t pd_index = page_index / 1024;
    uint32_t pt_index = page_index % 1024;

    uint32_t* page_table;
    int i;

    /* Wir brauchen 4k-Alignment */
    if ((virt & 0xFFF) || (phys & 0xFFF)) {
        return -1;
    }

    /* Page Table heraussuchen bzw. anlegen */
    if (context->pagedir[pd_index] & PTE_PRESENT) {
        /* Page Table ist schon vorhanden */
        page_table = (uint32_t*) (context->pagedir[pd_index] & ~0xFFF);
    } else {
        /* Neue Page Table muss angelegt werden */
        page_table = pmm_alloc();
        for (i = 0; i < 1024; i++) {
            page_table[i] = 0;
        }
        context->pagedir[pd_index] =
            (uint32_t) page_table | PTE_PRESENT | PTE_WRITE | PTE_USER;
    }

    /* Neues Mapping in the Page Table eintragen */
    page_table[pt_index] = phys | PTE_PRESENT | PTE_WRITE | PTE_USER;
    asm volatile("invlpg %0" : : "m" (*(char*)virt));

    return 0;
}

void vmm_activate_context(struct vmm_context* context)
{
    asm volatile("mov %0, %%cr3" : : "r" (context->pagedir));
}

void vmm_init(void)
{
    uint32_t cr0;
    int i;

    /* Speicherkontext anlegen */
    kernel_context = vmm_create_context();
	
	last_addr=0;
	
    /* Die ersten 4 MB an dieselbe physische wie virtuelle Adresse mappen */
    for (; last_addr < 4096 * 1024; last_addr += 0x1000) {
        vmm_map_page(kernel_context, last_addr, last_addr);
    }
	last_addr += 0x1000;
	
	for (i = 0xB8000; i < 0xC0000; i += 0x1000) {
        vmm_map_page(kernel_context, i, i);
    }
	
    vmm_activate_context(kernel_context);

    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= (1 << 31);
    asm volatile("mov %0, %%cr0" : : "r" (cr0));
}