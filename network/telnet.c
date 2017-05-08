#include "includes.h"

void init_telnet(void);

uint8_t *buffer;
int buf_length = 0;

void check_telnet_command(void) {
	for(int i=0; i < buf_length; i++) {
		if(buffer[i] == '\r' && buffer[i+1] == '\n') {
			
		}
	}
}

void handle_telnet(struct tcp_callback cb) {
	/*kprintf("Telnet-Data: %d\n",cb.data_length);
	for(int i=0;i<cb.data_length;i++) {
		kprintf("%c\n",cb.data[i]);
	}*/
	if(cb.data_length == 3 && cb.data[0] == 0xff && cb.data[1] == 0xff && cb.data[2] == 0xff) {
		cb.data[0] = 0xff;
		cb.data[1] = 0xfd;
		cb.data[2] = 0x01;
		cb.data[3] = 0xff;
		cb.data[4] = 0xfb;
		cb.data[5] = 0x01;
		cb.data_length = 6;
	} else {
		for(int i=0;i<cb.data_length;i++) {
			buffer[buf_length] = cb.data[i];
			buf_length++;
		}
		check_telnet_command();
	}
	sendData(cb);
}

void init_telnet(void) {
	if(register_tcp_listener(23, handle_telnet)) {
		buffer = pmm_alloc();
		kprintf("Telnet registered\n");
	} else {
		kprintf("Telnet not registered\n");
	}
}