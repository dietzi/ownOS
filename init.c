/**
* @mainpage ownOS
*
* ownOS ist mein erstes Betriebssystem.<br>
*
* @author Martin Dietz
*/

/**
* @file init.c
*
* @brief Diese Datei ist der Einstiegspunkt für das Betriebssystem.
**/

#include "includes.h"

void init(struct multiboot_info *mb_info)
{
	init_complete=false;
	show_prefix=false;
	video_active=false;
	timer_ticks=0;
	init_console();
	clrscr();
	init_status();
	kprintf("Starting OS ...\n");
	kprintf("Initialization keyboard\n");
	keyboard_init();
	kprintf("Initialization mouse\n");
	mouse_init();
	kprintf("Initialization Timer\n");
	pit_init();
	kprintf("Initialization physical memory\n");
	pmm_init(mb_info);
	//kprintf("Initialization paging\n");
	//vmm_init();
	kprintf("Initialization GDT\n");
	init_gdt();
	kprintf("Initialization IDT\n");
	init_intr();
	kprintf("Initialization multitasking\n");
	init_multitasking(mb_info);
	//beep();
	kprintf("Initialization complete\n");
	kprintf("#######################\n");
	kprintf("# ");
	time();
	kprintf(" #\n");
	kprintf("#######################\n");
	
	start_nic();
	/*for(int bus=0;bus<256;bus++) {
		for(int j=0;j<32;j++) {
			for(int k=0;k<8;k++) {
				pci_bdf_t addr;
				addr.bus=bus;
				addr.dev=j;
				addr.func=k;
				pci_device dev=get_pci_device(addr);
				//if(dev.vendor_id!=0xffff && dev.vendor_id!=0x0) {
				//	class_to_text(get_pci_device(addr));
				//}
				if(dev.class_high==0x02 && dev.class_middle==0x0) {
					kprintf("%x\n",(uint8_t)(pci_config_readd(addr,0x3C)));
					pci_config_writed(addr,0x3C,0xE);
					kprintf("%x\n",(uint8_t)(pci_config_readd(addr,0x3C)));
					//pci_read_register(addr,0,0x3C);
				}
			}
		}
	}*/

	init_complete=true;
	show_prefix=true;
	kprintf("\n");
}
