#include "includes.h"

#define SIZE_OF_MAP 32768

extern const void kernel_start;
extern const void kernel_end;

uint32_t MemoryMap[SIZE_OF_MAP]; //----------Erzeugt eine MemoryMap----------


void init_pmm(struct multiboot_info* mbinfo) {

        struct multiboot_mmap* mmap = (void*)mbinfo->mi_mmap_addr;  //----------Erzeugt die MemoryMap von GRUB----------
        struct multiboot_mmap* mmap_end = (void*) ((uintptr_t)mbinfo->mi_mmap_addr + mbinfo->mi_mmap_length);  //----------Die Addresse des Ende der Map ist die Start-Addresse plus der L?nge der MemoryMap----------
        kmemset(MemoryMap, 0, sizeof(MemoryMap));       //----------Belege die MemoryMap mit 0, damit alle Bl?cke als belegt makiert sind----------

        //----------Alle als frei angezeigten Bl?cke in der MemoryMap von GRUB werden freigegeben----------
        while(mmap < mmap_end)
        {
                if(mmap->mm_type == 1)
                {
                uintptr_t m_address = mmap->mm_base_addr;
                uintptr_t m_end = m_address + mmap->mm_length;
                        while(m_address < m_end)
                        {
                                pmm_free((void*)m_address);
                                m_address += 0x1000;
                        }
                }
                mmap++;
        }
        
        //----------Sperre die Speicherbl?cke, in denen der Kernel liegt----------
        
        uintptr_t kstart = (uintptr_t)&kernel_start;    //----------kernel_start ist die Start-Addresse des Kernels, deklariert im Linker-Script----------
        while(kstart < (uintptr_t) &kernel_end)         //----------kernel_end ist die End-Addresse des Kernels, deklariert im Linker-Script----------
        {
                pmm_mark_used((void*)kstart);           //----------Sperrt die angegebene Addresse----------
                kstart += 0x1000;
        }
        
        //----------Die Strukturen werden auch als belegt makiert----------
        struct multiboot_module* grub_modules = (void*)mbinfo->mi_mods_addr;
        pmm_mark_used(grub_modules);
        pmm_mark_used(mbinfo);
        
        //----------Die Module werden als belegt makiert, immerhin sollen diese nicht ?berschrieben werden----------
        for(int i = 0; i < mbinfo->mi_mods_count; i++)
        {
                uintptr_t addr = grub_modules[i].start;
                while(addr < grub_modules[i].end)
                {
                        pmm_mark_used((void*)addr);
                        addr += 0x1000;
                }
        }
        return;
}


void* pmm_alloc() {
        for(int i = 0; i < SIZE_OF_MAP; i++) {  //----------F?hre das alles pro Block aus----------
                if(MemoryMap[i] != 0)           //----------Wenn dieser Block nicht 0 ist, also irgendein Bit gesetzt ist, pr?fe weiter----------
                {
                        for(int k = 0; k < 32; k++)     //----------F?hre dies f?r alle 32 Bits aus----------
                        {
                                if(MemoryMap[i] & (1 << k))     //----------Finde das gesetzte Bit----------
                                {
                                        MemoryMap[i] &= ~(1 << k);      //----------L?sche das gesetzte Bit(Block als belegt markieren) und gebe die aktuelle Addresse des Speicherblocks zur?ck----------
                                        return (void*) ((i * 32 + k) * 4096);
                                }
                        }
                }
        }
        //kernel_panic(1,0);      //----------Wenn alle Bl?cke belegt sind, f?hre die Kernel-Panic Nummer 1 aus----------
        return 0;
}


void pmm_free(void* pmm_page) {
        uintptr_t i = (uintptr_t)pmm_page / 0x1000;
        MemoryMap[i / 32] |= (1 << (i % 32));   //----------Setze das Bit zum freigeben des angegebenen Blocks----------
}


void pmm_mark_used(void* pmm_page) {
        uintptr_t i = (uintptr_t)pmm_page / 0x1000;
        MemoryMap[i / 32] &= ~(1 << (i % 32));  //----------L?sche das Bit zum sperren des angegebenen Blocks----------
}