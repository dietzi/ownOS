#include "includes.h"

void udp_handle(struct ip_header ip, struct ether_header ether) {
	struct udp_header udp;
	udp.source_port = (uint16_t)((ip.data[0] << 8) + (ip.data[1]));
	udp.destination_port = (uint16_t)((ip.data[2] << 8) + (ip.data[3]));
	udp.packetsize = (uint16_t)((ip.data[4] << 8) + (ip.data[5]));
	udp.checksum = (uint16_t)((ip.data[6] << 8) + (ip.data[7]));
	
	for(int i=0;i<udp.packetsize - 8;i++) {
		udp.data[i] = ip.data[8 + i];
	}
	
	if(udp.source_port == 67 && udp.destination_port == 68) {
		int udp_length = 8;
		int dhcp_length = 48 + 64 + 128;
		
		struct dhcp_packet dhcp;
		union temp_dhcp {
			struct dhcp_packet dhcp;
			uint8_t data[dhcp_length];
		};
		union temp_dhcp temp_dhcp;
		
		for(int i=0;i<dhcp_length;i++) {
			temp_dhcp.data[i] = udp.data[i];
		}
		dhcp = temp_dhcp.dhcp;

		if(dhcp.operation == 2) { // DHCP reply
			for(int i=dhcp_length;i<udp.packetsize - udp_length;i++) {
				if(udp.data[i] == 255) break;
				if(udp.data[i] == 53 && udp.data[i + 2] == 2) {
					dhcp_offer(ether, udp, dhcp);
					break;
				}
				kprintf("Option %d Laenge %d: ", udp.data[i], udp.data[i + 1]);
				int j;
				for(j=0;j<udp.data[i + 1];j++) {
					kprintf("%d ",udp.data[i + 2 + j]);
				}
				kprintf("\n");
				i += 1 + j;
			}
		}
	}
	
	kprintf("Typ: UDP\n");
	kprintf("Quellport: %d\n",udp.source_port);
	kprintf("Zielport: %d\n",udp.destination_port);
	kprintf("Packetlaenge: %d\n",udp.packetsize);
	kprintf("Pruefsumme: 0x%x\n",udp.checksum);
	kprintf("Datenlaenge: %d\n",ip.data_length - 8);
}
