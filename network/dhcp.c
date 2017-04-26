#include "includes.h"

int dhcp_status = 0;

uint32_t connection_id = 0x33224411;

void dhcp_discover(void) {
	struct ether_header ether;
	struct ip_header ip;
	struct udp_header udp;
	
	struct ip_addr ip11;
	struct ip_addr ip22;

	ip11.ip1 = 0x0;
	ip11.ip2 = 0x0;
	ip11.ip3 = 0x0;
	ip11.ip4 = 0x0;
	ip22.ip1 = 0xff;
	ip22.ip2 = 0xff;
	ip22.ip3 = 0xff;
	ip22.ip4 = 0xff;	
	
	struct dhcp_packet dhcp = {
		.operation = 0x1, // 1 Byte
		.network_type = 0x1, // 1 Byte
		.network_addr_length = 0x6, // 1 Byte
		.relay_agents = 0x0, // 1 Byte
		.connection_id = HTONL(connection_id), // 4 Byte
		.seconds_start = 0x0, // 2 Byte
		.flags = HTONS(0x8000), // 2 Byte
		.client_ip = ip11,
		.own_ip = ip11,
		.server_ip = ip11,
		.relay_ip = ip11,
		.client_mac = my_mac,
		.magic_cookie = HTONL(0x63825363),
	};
	
	//connection_id++;
	
	for(int i=0;i<10;i++) {
		dhcp.mac_padding[i] = 0;
	}
	for(int i=0;i<64;i++) {
		dhcp.server_name[i] = 0;
	}
	for(int i=0;i<128;i++) {
		dhcp.file_name[i] = 0;
	}
	
	ether.sender_mac.mac1 = 0xff;
	ether.sender_mac.mac2 = 0xff;
	ether.sender_mac.mac3 = 0xff;
	ether.sender_mac.mac4 = 0xff;
	ether.sender_mac.mac5 = 0xff;
	ether.sender_mac.mac6 = 0xff;
	ether.receipt_mac.mac1 = 0xff;
	ether.receipt_mac.mac2 = 0xff;
	ether.receipt_mac.mac3 = 0xff;
	ether.receipt_mac.mac4 = 0xff;
	ether.receipt_mac.mac5 = 0xff;
	ether.receipt_mac.mac6 = 0xff;
	ether.type = (0x0800);
	
	int udp_length = 8;
	int dhcp_length = 48 + 64 + 128;
	int dhcp_options_length = 3 + 6 + 1;
	
	int packet_length = udp_length + dhcp_length + dhcp_options_length;
	
	ip.headerlen = 5;
	ip.version = 4;
	ip.priority = 0x0;
	ip.packetsize = HTONS(20 + packet_length);
	ip.id = HTONS(packet_id);
	ip.fragment = 0x0040;
	ip.ttl = 128;
	ip.protocol = 0x11;
	ip.checksum = 0x0;
	ip.sourceIP = ip11;
	ip.destinationIP = ip22;
	
	packet_id++;

	union ip_union ip1;
	ip1.ip = ip;
	ip1.ip.checksum = HTONS(checksum(ip1.data, ip.headerlen * 4));

	ip.checksum = ip1.ip.checksum;
	
	udp.source_port = HTONS(68);
	udp.destination_port = HTONS(67);
	udp.packetsize = HTONS(packet_length);
	udp.checksum = 0x0000;
	
	union temp_dhcp {
		struct dhcp_packet dhcp;
		uint8_t data[HTONS(udp.packetsize) - 8];
	};
	union temp_dhcp temp_dhcp;
	temp_dhcp.dhcp = dhcp;
	
	/*for(int i=0;i<HTONS(udp.packetsize) - 8 - dhcp_options_length;i++) {
		udp.data[i] = temp_dhcp.data[i];
	}
	udp.data[udp_length + dhcp_length + 0] = 53;
	udp.data[udp_length + dhcp_length + 1] = 1;
	udp.data[udp_length + dhcp_length + 2] = 1;
	
	udp.data[udp_length + dhcp_length + 3] = 53;
	udp.data[udp_length + dhcp_length + 4] = 1;
	udp.data[udp_length + dhcp_length + 5] = 1;
	udp.data[udp_length + dhcp_length + 6] = 53;
	udp.data[udp_length + dhcp_length + 7] = 1;
	udp.data[udp_length + dhcp_length + 8] = 1;
	
	udp.data[udp_length + dhcp_length + 9] = 255;*/

	union temp_udp {
		struct udp_header udp;
		uint8_t data[HTONS(udp.packetsize)];
	};
	union temp_udp temp_udp;
	temp_udp.udp = udp;
	
	for(int i=0;i<HTONS(udp.packetsize) - 8 - dhcp_options_length;i++) {
		temp_udp.data[8+i] = temp_dhcp.data[i];
	}
	temp_udp.data[udp_length + dhcp_length + 0] = 53;
	temp_udp.data[udp_length + dhcp_length + 1] = 1;
	temp_udp.data[udp_length + dhcp_length + 2] = 1;
	
	temp_udp.data[udp_length + dhcp_length + 3] = 55;
	temp_udp.data[udp_length + dhcp_length + 4] = 4;
	temp_udp.data[udp_length + dhcp_length + 5] = 1;
	temp_udp.data[udp_length + dhcp_length + 6] = 3;
	temp_udp.data[udp_length + dhcp_length + 7] = 15;
	temp_udp.data[udp_length + dhcp_length + 8] = 6;
	
	temp_udp.data[udp_length + dhcp_length + 9] = 255;
	
	uint8_t checksum_header[12 + HTONS(udp.packetsize)];
	checksum_header[0] = 0;
	checksum_header[1] = 0;
	checksum_header[2] = 0;
	checksum_header[3] = 0;
	checksum_header[4] = 255;
	checksum_header[5] = 255;
	checksum_header[6] = 255;
	checksum_header[7] = 255;
	checksum_header[8] = 0;
	checksum_header[9] = 17;
	checksum_header[10] = temp_udp.data[4];
	checksum_header[11] = temp_udp.data[5];

	for(int i=0;i<HTONS(udp.packetsize);i++) {
		checksum_header[12 + i] = temp_udp.data[i];
	}
	
	//temp_udp.udp.checksum = HTONS(checksum(temp_udp.data,HTONS(udp.packetsize)));
	temp_udp.udp.checksum = HTONS(checksum(checksum_header,12 + HTONS(udp.packetsize)));
	
	uint8_t buffer1[HTONS(ip.packetsize)];
	
	memcpy(buffer1,&ip,20);
	
	for(int m=0;m<HTONS(udp.packetsize);m++) {
		buffer1[20 + m] = temp_udp.data[m];
	}
/*	for(int m=0;m<68;m++) {
		buffer1[28 + m] = temp_dhcp.data[m];
	}*/
	via_send(ether, buffer1, HTONS(ip.packetsize));
	dhcp_status = 1;
}

