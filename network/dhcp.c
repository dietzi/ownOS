#include "includes.h"

int dhcp_status = 0;

uint32_t connection_id = 0x33224411;

struct dhcp_packet_created create_dhcp_packet(struct dhcp_packet);
void dhcp_discover(void);
void dhcp_offer(struct dhcp_packet dhcp);
void dhcp_request(struct ip_addr server_ip, struct ip_addr own_ip);
void dhcp_ack(struct dhcp_packet dhcp);
void dhcp_get_ip(void);
void handle_dhcp(struct ether_header ether, struct udp_header udp1);

void dhcp_discover(void) {
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
		.options = pmm_alloc()
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

	//struct dhcp_packet_created dhcp_send = 
	//sleep(2000);
	create_dhcp_packet(dhcp);
	//sendPacket(dhcp_send.ether, dhcp_send.data, dhcp_send.length);

	for(int i=0;i<255;i++) {
		pmm_free(dhcp.options[i].data);
	}

	pmm_free(dhcp.options);
	dhcp_status = 1;	
}

struct ip_addr server_ip;
struct ip_addr own_ip;

void dhcp_request(struct ip_addr server_ip, struct ip_addr own_ip) {
	kprintf("DHCP-REQUEST...\n");
	
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
	
	//struct dhcp_packet_created dhcp_send = 
	create_dhcp_packet(dhcp);
	//sendPacket(dhcp_send.ether, dhcp_send.data, dhcp_send.length);

	for(int i=0;i<255;i++) {
		pmm_free(dhcp.options[i].data);
	}

	pmm_free(dhcp.options);

	dhcp_status = 3;
}

void dhcp_offer(struct dhcp_packet dhcp1) {
	kprintf("DHCP-OFFER...\n");
	
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
	kprintf("DHCP-ACK...\n");
	//my_ip.ip1 = 0x0;
	dhcp_status = 0;
}

void dhcp_get_ip(void) {
/*	switch(dhcp_status) {
		case 0:
			kprintf("DHCP-DISCOVER...\n");
			dhcp_discover();
			break;
			
		case 2:
			dhcp_request(server_ip, own_ip);
			break;
	}*/
//	sleep(1000);
	kprintf("DHCP-DISCOVER...\n");
	dhcp_discover();
}

void handle_dhcp(struct ether_header ether, struct udp_header udp1) {
	struct udp_header udp = udp1;
	int udp_length = 8;
	int dhcp_length = 48 + 64 + 128;
	
	struct dhcp_packet dhcp;
	for(int i=0;i<255;i++) {
		dhcp.options[i].data = pmm_alloc();
		dhcp.options[i].index = 0;
	}
	
	for(int i=dhcp_length;i<udp.packetsize - udp_length;i++) {
		if(udp.data[i] == 255) break;
		
		uint8_t optionIndex = udp.data[i];
				
		dhcp.options[optionIndex].index = optionIndex;
		dhcp.options[optionIndex].length = udp.data[i + 1];

		int j;
		
		for(j=0;j<dhcp.options[optionIndex].length;j++) {
			dhcp.options[optionIndex].data[j] = udp.data[i + 2 + j];
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
		temp_dhcp.data[i] = udp.data[i];
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
