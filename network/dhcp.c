#include "includes.h"

void dhcp_discover(void);
void dhcp_offer(struct dhcp_packet dhcp);
void dhcp_request(struct ip_addr server_ip, struct ip_addr own_ip);
void dhcp_ack(struct dhcp_packet dhcp);
void dhcp_get_ip(void);
void handle_dhcp(struct ether_header* ether, struct udp_header* udp1);

uint32_t connection_id = 0x33224411;

//struct dhcp_packet_created create_dhcp_packet(struct dhcp_packet);

int dhcp_status = 0;
int dhcp_timer = 0;

struct dhcp_packet_created create_dhcp_packet(struct dhcp_packet dhcp) {
	//kprintf("Creating DHCP-Packet\n");
	struct ether_header* ether;
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
	
	ether->sender_mac.mac1 = 0xff;
	ether->sender_mac.mac2 = 0xff;
	ether->sender_mac.mac3 = 0xff;
	ether->sender_mac.mac4 = 0xff;
	ether->sender_mac.mac5 = 0xff;
	ether->sender_mac.mac6 = 0xff;
	ether->receipt_mac.mac1 = 0xff;
	ether->receipt_mac.mac2 = 0xff;
	ether->receipt_mac.mac3 = 0xff;
	ether->receipt_mac.mac4 = 0xff;
	ether->receipt_mac.mac5 = 0xff;
	ether->receipt_mac.mac6 = 0xff;
	ether->type = (0x0800);
	
	int udp_length = 8;
	int dhcp_length = 48 + 64 + 128;
	int dhcp_options_length = 1;

	for(int i=1;i<255;i++) {
		if(dhcp.options[i].index == i) {
			dhcp_options_length++;
			dhcp_options_length++;
			for(int j=0;j<dhcp.options[i].length;j++) {
				dhcp_options_length++;
			}
		}
	}
	
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

	union ip_union {
		struct ip_header ip;
		uint8_t data[sizeof(struct ip_header)];
	};

	union ip_union ip1;
	ip1.ip = ip;
	ip1.ip.checksum = HTONS(checksum(ip1.data, ip.headerlen * 4));

	ip.checksum = ip1.ip.checksum;
	
	last_message = "Creating UDP-Header";
	
	udp.source_port = HTONS(68);
	udp.destination_port = HTONS(67);
	udp.packetsize = HTONS(packet_length);
	udp.checksum = 0x0000;
	
	union temp_dhcp {
		struct dhcp_packet dhcp;
		uint8_t data[packet_length - 8];
	};
	union temp_dhcp temp_dhcp;
	temp_dhcp.dhcp = dhcp;

	union temp_udp {
		struct udp_header udp;
		uint8_t data[packet_length];
	};
	union temp_udp temp_udp;
	temp_udp.udp = udp;
	
	for(int i=0;i<packet_length - 8 - dhcp_options_length;i++) {
		temp_udp.data[8+i] = temp_dhcp.data[i];
	}
	int counter = 0;
		
	//kprintf("Getting DHCP-Options\n");
	for(int i=1;i<255;i++) {
		if(dhcp.options[i].index == i) {
			temp_udp.data[udp_length + dhcp_length + counter] = i;
			counter++;
			temp_udp.data[udp_length + dhcp_length + counter] = dhcp.options[i].length;
			counter++;
			for(int j=0;j<dhcp.options[i].length;j++) {
				temp_udp.data[udp_length + dhcp_length + counter] = dhcp.options[i].data[j];
				counter++;
			}
		}
	}
	temp_udp.data[udp_length + dhcp_length + counter] = 255;
	
	last_message = "Getting DHCP-Options";
	
	uint8_t checksum_header[12 + packet_length];
	checksum_header[0] = ip11.ip1;
	checksum_header[1] = ip11.ip2;
	checksum_header[2] = ip11.ip3;
	checksum_header[3] = ip11.ip4;
	checksum_header[4] = ip22.ip1;
	checksum_header[5] = ip22.ip2;
	checksum_header[6] = ip22.ip3;
	checksum_header[7] = ip22.ip4;
	checksum_header[8] = 0;
	checksum_header[9] = 17;
	checksum_header[10] = temp_udp.data[4];
	checksum_header[11] = temp_udp.data[5];

	for(int i=0;i<packet_length;i++) {
		checksum_header[12 + i] = temp_udp.data[i];
	}
	
	temp_udp.udp.checksum = HTONS(checksum(checksum_header,12 + packet_length));
	