void dhcp_request(void) {
	kprintf("DHCP-REQUEST...\n");
	
	dhcp_status = 3;
}

void dhcp_get_ip(void) {
	kprintf("DHCP-DISCOVER...\n");
	dhcp_discover();
	//dhcp_discover();
}

void dhcp_offer(struct ether_header ether, struct udp_header udp, struct dhcp_packet dhcp) {
	kprintf("DHCP-OFFER...\n");
	
	int udp_length = 8;
	int dhcp_length = 48 + 64 + 128;
	
	struct dhcp_options options[255];
	
	for(int i=dhcp_length;i<udp.packetsize - udp_length;i++) {
		if(udp.data[i] == 255) break;

		struct dhcp_options option;
		
		option.option = udp.data[i];
		option.length = udp.data[i + 1];
		
		//kprintf("Option %d Laenge %d: ", udp.data[i], udp.data[i + 1]);
		int j;
		
		for(j=0;j<udp.data[i + 1];j++) {
			option.data[j] = udp.data[i + 2 + j];
			//data_temp[j] = udp.data[i + 2 + j];
			//kprintf("%d ",udp.data[i + 2 + j]);
		}
		//kprintf("\n");
		kprintf("Option %d Laenge %d: %d\n", option.option, option.length, option.data[0]); // Funktioniert
		options[i] = option; // Hier passiert der Fehler......
		i += 1 + j;
	}
	
	kprintf("Option %d Laenge %d\n",options[53].option,options[53].length); //Alle Werte sind 0
	kprintf("Option %d Laenge %d\n",options[54].option,options[54].length); //Alle Werte sind 0
	kprintf("Option %d Laenge %d\n",options[55].option,options[55].length); //Alle Werte sind 0
	if(options[54].option == 54) { // Funktioniert nicht
		kprintf("Server-Identifier: %d.%d.%d.%d\n",options[54].data[0],options[54].data[1],options[54].data[2],options[54].data[3]);
	}

	dhcp_status = 2;
	dhcp_request();
}

void dhcp_ack(struct ether_header ether, struct udp_header udp, struct dhcp_packet dhcp) {
	kprintf("DHCP-ACK...\n");
	//my_ip.ip1 = 0x0;
	dhcp_status = 0;
}
