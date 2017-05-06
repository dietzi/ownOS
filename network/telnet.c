#include "includes.h"

void init_telnet(void);

void handle_telnet(struct tcp_callback cb) {
	kprintf("Telnet-Data: %d\n",cb.data_length);
	sleep(5000);
	//for(int i=0;i<cb.data_length;i++) {
//		kprintf("%c\n",cb.data[i]);
//	}
}

void init_telnet(void) {
	if(register_tcp_listener(23, handle_telnet)) {
		kprintf("Telnet registered\n");
	} else {
		kprintf("Telnet not registered\n");
	}
}