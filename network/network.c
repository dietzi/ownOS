#include "includes.h"

void sendPacket(struct ether_header* ether, uint8_t data[], int data_length);
void handle_new_packet(struct network_packet *packet);
void init_network(void);

void (*init_card)(pci_bdf_t device);
void (*send_packet)(uint8_t *data, int data_length);

struct pci_device {
	uint16_t vendor_id;
	uint16_t device_id;
	void* init_pointer;
	void* send_pointer;
};

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

void sendPacket(struct ether_header* ether, uint8_t *data, int data_length) {
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

		struct ether_header* ether_temp = pmm_alloc();
		
		ether_temp->receipt_mac.mac1 = ether->sender_mac.mac1;
		ether_temp->receipt_mac.mac2 = ether->sender_mac.mac2;
		ether_temp->receipt_mac.mac3 = ether->sender_mac.mac3;
		ether_temp->receipt_mac.mac4 = ether->sender_mac.mac4;
		ether_temp->receipt_mac.mac5 = ether->sender_mac.mac5;
		ether_temp->receipt_mac.mac6 = ether->sender_mac.mac6;
		
		ether_temp->sender_mac.mac1 = my_mac.mac1;
		ether_temp->sender_mac.mac2 = my_mac.mac2;
		ether_temp->sender_mac.mac3 = my_mac.mac3;
		ether_temp->sender_mac.mac4 = my_mac.mac4;
		ether_temp->sender_mac.mac5 = my_mac.mac5;
		ether_temp->sender_mac.mac6 = my_mac.mac6;
		
		ether_temp->type = HTONS(ether->type);
		
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
		last_message = "network_send...";
		//via_send(buffer,i);
		send_packet(buffer,i);
}

