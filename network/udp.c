#include "includes.h"

void udp_handle(struct ip_header ip, struct ether_header ether) {
	struct udp_header udp;
	udp.source_port = (uint16_t)((ip.data[0] << 8) + (ip.data[1]));
	udp.destination_port = (uint16_t)((ip.data[2] << 8) + (ip.data[3]));
	udp.packetsize = (uint16_t)((ip.data[4] << 8) + (ip.data[5]));
	udp.checksum = (uint16_t)((ip.data[6] << 8) + (ip.data[7]));
	udp.data = pmm_alloc();
	
	for(int i=0;i<udp.packetsize - 8;i++) {
		udp.data[i] = ip.data[8 + i];
	}
	
	if(udp.source_port == 67 && udp.destination_port == 68) {
		handle_dhcp(ether, udp);
	}
	
	kprintf("Typ: UDP\n");
	kprintf("Quellport: %d\n",udp.source_port);
	kprintf("Zielport: %d\n",udp.destination_port);
	kprintf("Packetlaenge: %d\n",udp.packetsize);
	kprintf("Pruefsumme: 0x%x\n",udp.checksum);
	kprintf("Datenlaenge: %d\n",ip.data_length - 8);
	
	pmm_free(udp.data);
}
