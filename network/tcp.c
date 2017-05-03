#include "includes.h"

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
	kprintf("Source-Port: %d\n",HTONS(tcp.source_port));
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
	
	kprintf("\n");
	
	/*tcp.destination_port = tcp.source_port;
	tcp.source_port = 23;
	tcp.flags.syn = 0;
	tcp.flags.ack = 1;
	tcp.checksum = 0;
	ip.destinationIP = ip.sourceIP;
	ip.sourceIP = my_ip;
	ip.data = &tcp;
	uint8_t buffer[1];
	
	sendPacket(ether,&ip,ip.packetsize);*/
}

void sendTCPpacket(struct ether_header ether, struct ip_header ip, struct tcp_header tcp, uint32_t options[], int options_count, uint8_t data[], int data_length) {
	int packetsize = 20 + 20 + options_count + data_length;
	int pos = 0;
	uint8_t *temp;
	
	ip.checksum = 0;
	tcp.checksum = 0;
	
	ip.checksum = checksum(&ip,20);
	
	uint8_t tcpChecksum[12 + packetsize - 20];
	
	uint8_t buffer[packetsize];
	*temp = &ip;
	for(int i = 0; i < 20; i++) { //ip_header
		buffer[pos] = temp[i];
		pos++;
	}
	*temp = &tcp;
	for(int i = 0; i < 20; i++) { //tcp_header
		buffer[pos] = temp[i];
		pos++;
	}
	*temp = &options;
	for(int i = 0; i < options_count; i++) { //tcp_options
		buffer[pos] = temp[i];
		pos++;
	}
	*temp = &data;
	for(int i = 0; i < data_length; i++) { //tcp_data
		buffer[pos] = temp[i];
		pos++;
	}
	pos--;
	sendPacket(ether,buffer,pos);
}