void handle_new_packet(struct network_packet* packet) {
	last_message = "handle_new_packet()";
	struct ether_header* ether = pmm_alloc();
	ether->receipt_mac.mac1 = ((uint8_t*)packet->bytes)[0];
	ether->receipt_mac.mac2 = ((uint8_t*)packet->bytes)[1];
	ether->receipt_mac.mac3 = ((uint8_t*)packet->bytes)[2];
	ether->receipt_mac.mac4 = ((uint8_t*)packet->bytes)[3];
	ether->receipt_mac.mac5 = ((uint8_t*)packet->bytes)[4];
	ether->receipt_mac.mac6 = ((uint8_t*)packet->bytes)[5];
	ether->sender_mac.mac1 = ((uint8_t*)packet->bytes)[6];
	ether->sender_mac.mac2 = ((uint8_t*)packet->bytes)[7];
	ether->sender_mac.mac3 = ((uint8_t*)packet->bytes)[8];
	ether->sender_mac.mac4 = ((uint8_t*)packet->bytes)[9];
	ether->sender_mac.mac5 = ((uint8_t*)packet->bytes)[10];
	ether->sender_mac.mac6 = ((uint8_t*)packet->bytes)[11];
	ether->type = HTONS(((uint16_t*)packet->bytes)[6]);
	//ether_header_print(ether);
	last_message = "ethernet-type";
	if(ether->type > 0x0600) { //Ethernet II
		switch(ether->type) {
			case 0x0806: //ARP
				last_message = "ARP";
				kprintf("");
				struct arp* arp1 = pmm_alloc();
				arp1->hardware_addr_type = HTONS(((uint16_t*)packet->bytes)[7]);
				arp1->network_addr_type = HTONS(((uint16_t*)packet->bytes)[8]);
				arp1->hardware_addr_length = (((uint8_t*)packet->bytes)[18]);
				arp1->network_addr_length = (((uint8_t*)packet->bytes)[19]);
				arp1->operation = HTONS(((uint16_t*)packet->bytes)[10]);
				arp1->sender_mac.mac1 = (((uint8_t*)packet->bytes)[22]);
				arp1->sender_mac.mac2 = (((uint8_t*)packet->bytes)[23]);
				arp1->sender_mac.mac3 = (((uint8_t*)packet->bytes)[24]);
				arp1->sender_mac.mac4 = (((uint8_t*)packet->bytes)[25]);
				arp1->sender_mac.mac5 = (((uint8_t*)packet->bytes)[26]);
				arp1->sender_mac.mac6 = (((uint8_t*)packet->bytes)[27]);
				arp1->sender_ip.ip1 = (((uint8_t*)packet->bytes)[28]);
				arp1->sender_ip.ip2 = (((uint8_t*)packet->bytes)[29]);
				arp1->sender_ip.ip3 = (((uint8_t*)packet->bytes)[30]);
				arp1->sender_ip.ip4 = (((uint8_t*)packet->bytes)[31]);
				arp1->receipt_mac.mac1 = (((uint8_t*)packet->bytes)[32]);
				arp1->receipt_mac.mac2 = (((uint8_t*)packet->bytes)[33]);
				arp1->receipt_mac.mac3 = (((uint8_t*)packet->bytes)[34]);
				arp1->receipt_mac.mac4 = (((uint8_t*)packet->bytes)[35]);
				arp1->receipt_mac.mac5 = (((uint8_t*)packet->bytes)[36]);
				arp1->receipt_mac.mac6 = (((uint8_t*)packet->bytes)[37]);
				arp1->receipt_ip.ip1 = (((uint8_t*)packet->bytes)[38]);
				arp1->receipt_ip.ip2 = (((uint8_t*)packet->bytes)[39]);
				arp1->receipt_ip.ip3 = (((uint8_t*)packet->bytes)[40]);
				arp1->receipt_ip.ip4 = (((uint8_t*)packet->bytes)[41]);

				if(arp1->receipt_ip.ip1 == my_ip.ip1 &&
						arp1->receipt_ip.ip2 == my_ip.ip2 &&
						arp1->receipt_ip.ip3 == my_ip.ip3 &&
						arp1->receipt_ip.ip4 == my_ip.ip4) {
				}
					arp(arp1, ether);
				pmm_free(arp1);
				break;
			case 0x0800: //IP
				last_message = "IP";
				kprintf("");
				struct ip_header* ip = pmm_alloc();
				ip->headerlen = (((uint8_t*)packet->bytes)[14]) & 0xF;
				ip->version = ((uint8_t*)packet->bytes)[14] >> 4;
				ip->priority = ((uint8_t*)packet->bytes)[15];
				ip->packetsize = HTONS(((uint16_t*)packet->bytes)[8]);
				ip->id = HTONS(((uint16_t*)packet->bytes)[9]);
				ip->fragment = HTONS(((uint16_t*)packet->bytes)[10]);
				ip->ttl = ((uint8_t*)packet->bytes)[22];
				ip->protocol = ((uint8_t*)packet->bytes)[23];
				ip->checksum = HTONS(((uint16_t*)packet->bytes)[12]);
				ip->sourceIP.ip1 = ((uint8_t*)packet->bytes)[26];
				ip->sourceIP.ip2 = ((uint8_t*)packet->bytes)[27];
				ip->sourceIP.ip3 = ((uint8_t*)packet->bytes)[28];
				ip->sourceIP.ip4 = ((uint8_t*)packet->bytes)[29];
				ip->destinationIP.ip1 = ((uint8_t*)packet->bytes)[30];
				ip->destinationIP.ip2 = ((uint8_t*)packet->bytes)[31];
				ip->destinationIP.ip3 = ((uint8_t*)packet->bytes)[32];
				ip->destinationIP.ip4 = ((uint8_t*)packet->bytes)[33];

				ip->data_length = (ip->packetsize - (ip->headerlen * 4));
				
				uint8_t *daten;
				for(int z = 0; z < (ip->packetsize - (ip->headerlen * 4)); z++) {
					int offset = (ip->headerlen * 4) + sizeof(struct ether_header) + z;
					ip->data[z] = ((uint8_t*)packet->bytes)[offset];
				}
				ip_handle(ip,ether);
				pmm_free(ip);
				break;
			default:
				last_message = "not handled";
				kprintf("Nicht behandeltes Protokoll: %x\n",ether->type);
		}
	} else if(ether->type <= 0x05DC) { //Ethernet I
		last_message = "etherne I";
		kprintf("Ethernet I Pakete werden nicht behandelt: ");
		kprintf("Sender-MAC: %x:%x:%x:%x:%x:%x\n",ether->sender_mac.mac1,
													ether->sender_mac.mac2,
													ether->sender_mac.mac3,
													ether->sender_mac.mac4,
													ether->sender_mac.mac5,
													ether->sender_mac.mac6);
	}
	last_message = "freeing ram";
	pmm_free(ether);
	last_message = "end handle_new_packet";
}

void init_network(void) {
	struct pci_device devices[2];
	devices[0].vendor_id = 0x1106;
	devices[0].device_id = 0x3065;
	devices[0].init_pointer = via_init;
	devices[0].send_pointer = via_send;

	devices[1].vendor_id = 0x10ec;
	devices[1].device_id = 0x8168;
	devices[1].init_pointer = realtek_init;
	devices[1].send_pointer = realtek_send_packet;
	
	pci_bdf_t device;
	
	for(int i = 0; i < 2; i++) {
		device = search_pci_device(devices[i].vendor_id,devices[i].device_id);
		if(device.bus >= 0 && device.func >= 0 && device.dev >= 0) {
			init_card = devices[i].init_pointer;
			send_packet = devices[i].send_pointer;
			init_card(device);
		}
	}
}