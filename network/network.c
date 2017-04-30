#include "includes.h"

void sendPacket(struct ether_header ether, uint8_t data[], int data_length);

int checksum(void *buffer, int size) {
  uint32_t sum = 0;
  int odd = 0, i;
  uint8_t *data = buffer;
 
  if (!buffer || (size < 2)) {
	  kprintf("Checksum: Buffer Fehler\n");
    return 0;
  }
  if ((size>>1)*2 != size)
  {
    odd = 1;
    size--;
  }
  for (i = 0; i < size; i += 2)
    sum += ((data[i]<<8) & 0xFF00) + (data[i+1] & 0xFF);
  if (odd)
    sum += (data[i]<<8) & 0xFF00;
  while (sum >> 16)
    sum = (sum&0xFFFF) + (sum>>16);
 
  return (~sum)&0xFFFF;
}
struct dhcp_packet_created create_dhcp_packet(struct dhcp_packet dhcp) {
	//kprintf("Creating DHCP-Packet\n");
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
	//return returner;
}


void sendPacket(struct ether_header ether, uint8_t *data, int data_length) {
	kprintf("Send Packet....\n");
	uint8_t buffer[data_length + 20];
	if((ether.receipt_mac.mac1 == my_mac.mac1 &&
			ether.receipt_mac.mac2 == my_mac.mac2 &&
			ether.receipt_mac.mac3 == my_mac.mac3 &&
			ether.receipt_mac.mac4 == my_mac.mac4 &&
			ether.receipt_mac.mac5 == my_mac.mac5 &&
			ether.receipt_mac.mac6 == my_mac.mac6) ||
			(ether.receipt_mac.mac1 == 0xff &&
			ether.receipt_mac.mac2 == 0xff &&
			ether.receipt_mac.mac3 == 0xff &&
			ether.receipt_mac.mac4 == 0xff &&
			ether.receipt_mac.mac5 == 0xff &&
			ether.receipt_mac.mac6 == 0xff)) {

		struct ether_header ether_temp;
		
		ether_temp.receipt_mac.mac1 = ether.sender_mac.mac1;
		ether_temp.receipt_mac.mac2 = ether.sender_mac.mac2;
		ether_temp.receipt_mac.mac3 = ether.sender_mac.mac3;
		ether_temp.receipt_mac.mac4 = ether.sender_mac.mac4;
		ether_temp.receipt_mac.mac5 = ether.sender_mac.mac5;
		ether_temp.receipt_mac.mac6 = ether.sender_mac.mac6;
		
		ether_temp.sender_mac.mac1 = my_mac.mac1;
		ether_temp.sender_mac.mac2 = my_mac.mac2;
		ether_temp.sender_mac.mac3 = my_mac.mac3;
		ether_temp.sender_mac.mac4 = my_mac.mac4;
		ether_temp.sender_mac.mac5 = my_mac.mac5;
		ether_temp.sender_mac.mac6 = my_mac.mac6;
		
		ether_temp.type = HTONS(ether.type);
		
		union ether_test ether_union;
		ether_union.ether_val1 = ether_temp;
		
		int i=0;
		int j=0;

		while(i < 14) {
			buffer[i] = ether_union.data[i];
			i++;
		}
		while(data_length - j > 0) {
			buffer[i] = data[j];
			i++;
			j++;
		}
		while(j < 46) {
			buffer[i] = 0x0;
			i++;
			j++;
		}
last_message = "via_send...";
		via_send(buffer,i);
	}
}