	//uint8_t buffer1[HTONS(ip.packetsize)];
	uint8_t *buffer1 = pmm_alloc();
	
	last_message = "Copying Buffer";
	
	ip1.ip = ip;

	//memcpy(buffer1,&ip,20);
	for(int m=0;m<20;m++) {
		buffer1[m] = ip1.data[m];
	}
	for(int m=0;m<packet_length;m++) {
		buffer1[20 + m] = temp_udp.data[m];
	}

	/*last_message = "Creating Returner";
	
	struct dhcp_packet_created returner;
		
	returner.data = pmm_alloc();
	
	returner.length = 20 + packet_length;
	//returner.data = buffer1;
	memcpy(returner.data,buffer1,20 + packet_length);
	returner.ether = ether;*/
	
	last_message = "Returning......";
	sendPacket(ether, buffer1, 20 + packet_length);
	pmm_free(buffer1);
	pmm_free(ether);
	//return returner;
}

void dhcp_discover(void) {
	//kprintf("DHCP-DISCOVER...\n");
	dhcp_status = 1;	
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
		//.options = pmm_alloc()
	};
	
	for(int i=0;i<255;i++) {
		dhcp.options[i].data = pmm_alloc();
		dhcp.options[i].index = 0;
	}
	
	dhcp.options[53].index = 53;
	dhcp.options[53].length = 1;
	dhcp.options[53].data[0] = 1;
	
	dhcp.options[55].index = 55;
	dhcp.options[55].length = 4;
	dhcp.options[55].data[0] = 1;
	dhcp.options[55].data[1] = 3;
	dhcp.options[55].data[2] = 15;
	dhcp.options[55].data[3] = 6;

	dhcp.options[12].index = 12;
	dhcp.options[12].length = 4;
	dhcp.options[12].data[0] = (int)'F';
	dhcp.options[12].data[1] = (int)'A';
	dhcp.options[12].data[2] = (int)'O';
	dhcp.options[12].data[3] = (int)'S';
	
	//struct dhcp_packet_created dhcp_send = 
	//sleep(2000);
	create_dhcp_packet(dhcp);
	//sendPacket(dhcp_send.ether, dhcp_send.data, dhcp_send.length);

	for(int i=0;i<255;i++) {
		pmm_free(dhcp.options[i].data);
	}

	//pmm_free(dhcp.options);
}

struct ip_addr server_ip;
struct ip_addr own_ip;

void dhcp_request(struct ip_addr server_ip, struct ip_addr own_ip) {
	//kprintf("DHCP-REQUEST...\n");
	
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

	struct dhcp_packet dhcp;
	
		dhcp.operation = 0x1; // 1 Byte
		dhcp.network_type = 0x1; // 1 Byte
		dhcp.network_addr_length = 0x6; // 1 Byte
		dhcp.relay_agents = 0x0; // 1 Byte
		dhcp.connection_id = HTONL(connection_id); // 4 Byte
		dhcp.seconds_start = 0x0; // 2 Byte
		dhcp.flags = HTONS(0x8000); // 2 Byte
		dhcp.client_ip = ip11;
		dhcp.own_ip = ip11;
		dhcp.server_ip = server_ip;
		dhcp.relay_ip = ip11;
		dhcp.client_mac = my_mac;
		dhcp.magic_cookie = HTONL(0x63825363);
		//dhcp.options = pmm_alloc();
	
	for(int i=0;i<255;i++) {
		dhcp.options[i].data = pmm_alloc();
		dhcp.options[i].index = 0;
	}
	
	dhcp.options[53].index = 53;
	dhcp.options[53].length = 1;
	dhcp.options[53].data[0] = 3;
	
	dhcp.options[50].index = 50;
	dhcp.options[50].length = 4;
	dhcp.options[50].data[0] = own_ip.ip1;
	dhcp.options[50].data[1] = own_ip.ip2;
	dhcp.options[50].data[2] = own_ip.ip3;
	dhcp.options[50].data[3] = own_ip.ip4;

	dhcp.options[54].index = 54;
	dhcp.options[54].length = 4;
	dhcp.options[54].data[0] = server_ip.ip1;
	dhcp.options[54].data[1] = server_ip.ip2;
	dhcp.options[54].data[2] = server_ip.ip3;
	dhcp.options[54].data[3] = server_ip.ip4;
	
	dhcp.options[12].index = 12;
	dhcp.options[12].length = 4;
	dhcp.options[12].data[0] = (int)'F';
	dhcp.options[12].data[1] = (int)'A';
	dhcp.options[12].data[2] = (int)'O';
	dhcp.options[12].data[3] = (int)'S';
	
	//struct dhcp_packet_created dhcp_send = 
	create_dhcp_packet(dhcp);
	//sendPacket(dhcp_send.ether, dhcp_send.data, dhcp_send.length);

	for(int i=0;i<255;i++) {
		pmm_free(dhcp.options[i].data);
	}

	//pmm_free(dhcp.options);

	dhcp_status = 3;
}

