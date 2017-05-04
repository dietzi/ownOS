#include "includes.h"

void sendTCPpacket(struct ether_header ether, struct ip_header ip, struct tcp_header tcp, uint32_t options[], int options_count, uint8_t *data, int data_length);
bool register_tcp_listener(int port, void *callback_pointer);

uint32_t last_seq = 0;
uint32_t last_ack = 0;
bool con_est = false;

struct tcp_callback tcp_listeners[65535];

void (*callback)(struct tcp_callback);

void tcp_handle(struct ip_header ip, struct ether_header ether) {
	struct tcp_header tcp;
	union tcpU {
		struct tcp_header tcp;
		uint8_t data[20];
	};
	union tcpU tcpU;
	for(int i=0;i<ip.data_length;i++) {
		tcpU.data[i] = ip.data[i];
	}
	tcp = tcpU.tcp;
	/*kprintf("Source-Port: %d\n",HTONS(tcp.source_port));
	kprintf("Destination-Port: %d\n",HTONS(tcp.destination_port));
	kprintf("Sequence: %d\n",HTONL(tcp.sequence_number));
	kprintf("ACK-Number: %d\n",HTONL(tcp.ack_number));
	kprintf("Header-Length: %d\n",tcp.headerlen * 4);
	kprintf("  CWR: %d\n",tcp.flags.cwr);
	kprintf("  ECE: %d\n",tcp.flags.ece);
	kprintf("  URG: %d\n",tcp.flags.urg);
	kprintf("  ACK: %d\n",tcp.flags.ack);
	kprintf("  PSH: %d\n",tcp.flags.psh);
	kprintf("  RST: %d\n",tcp.flags.rst);
	kprintf("  SYN: %d\n",tcp.flags.syn);
	kprintf("  FIN: %d\n",tcp.flags.fin);
	kprintf("Window-Size: %d\n",HTONS(tcp.window));
	kprintf("Checksum: 0x%x\n",HTONS(tcp.checksum));
	kprintf("Urgent-Pointer: 0x%x\n",HTONS(tcp.urgent_pointer));
	
	kprintf("\n");*/
	
	uint16_t temp_port = tcp.destination_port;
	tcp.destination_port = tcp.source_port;
	tcp.source_port = temp_port;

	ip.destinationIP = ip.sourceIP;
	ip.sourceIP = my_ip;
	ip.fragment = HTONS(ip.fragment);
	ip.id = HTONS(ip.id);
	
	if(tcp_listeners[HTONS(temp_port)].enabled) {
		if(tcp_listeners[HTONS(temp_port)].con_est) {
			if(tcp.flags.ack && tcp.flags.psh) {
				//raise event / data_incoming
				tcp_listeners[HTONS(temp_port)].data = tcp.data;
				tcp_listeners[HTONS(temp_port)].data_length = HTONS(ip.packetsize) - (ip.headerlen * 4) - (tcp.headerlen * 4);
				callback = tcp_listeners[HTONS(temp_port)].callback_pointer;
				callback(tcp_listeners[HTONS(temp_port)]);
			}
		} else {
			if(!tcp.flags.rst) {
				if(tcp.flags.syn && !tcp.flags.ack) {
					tcp.flags.syn = 1;
					tcp.flags.ack = 1;
					tcp.ack_number = HTONL(HTONL(tcp.sequence_number) + 1);
					tcp.sequence_number = HTONL(tcp.sequence_number);
					tcp_listeners[HTONS(temp_port)].last_seq = HTONL(tcp.sequence_number);
					tcp_listeners[HTONS(temp_port)].last_ack = HTONL(tcp.ack_number);
					sendTCPpacket(ether, ip, tcp, tcp.options, 0, tcp.data, 0);
				}
				if(!tcp.flags.syn && tcp.flags.ack) {
					if(tcp_listeners[HTONS(temp_port)].last_ack == HTONL(tcp.sequence_number) && HTONL(tcp.ack_number) == tcp_listeners[HTONS(temp_port)].last_seq + 1) {
						tcp_listeners[HTONS(temp_port)].con_est = true;
						/*tcp.flags.psh = 1;
						uint32_t temp_ack = tcp.ack_number;
						tcp.ack_number = tcp.sequence_number;
						tcp.sequence_number = temp_ack;
						last_seq = HTONL(tcp.sequence_number);
						last_ack = HTONL(tcp.ack_number);
						uint8_t *data = pmm_alloc();
						data = "Hallo\r\n\r\nEs funktioniert";
						sendTCPpacket(ether, ip, tcp, tcp.options, 0, data, 24);*/
					}
				}
			} else {
				//do reset
			}
		}
	}
}

bool register_tcp_listener(int port, void *callback_pointer) {
	struct tcp_callback cb = {
		.callback_pointer = &callback_pointer,
		.port = 23,
		.con_est = false,
		.enabled = true,
	};
	if(tcp_listeners[cb.port].enabled) {
		return false;
	} else {
		tcp_listeners[cb.port] = cb;
		tcp_listeners[cb.port].enabled = true;
		return true;
	}
}

void senData(struct tcp_callback cb) {
	
}

void sendTCPpacket(struct ether_header ether, struct ip_header ip, struct tcp_header tcp, uint32_t options[], int options_count, uint8_t *data, int data_length) {
	int packetsize = 20 + 20 + options_count + data_length;
	int pos = 0;
	int pos1 = 0;
	uint8_t *temp;
	
	ip.checksum = 0;
	ip.packetsize = HTONS((uint16_t)packetsize);
	ip.headerlen = 5;
	tcp.checksum = 0;
	tcp.headerlen = (packetsize - data_length - 20) / 4;
	
	ip.checksum = HTONS(checksum(&ip,20));
	
	struct tcp_pseudo_header head = {
		.sourceIP = ip.sourceIP,
		.destinationIP = ip.destinationIP,
		.protocol = 6,
		.tcp_length = HTONS((uint16_t)packetsize - 20)
	};
	
	uint8_t tcpChecksum[12 + packetsize - 20];
	temp = &head;
	for(int i = 0; i < 12; i++) {
		tcpChecksum[pos1] = temp[i];
		pos1++;
	}
	temp = &tcp;
	for(int i = 0; i < 20; i++) { //tcp_header
		tcpChecksum[pos1] = temp[i];
		pos1++;
	}
	temp = &options;
	for(int i = 0; i < options_count; i++) { //tcp_options
		tcpChecksum[pos1] = temp[i];
		pos1++;
	}
	//temp = &data;
	for(int i = 0; i < data_length; i++) { //tcp_data
		tcpChecksum[pos1] = data[i];
		pos1++;
	}
	//pos1--;
	tcp.checksum = HTONS(checksum(&tcpChecksum, pos1));
	
	uint8_t buffer[packetsize];
	temp = &ip;
	for(int i = 0; i < 20; i++) { //ip_header
		buffer[pos] = temp[i];
		pos++;
	}
	temp = &tcp;
	for(int i = 0; i < 20; i++) { //tcp_header
		buffer[pos] = temp[i];
		pos++;
	}
	temp = &options;
	for(int i = 0; i < options_count; i++) { //tcp_options
		buffer[pos] = temp[i];
		pos++;
	}
	//temp = &data;
	for(int i = 0; i < data_length; i++) { //tcp_data
		buffer[pos] = data[i];
		pos++;
	}
	//pos--;
	sendPacket(ether,buffer,pos);
}