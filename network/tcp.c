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
	kprintf("  CWR: %d\n",tcp.flags.cwr);
	kprintf("  ECE: %d\n",tcp.flags.ece);
	kprintf("  URG: %d\n",tcp.flags.urg);
	kprintf("  ACK: %d\n",tcp.flags.ack);
	kprintf("  PSH: %d\n",tcp.flags.psh);
	kprintf("  RST: %d\n",tcp.flags.rst);
	kprintf("  SYN: %d\n",tcp.flags.syn);
	kprintf("  FIN: %d\n",tcp.flags.fin);
	
	kprintf("\n");
}