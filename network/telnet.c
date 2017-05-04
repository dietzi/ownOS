#include "includes.h"

void init_telnet(void);

void handle_telnet(struct tcp_callback cb) {
	kprintf("Telnet-Data:\n");
	for(int i=0;i<cb.data_length;i++) {
		kprintf("%c\n",cb.data[i]);
	}
}

void init_telnet(void) {
	struct tcp_callback cb = {
		.callback_pointer = &handle_telnet,
		.port = 23,
		.con_est = false,
		.enabled = true,
	};
	register_tcp_listener(cb);
}