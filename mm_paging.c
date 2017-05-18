#include "includes.h"

#define PAGE_SIZE 0x1000

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
    uint32_t page_index = virt / PAGE_SIZE;
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
	last_message="doing asm";
    asm volatile("invlpg %0" : : "m" (*(char*)virt));
	last_message="asm end";
    return 0;
}

int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys)
{
    uint32_t page_index = virt / PAGE_SIZE;
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
            (uint32_t) page_table | PTE_PRESENT | PTE_WRITE;
    }

    /* Neues Mapping in the Page Table eintragen */
    page_table[pt_index] = phys | PTE_PRESENT | PTE_WRITE;
    asm volatile("invlpg %0" : : "m" (*(char*)virt));

    return 0;
}

void vmm_activate_context(struct vmm_context* context)
{
    asm volatile("mov %0, %%cr3" : : "r" (context->pagedir));
}

extern struct task* current_task;


void* vmm_alloc(void) {
	uint32_t page_index = 4096 / PAGE_SIZE;
    uint32_t pd_index = page_index / 1024;
    uint32_t pt_index = page_index % 1024;
	uint32_t* page_table;
	
	struct vmm_context* alloc_context = vmm_create_context();
	
    for (int i=0; i < 4096; i += PAGE_SIZE) {
        vmm_map_page(alloc_context, i, last_addr);
		last_addr+=PAGE_SIZE;
    }
	//vmm_activate_context(alloc_context);
	kprintf("Addr: 0x%x\n",alloc_context->pagedir[0]);
	sleep(2000);
	return alloc_context->pagedir[0];
}

void vmm_init(void)
{
    uint32_t cr0;
    int i;

    /* Speicherkontext anlegen */
    kernel_context = vmm_create_context();
	
	last_addr=0;
	
    /* Die ersten 4 MB an dieselbe physische wie virtuelle Adresse mappen */
    for (; last_addr < /* 4096 */ 8192 * 1024; last_addr += PAGE_SIZE) {
        vmm_map_page(kernel_context, last_addr, last_addr);
    }
	last_addr += PAGE_SIZE;
	
	for (i = 0xB8000; i < 0xC0000; i += PAGE_SIZE) {
        vmm_map_page(kernel_context, i, i);
    }
	
    vmm_activate_context(kernel_context);

    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= (1 << 31);
    asm volatile("mov %0, %%cr0" : : "r" (cr0));
}
