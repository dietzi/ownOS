#ifndef MM_H
#define MM_H

#include "multiboot.h"

#define NULL ((void*) 0)

void pmm_init(struct multiboot_info* mb_info);
void* pmm_alloc(void);
void pmm_free(void* page);

#define PTE_PRESENT 0x1
#define PTE_WRITE   0x2
#define PTE_USER    0x4

void vmm_init(void);
struct vmm_context* vmm_create_context(void);
struct vmm_context* vmm_create_context_user(void);
int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys);
int vmm_map_page_user(struct vmm_context* context, uintptr_t virt, uintptr_t phys);
void vmm_activate_context(struct vmm_context* context);
void* vmm_alloc(void);
void* vmm_alloc_context(struct vmm_context* context);

uint32_t last_addr;

#endif
