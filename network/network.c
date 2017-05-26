#include "includes.h"

void sendPacket(struct ether_header ether, uint8_t data[], int data_length);
void handle_new_packet(struct network_packet *packet);

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

void sendPacket(struct ether_header ether, uint8_t *data, int data_length) {
	uint8_t buffer[data_length + 14];
	/*if((ether.receipt_mac.mac1 == my_mac.mac1 &&
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
			ether.receipt_mac.mac6 == 0xff)) {*/

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
	//}
}

void handle_new_packet(struct network_packet *packet) {
	kprintf("Got %d Bytes\n",packet->data_length);

	struct ether_header ether = {
		.receipt_mac.mac1 = ((uint8_t*)packet->bytes)[0],
		.receipt_mac.mac2 = ((uint8_t*)packet->bytes)[1],
		.receipt_mac.mac3 = ((uint8_t*)packet->bytes)[2],
		.receipt_mac.mac4 = ((uint8_t*)packet->bytes)[3],
		.receipt_mac.mac5 = ((uint8_t*)packet->bytes)[4],
		.receipt_mac.mac6 = ((uint8_t*)packet->bytes)[5],
		.sender_mac.mac1 = ((uint8_t*)packet->bytes)[6],
		.sender_mac.mac2 = ((uint8_t*)packet->bytes)[7],
		.sender_mac.mac3 = ((uint8_t*)packet->bytes)[8],
		.sender_mac.mac4 = ((uint8_t*)packet->bytes)[9],
		.sender_mac.mac5 = ((uint8_t*)packet->bytes)[10],
		.sender_mac.mac6 = ((uint8_t*)packet->bytes)[11],
		.type = HTONS(((uint16_t*)packet->bytes)[6])
	};
	//ether_header_print(ether);
	if(ether.type > 0x0600) { //Ethernet II
		switch(ether.type) {
			case 0x0806: //ARP
				kprintf("");
				struct arp arp1 = {
					.hardware_addr_type = HTONS(((uint16_t*)packet->bytes)[7]),
					.network_addr_type = HTONS(((uint16_t*)packet->bytes)[8]),
					.hardware_addr_length = (((uint8_t*)packet->bytes)[18]),
					.network_addr_length = (((uint8_t*)packet->bytes)[19]),
					.operation = HTONS(((uint16_t*)packet->bytes)[10]),
					.sender_mac.mac1 = (((uint8_t*)packet->bytes)[22]),
					.sender_mac.mac2 = (((uint8_t*)packet->bytes)[23]),
					.sender_mac.mac3 = (((uint8_t*)packet->bytes)[24]),
					.sender_mac.mac4 = (((uint8_t*)packet->bytes)[25]),
					.sender_mac.mac5 = (((uint8_t*)packet->bytes)[26]),
					.sender_mac.mac6 = (((uint8_t*)packet->bytes)[27]),
					.sender_ip.ip1 = (((uint8_t*)packet->bytes)[28]),
					.sender_ip.ip2 = (((uint8_t*)packet->bytes)[29]),
					.sender_ip.ip3 = (((uint8_t*)packet->bytes)[30]),
					.sender_ip.ip4 = (((uint8_t*)packet->bytes)[31]),
					.receipt_mac.mac1 = (((uint8_t*)packet->bytes)[32]),
					.receipt_mac.mac2 = (((uint8_t*)packet->bytes)[33]),
					.receipt_mac.mac3 = (((uint8_t*)packet->bytes)[34]),
					.receipt_mac.mac4 = (((uint8_t*)packet->bytes)[35]),
					.receipt_mac.mac5 = (((uint8_t*)packet->bytes)[36]),
					.receipt_mac.mac6 = (((uint8_t*)packet->bytes)[37]),
					.receipt_ip.ip1 = (((uint8_t*)packet->bytes)[38]),
					.receipt_ip.ip2 = (((uint8_t*)packet->bytes)[39]),
					.receipt_ip.ip3 = (((uint8_t*)packet->bytes)[40]),
					.receipt_ip.ip4 = (((uint8_t*)packet->bytes)[41])
				};
				if(arp1.receipt_ip.ip1 == my_ip.ip1 &&
						arp1.receipt_ip.ip2 == my_ip.ip2 &&
						arp1.receipt_ip.ip3 == my_ip.ip3 &&
						arp1.receipt_ip.ip4 == my_ip.ip4) {
					arp(arp1, ether);
				}
				break;
			case 0x0800: //IP
				kprintf("");
				struct ip_header ip = {
					.headerlen = (((uint8_t*)packet->bytes)[14]) & 0xF,
					.version = ((uint8_t*)packet->bytes)[14] >> 4,
					.priority = ((uint8_t*)packet->bytes)[15],
					.packetsize = HTONS(((uint16_t*)packet->bytes)[8]),
					.id = HTONS(((uint16_t*)packet->bytes)[9]),
					.fragment = HTONS(((uint16_t*)packet->bytes)[10]),
					.ttl = ((uint8_t*)packet->bytes)[22],
					.protocol = ((uint8_t*)packet->bytes)[23],
					.checksum = HTONS(((uint16_t*)packet->bytes)[12]),
					.sourceIP.ip1 = ((uint8_t*)packet->bytes)[26],
					.sourceIP.ip2 = ((uint8_t*)packet->bytes)[27],
					.sourceIP.ip3 = ((uint8_t*)packet->bytes)[28],
					.sourceIP.ip4 = ((uint8_t*)packet->bytes)[29],
					.destinationIP.ip1 = ((uint8_t*)packet->bytes)[30],
					.destinationIP.ip2 = ((uint8_t*)packet->bytes)[31],
					.destinationIP.ip3 = ((uint8_t*)packet->bytes)[32],
					.destinationIP.ip4 = ((uint8_t*)packet->bytes)[33]
				};
				ip.data_length = (ip.packetsize - (ip.headerlen * 4));
				
				uint8_t *daten;
				for(int z = 0; z < (ip.packetsize - (ip.headerlen * 4)); z++) {
					int offset = (ip.headerlen * 4) + sizeof(struct ether_header) + z;
					ip.data[z] = ((uint8_t*)packet->bytes)[offset];
				}
				ip_handle(ip,ether);
				break;
			default:
				kprintf("Nicht behandeltes Protokoll: %x\n",ether.type);
		}
	} else if(ether.type <= 0x05DC) { //Ethernet I
		kprintf("Ethernet I Pakete werden nicht behandelt\n");
		kprintf("Sender-MAC: %x:%x:%x:%x:%x:%x\n",ether.sender_mac.mac1,
													ether.sender_mac.mac2,
													ether.sender_mac.mac3,
													ether.sender_mac.mac4,
													ether.sender_mac.mac5,
													ether.sender_mac.mac6);
	}
	
	pmm_free(packet->bytes);
	pmm_free(packet);
}