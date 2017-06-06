#include "includes.h"

void init_telnet(void);

uint8_t *buffer;
int buf_length = 0;

bool check(char *cmd, char *input) {
	if(strlen(input)+1!=strlen(cmd)+2) return false;
	for(int i=0;i<strlen(cmd)+2;i++) {
		if(cmd[i] != input[i+1]) return false;
	}
	return true;
}

void checker(uint8_t *cmd,struct tcp_callback cb) {
	if(check("exit",cmd)) closeCon(cb);
	if(check("hello",cmd)) kprintf("Funktioniert\n");
}

void check_telnet_command(struct tcp_callback cb) {
	int last_i = 0;
begin:
	for(int i=0; i < buf_length; i++) {
		if(buffer[i] == '\r' && buffer[i+1] == '\n') {
			if(i>0) {
				uint8_t *cmd = pmm_alloc();
				memset(cmd,'\0',200);
				int counter = 0;
				for(int j=last_i;j<i;j++) {
					cmd[counter] = buffer[j];
					counter++;
				}
				checker(cmd,cb);
				pmm_free(cmd);
				i++;
				buf_length -= i;
				last_i = i;
				for(int j=last_i;j<last_i + buf_length;j++) {
					buffer[j - last_i] = buffer[j];
				}
				goto begin;
			}
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
		sendData(cb);
	} else {
		for(int i=0;i<cb.data_length;i++) {
			buffer[buf_length] = cb.data[i];
			buf_length++;
		}
		//sendData(cb);
		check_telnet_command(cb);
	}
}

void init_telnet(void) {
	if(register_tcp_listener(23, handle_telnet)) {
		buffer = pmm_alloc();
		kprintf("Telnet registered\n");
	} else {
		kprintf("Telnet not registered\n");
	}
}