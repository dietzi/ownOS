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

void testv86(void) {
	asm("int $0x10");
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
	//kprintf("Starting OS ...\n");
	//kprintf("Initialization keyboard\n");
	keyboard_init();
	//kprintf("Initialization mouse\n");
	mouse_install();
	//kprintf("Initialization Timer\n");
	pit_init();
	//kprintf("Initialization physical memory\n");
	pmm_init(mb_info);
	//kprintf("Initialization paging\n");
	//vmm_init();
	//kprintf("Initialization GDT\n");
	init_gdt();
	//kprintf("Initialization IDT\n");
	init_intr();
	//kprintf("Initialization multitasking\n");
	init_multitasking(mb_info);
	//set_vesa_mode(0x11b);
	//kprintf("Initialization Network\n");
	start_nic();
	//kprintf("Initialization complete\n");
	kprintf("#######################\n");
	kprintf("# ");
	time();
	kprintf(" #\n");
	kprintf("#######################\n");
	//beep();
//init_task(testv86,V86);
	init_complete=true;
	show_prefix=true;
	
	init_telnet();
	
	struct ip_addr ip = {
		.ip1 = 10,
		.ip2 = 0,
		.ip3 = 0,
		.ip4 = 100,
	};
	uint16_t port = HTONS(6356);
	uint32_t socketID = (ip.ip1) +
						(ip.ip2) +
						(ip.ip3) +
						(ip.ip4) +
						(HTONS(port)) +
						checksum(ip,4) +
						checksum(port,2);
	kprintf("Socket-ID: 0x%x\n",socketID);
	port = HTONS(54296);
	socketID = (ip.ip1) +
				(ip.ip2) +
				(ip.ip3) +
				(ip.ip4) +
				(HTONS(port)) +
				checksum(ip,4) +
				checksum(port,2);
	kprintf("Socket-ID: 0x%x\n",socketID);
	ip.ip1 = 100;
	ip.ip4 = 10;
	socketID = (ip.ip1) +
				(ip.ip2) +
				(ip.ip3) +
				(ip.ip4) +
				(HTONS(port)) +
				checksum(ip,4) +
				checksum(port,2);
	kprintf("Socket-ID: 0x%x\n",socketID);
	port = HTONS(6356);
	socketID = (ip.ip1) +
				(ip.ip2) +
				(ip.ip3) +
				(ip.ip4) +
				(HTONS(port)) +
				checksum(ip,4) +
				checksum(port,2);
	kprintf("Socket-ID: 0x%x\n",socketID);
	
	
	kprintf("\n");
}
