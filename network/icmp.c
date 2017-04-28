#include "includes.h"

void icmp_handle(struct ip_header ip, struct ether_header ether) {
	if(ip.data[0] == 0x08) { //ICMP-Request //Ping
		ip.headerlen = 5;
		ip.version = 4;
		ip.priority = 0x0;
		ip.packetsize = HTONS(ip.packetsize);
		ip.id = HTONS(packet_id);
		ip.fragment = 0x0040;
		ip.ttl = 128;
		ip.protocol = 0x1;
		ip.checksum = 0x0;
		ip.destinationIP = ip.sourceIP;
		ip.sourceIP = my_ip;
		
		packet_id++;
		
		union ip_union ip1;
		ip1.ip = ip;
		ip1.ip.checksum = HTONS(checksum(ip1.data, ip.headerlen * 4));
		
		ip.checksum = ip1.ip.checksum;

		
		ip.data[0] = 0x0;
		ip.data[1] = 0x0;
		ip.data[2] = 0x0;
		ip.data[3] = 0x0;

		uint8_t data1[ip.data_length];
		for(int m=0;m<ip.data_length;m++) {
			data1[m] = ip.data[m];
		}
		
		uint16_t checksum1 = (checksum(data1,ip.data_length));

		ip.data[2] = ((uint8_t)(checksum1 >> 8));
		ip.data[3] = ((uint8_t)checksum1);

		uint8_t buffer1[HTONS(ip1.ip.packetsize)];
		
		memcpy(buffer1,&ip,20);
		
		for(int m=0;m<ip.data_length;m++) {
			buffer1[20 + m] = ip.data[m];
		}
		
		sendPacket(ether, buffer1, HTONS(ip1.ip.packetsize));
	}
}
