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

void test_timer(uint32_t test_arg) {
	kprintf("Test...\n");
}

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
	pmm_init(mb_info);
	//kprintf("Initialization paging\n");
	//last_message="vmm_init";
	//vmm_init();
	kprintf("Initialization Timer\n");
	pit_init();
	kprintf("Initialization keyboard\n");
	keyboard_init();
	kprintf("Initialization mouse\n");
	mouse_install();
	kprintf("Initialization multitasking\n");
	init_multitasking(mb_info);
	kprintf("Initialization GDT\n");
	init_gdt();
	kprintf("Initialization IDT\n");
	init_intr();
	//set_vesa_mode(0x11b);
	kprintf("Initialization Network\n");
	init_network();
	kprintf("Initialization complete\n");
	kprintf("#######################\n");
	kprintf("# ");
	time();
	kprintf(" #\n");
	kprintf("#######################\n");

	init_complete=true;
	show_prefix=true;
	
	init_telnet();
	kprintf("\n");
	uint32_t *args = pmm_alloc();
	last_message = "register timer";
	register_timer(test_timer,1000,args);
	last_message = "timer registered";
	//get_pci_devices();
}
