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
	screen.x = 80;
	screen.y = 25;
	show_prefix=false;
	video_active=false;
	timer_ticks=0;
	init_console();
	clrscr();
	init_status();
	kprintf("Starting OS ...\n");
	kprintf("Initialization physical memory\n");
	last_message="pmm_init";
	pmm_init(mb_info);
	//kprintf("Initialization paging\n");
	//last_message="vmm_init";
	//vmm_init();
	kprintf("Initialization keyboard\n");
	last_message="keyboard_init";
	keyboard_init();
	kprintf("Initialization mouse\n");
	last_message="mouse_install";
	mouse_install();
	kprintf("Initialization Timer\n");
	last_message="pit_init";
	pit_init();
	kprintf("Initialization GDT\n");
	last_message="init_gdt";
	init_gdt();
	kprintf("Initialization IDT\n");
	last_message="init_intr";
	init_intr();
	asm volatile("sti");
	kprintf("Initialization multitasking\n");
	last_message="init_multitasking";
	init_multitasking(mb_info);
	last_message="ready";
	//set_vesa_mode(0x11b);
	//kprintf("Initialization Network\n");
	//start_nic();
	kprintf("Initialization complete\n");
	kprintf("#######################\n");
	kprintf("# ");
	time();
	kprintf(" #\n");
	kprintf("#######################\n");
	//beep();
//init_task(testv86,V86);
	init_complete=true;
	show_prefix=true;
	
	//init_telnet();
	
	kprintf("\n");
}