void dhcp_offer(struct dhcp_packet dhcp1) {
	//kprintf("DHCP-OFFER...\n");
	
	if(dhcp1.connection_id == HTONL(connection_id)) {
		dhcp_status = 2;
		server_ip.ip1 = dhcp1.options[54].data[0];
		server_ip.ip2 = dhcp1.options[54].data[1];
		server_ip.ip3 = dhcp1.options[54].data[2];
		server_ip.ip4 = dhcp1.options[54].data[3];
		own_ip.ip1 = dhcp1.own_ip.ip1;
		own_ip.ip2 = dhcp1.own_ip.ip2;
		own_ip.ip3 = dhcp1.own_ip.ip3;
		own_ip.ip4 = dhcp1.own_ip.ip4;
		dhcp_request(server_ip, own_ip);
	}
}

void dhcp_ack(struct dhcp_packet dhcp) {
	//kprintf("DHCP-ACK...\n");
	if(dhcp.connection_id == HTONL(connection_id)) {
		if(dhcp.own_ip.ip1 == own_ip.ip1 &&
				dhcp.own_ip.ip2 == own_ip.ip2 &&
				dhcp.own_ip.ip3 == own_ip.ip3 &&
				dhcp.own_ip.ip4 == own_ip.ip4) {
			my_ip = own_ip;
			uint32_t timer = dhcp.options[51].data[0] >> 24 |
								dhcp.options[51].data[1] >> 16 |
								dhcp.options[51].data[2] >> 8 |
								(dhcp.options[51].data[3] & 0x000000FF);
			dhcp_timer = timer * 1000;
			register_timer(dhcp_discover, timer * 1000, true, NULL);
			kprintf("IP: %d.%d.%d.%d   Release in: %ds\n",my_ip.ip1,my_ip.ip2,my_ip.ip3,my_ip.ip4, timer);
			dhcp_status = 5;
		} else {
			dhcp_status = 0;
		}
	}
}

void dhcp_get_ip(void) {
	if(dhcp_timer <= 0 && dhcp_status == 5) {
		//dhcp_status = 0;
	}
	switch(dhcp_status) {
		case 0:
			dhcp_discover();
			break;
			
		case 2:
			dhcp_request(server_ip, own_ip);
			break;
	}
//	sleep(1000);
//	kprintf("DHCP-DISCOVER...\n");
//	dhcp_discover();
}

void handle_dhcp(struct ether_header* ether, struct udp_header* udp) {
	//struct udp_header udp = udp1;
	int udp_length = 8;
	int dhcp_length = 48 + 64 + 128;
	
	struct dhcp_packet dhcp;
	for(int i=0;i<255;i++) {
		dhcp.options[i].data = pmm_alloc();
		dhcp.options[i].index = 0;
	}
	
	for(int i=dhcp_length; i < udp->packetsize - udp_length; i++) {
		if(udp->data[i] == 255) break;
		
		uint8_t optionIndex = udp->data[i];
				
		dhcp.options[optionIndex].index = optionIndex;
		dhcp.options[optionIndex].length = udp->data[i + 1];

		int j;
		
		for(j=0; j < dhcp.options[optionIndex].length; j++) {
			dhcp.options[optionIndex].data[j] = udp->data[i + 2 + j];
		}
		
		i += 1 + j;
	}
	union temp_dhcp {
		struct dhcp_packet dhcp;
		uint8_t data[dhcp_length];
	};
	union temp_dhcp temp_dhcp;
	
	temp_dhcp.dhcp = dhcp;
	
	for(int i=0;i<dhcp_length;i++) {
		temp_dhcp.data[i] = udp->data[i];
	}
	dhcp = temp_dhcp.dhcp;
	
	if(dhcp.operation == 2) { // DHCP reply
		if(dhcp.options[53].data[0] == 2) {
			dhcp_offer(dhcp);			
		}
		if(dhcp.options[53].data[0] == 5) {
			dhcp_ack(dhcp);			
		}
	}	
	for(int i=0;i<255;i++) {
		pmm_free(dhcp.options[i].data);
	}
}
