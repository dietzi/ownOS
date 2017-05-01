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

void init(struct multiboot_info *mb_info) {
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
	kprintf("Initialization Network\n");
	start_nic();
	kprintf("Initialization complete\n");
	kprintf("#######################\n");
	kprintf("# ");
	time();
	kprintf(" #\n");
	kprintf("#######################\n");
	//beep();
	
	init_complete=true;
	show_prefix=true;
	kprintf("\n");
}
