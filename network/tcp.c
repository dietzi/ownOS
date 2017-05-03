#include "includes.h"

void tcp_handle(struct ip_header ip, struct ether_header ether) {
	struct tcp_header tcp;
	union tcpU {
		struct tcp_header tcp;
		uint8_t data[20];
	};
	union tcpU tcpU;
	for(int i=0;i<20;i++) {
		tcpU.data[i] = ip.data[i];
	}
	tcp = tcpU.tcp;
	kprintf("Source-Port: %d\n",HTONS(tcp.source_port));
	kprintf("Destination-Port: %d\n",HTONS(tcp.destination_port));
	kprintf("Sequence: %d\n",HTONL(tcp.sequence_number));
	
}