#ifndef PMM_H
#define PMM_H

void init_pmm(struct multiboot_info*);
void* pmm_alloc();
void pmm_free(void*);
void pmm_mark_used(void*);

#endif