#include "includes.h"

//int16_t __builtin_bswap16 (uint16_t x)

#define HTONS(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#define NTOHS(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))

#define HTONL(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                  ((((unsigned long)(n) & 0xFF00)) << 8) | \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))

#define NTOHL(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                  ((((unsigned long)(n) & 0xFF00)) << 8) | \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))

int base=0;
uint16_t packet_id = 0x0000;
int next_tx = 0;
void *tx_addr;

int init_i = 0;

pci_bdf_t addr = {
	.bus=0,
	.dev=18,
	.func=0
};

struct ip_addr my_ip = {
	.ip1 = 10,
	.ip2 = 0,
	.ip3 = 0,
	.ip4 = 114
};

struct ip_addr broadcast_ip = {
	.ip1 = 255,
	.ip2 = 255,
	.ip3 = 255,
	.ip4 = 255
};

struct mac my_mac = {
	.mac1 = 0x00,
	.mac2 = 0xE0,
	.mac3 = 0xC5,
	.mac4 = 0x52,
	.mac5 = 0xD2,
	.mac6 = 0x54
};

struct mac broadcast_mac = {
	.mac1 = 0xFF,
	.mac2 = 0xFF,
	.mac3 = 0xFF,
	.mac4 = 0xFF,
	.mac5 = 0xFF,
	.mac6 = 0xFF
};

void via_wait_bit(uint8_t reg, uint8_t mask, bool low) {
	int i;
	
	for(i=0;i<1024;i++) {
		bool has_mask_bits = !!(pci_read_register(addr,base,reg) & mask);
		if(low ^ has_mask_bits) break;
		sleep(10);
	}
}

void via_wait_bit_low(uint8_t reg, uint8_t mask) {
	via_wait_bit(reg,mask,true);
}

void via_wait_bit_high(uint8_t reg, uint8_t mask) {
	via_wait_bit(reg,mask,false);
}

void via_chip_reset(void) {
	pci_write_register(addr,base,0x09,0x80);
	if(pci_read_register(addr,base,0x09) & 0x80) {
		kprintf("Reset not completed. Hardreset....\n");
		pci_write_register(addr,base,0x81,0x40);
		//via_wait_bit_low(0x09,0x80);
	}
	//kprintf("Chip reset complete\n");
}

void via_reload_eeprom(void) {
	pci_write_register(addr,base,0x74,0x20);
	//kprintf("Reload EEPROM complete\n");
}

void via_power_init(void) {
	uint16_t wolstat;
	pci_write_register(addr,base,0x83,pci_read_register(addr,base,0x83) & 0xFC);
	pci_write_register(addr,base,0xA7,0x80);
	pci_write_register(addr,base,0xA4,0xFF);
	pci_write_register(addr,base,0xA4,0xFF);
	wolstat = pci_read_register(addr,base,0xA0);
	pci_write_register(addr,base,0xAC,0xFF);
	if(wolstat) {
		kprintf("WOL: ");
		switch(wolstat) {
			case 0x20:
				kprintf("Magic Packet\n");
				break;
			case 0x40:
				kprintf("Link went up\n");
				break;
			case 0x80:
				kprintf("Link went down\n");
				break;
			case 0x10:
				kprintf("Unicast Packet\n");
				break;
			case 0x30:
				kprintf("Multicast/Broadcast Packet\n");
				break;
			default:
				kprintf("Unknown\n");
		}
	}
	//kprintf("Power init complete\n");
}

struct rx_desc {
	uint32_t status;
	uint32_t length;
	uint32_t addr;
	uint32_t next;
};

struct tx_desc {
	uint32_t status;
	uint32_t length;
	uint32_t addr;
	uint32_t next;
};

struct tx_desc* tx1[10];
struct rx_desc* rx1[10];

void start_nic(void) {
	pci_config_write_8(addr,base,0x04,0x07);
	//kprintf("Config: %b\n",pci_config_read_8(addr,base,0x04));
	for(int i=0;i<10;i++) {
		struct rx_desc* rx=(void*)pmm_alloc();
		rx->status=0x80000000;
		rx->length=0x200;
		rx->addr=(uint32_t*)pmm_alloc();
		rx1[i]=rx;
	}
	for(int i=0;i<10;i++) {
		if(i==9) {
			rx1[i]->next=rx1[0];
		} else {
			rx1[i]->next=rx1[i+1];
		}
	}
	for(int i=0;i<10;i++) {
		struct tx_desc* tx=(void*)pmm_alloc();
		tx->status=0;
		tx->length=0x200;
		tx->addr=(uint32_t*)pmm_alloc();
		tx1[i]=tx;
	}
	for(int i=0;i<10;i++) {
		if(i==9) {
			tx1[i]->next=tx1[0];
		} else {
			tx1[i]->next=tx1[i+1];
		}
	}
	pci_bdf_t addr1 = {
		.bus=0,
		.dev=17,
		.func=0
	};	
	pci_config_write_8(addr1,0x51,0x3D);
	//pci_config_write_8(addr1,0x55,0xF0); //INTA == IRQ15
	//pci_config_write_8(addr1,0x55,0xBE); //INTB == IRQ14 / INTC == IRQ11
	//pci_config_write_8(addr1,0x55,0xA0); //INTD == IRQ10
	via_power_init();
	via_chip_reset();
	via_reload_eeprom();
	
	pci_write_register_16(addr,base,0x6E,0x0006);
	//pci_write_register_8(addr,base,0x06,0x50);
	pci_write_register(addr,base,0x07,0xE0); //store & forward //0x20

	pci_write_register_16(addr,base,0x0E,/*RHINE_EVENT & */0xffff);
	pci_write_register(addr,base,0x09,0x04);
	
	uint32_t nic_status = pci_read_register_16(addr,0,0x0c);
	nic_status |= pci_read_register_16(addr,0,0x84) << 16;
	nic_status = nic_status & RHINE_EVENT_SLOW;
	pci_write_register(addr,0,0x84,nic_status >> 16);
	pci_write_register_16(addr,0,0x0c,nic_status);

	// enable linkMon
	pci_write_register(addr,base,0x70,0x0);
	pci_write_register(addr,base,0x71,0x01);
	pci_write_register(addr,base,0x70,0x80);
	via_wait_bit_high(0x71,0x20);
	pci_write_register(addr,base,0x71,0x01 | 0x40);
	
	pci_write_register_32(addr,base,0x18,rx1[0]);
	pci_write_register_32(addr,base,0x1C,tx1[0]);
	
	tx_addr = pmm_alloc();
	pci_write_register_16(addr,base,0x08, 0x7b); //0x02 | 0x10 | 0x08 | (0x08 << 8));
	
	kprintf("Network init complete\n");
}

struct ether_header {
	struct mac receipt_mac;
	struct mac sender_mac;
	uint16_t type;
};

struct ip_header {
  unsigned headerlen : 4; //Vertauscht, da höherwertige Bits zuerst kommen (Big Endian wird nur für die Bytereihenfolge verwendet)
  unsigned version : 4;
  uint8_t  priority;
  uint16_t packetsize;
  uint16_t id;
  uint16_t fragment;
  uint8_t  ttl;
  uint8_t  protocol;
  uint16_t checksum;
  struct ip_addr sourceIP;
  struct ip_addr destinationIP;
  uint8_t *data;
  uint32_t data_length;
} __attribute__((packed));

struct udp_header {
	uint16_t source_port;
	uint16_t destination_port;
	uint16_t packetsize;
	uint16_t checksum;
	uint8_t *data;
};

struct dhcp_packet {
	uint8_t operation;
	uint8_t network_type;
	uint8_t network_addr_length;
	uint8_t relay_agents;
	uint32_t connection_id;
	uint16_t seconds_start;
	uint16_t flags;
	struct ip_addr client_ip;
	struct ip_addr own_ip;
	struct ip_addr server_ip;
	struct ip_addr relay_ip;
	struct mac client_mac;
	uint8_t mac_padding[10];
	uint8_t server_name[64];
	uint8_t file_name[128];
	uint32_t magic_cookie;
};

struct arp {
	uint16_t hardware_addr_type;
	uint16_t network_addr_type;
	uint8_t hardware_addr_length;
	uint8_t network_addr_length;
	uint16_t operation;
	/*uint8_t sender_mac1;
	uint8_t sender_mac2;
	uint8_t sender_mac3;
	uint8_t sender_mac4;
	uint8_t sender_mac5;
	uint8_t sender_mac6;
	uint8_t sender_ip1;
	uint8_t sender_ip2;
	uint8_t sender_ip3;
	uint8_t sender_ip4;
	uint8_t receipt_mac1;
	uint8_t receipt_mac2;
	uint8_t receipt_mac3;
	uint8_t receipt_mac4;
	uint8_t receipt_mac5;
	uint8_t receipt_mac6;
	uint8_t receipt_ip1;
	uint8_t receipt_ip2;
	uint8_t receipt_ip3;
	uint8_t receipt_ip4;*/
	struct mac sender_mac;
	struct ip_addr sender_ip;
	struct mac receipt_mac;
	struct ip_addr receipt_ip;
} __attribute__((packed));

struct icmp_echo_packet {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
    uint16_t echo_id;
    uint16_t echo_seq;
};

union arp_test {
	struct arp arp_val1;
	uint8_t data[sizeof(struct arp)];
};

union ether_test {
	struct ether_header ether_val1;
	uint8_t data[sizeof(struct ether_header)];
};

union ip_union {
	struct ip_header ip;
	uint8_t data[sizeof(struct ip_header)];
};

union icmp_ping_union {
	struct icmp_echo_packet icmp;
	uint8_t data[sizeof(struct icmp_echo_packet)];
};

void via_send(struct ether_header ether, uint8_t data[], int data_length) {
	
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
		if(next_tx >= 10) next_tx=0;

		while(i < 14) {
			((uint8_t*)tx1[next_tx]->addr)[i] = ether_union.data[i];
			//ether++;
			i++;
		}
		while(/*j < 28*/ data_length - j > 0) {
			((uint8_t*)tx1[next_tx]->addr)[i] = data[j];
			//data++;
			i++;
			j++;
		}
		while(j < 46) {
			((uint8_t*)tx1[next_tx]->addr)[i] = 0x0;
			i++;
			j++;
		}
		tx1[next_tx]->length = ((uint32_t)i) | 0x600000;
		tx1[next_tx]->status |= 0x80000000;
		next_tx++;
		//pci_write_register_16(addr,base,0x08,pci_read_register_16(addr,base,0x08) | 0x20); //poll TX
	}
}

/*void ether_header_print(struct ether_header ether) {
	kprintf("Empfaenger-MAC: %x:%x:%x:%x:%x:%x\n",ether.receipt_mac1,ether.receipt_mac2,
													ether.receipt_mac3,ether.receipt_mac4,
													ether.receipt_mac5,ether.receipt_mac6);
													
	kprintf("Sender-MAC: %x:%x:%x:%x:%x:%x\n",ether.sender_mac1,ether.sender_mac2,
												ether.sender_mac3,ether.sender_mac4,
												ether.sender_mac5,ether.sender_mac6);	
}*/

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

void arp(struct arp arp_val, struct ether_header ether) {
	
	struct arp arp_temp;
	
	arp_temp.hardware_addr_type = HTONS(arp_val.hardware_addr_type);
	arp_temp.network_addr_type = HTONS(arp_val.network_addr_type);
	arp_temp.hardware_addr_length = arp_val.hardware_addr_length;
	arp_temp.network_addr_length = arp_val.network_addr_length;
	arp_temp.operation = HTONS(0x0002);
	
	arp_temp.receipt_mac.mac1 = arp_val.sender_mac.mac1;
	arp_temp.receipt_mac.mac2 = arp_val.sender_mac.mac2;
	arp_temp.receipt_mac.mac3 = arp_val.sender_mac.mac3;
	arp_temp.receipt_mac.mac4 = arp_val.sender_mac.mac4;
	arp_temp.receipt_mac.mac5 = arp_val.sender_mac.mac5;
	arp_temp.receipt_mac.mac6 = arp_val.sender_mac.mac6;
	
	arp_temp.receipt_ip.ip1 = arp_val.sender_ip.ip1;
	arp_temp.receipt_ip.ip2 = arp_val.sender_ip.ip2;
	arp_temp.receipt_ip.ip3 = arp_val.sender_ip.ip3;
	arp_temp.receipt_ip.ip4 = arp_val.sender_ip.ip4;
	
	arp_temp.sender_ip.ip1 = my_ip.ip1;
	arp_temp.sender_ip.ip2 = my_ip.ip2;
	arp_temp.sender_ip.ip3 = my_ip.ip3;
	arp_temp.sender_ip.ip4 = my_ip.ip4;
	
	arp_temp.sender_mac.mac1 = my_mac.mac1;
	arp_temp.sender_mac.mac2 = my_mac.mac2;
	arp_temp.sender_mac.mac3 = my_mac.mac3;
	arp_temp.sender_mac.mac4 = my_mac.mac4;
	arp_temp.sender_mac.mac5 = my_mac.mac5;
	arp_temp.sender_mac.mac6 = my_mac.mac6;
	
	union arp_test tester1;
	tester1.arp_val1 = (arp_temp);
	//kprintf("Test: Offset 2: 0x%x\n",arp_val.hardware_addr_length);
	via_send(ether,tester1.data, sizeof(struct arp));
}

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
		
		via_send(ether, buffer1, HTONS(ip1.ip.packetsize));
	}
}

int dhcp_status = 0;

uint32_t connection_id = 0x33224411;

struct dhcp_options {
	uint8_t option;
	uint8_t length;
	uint8_t *data;
};

void dhcp_offer(struct ether_header ether, struct udp_header udp, struct dhcp_packet dhcp) {
	kprintf("DHCP-OFFER...\n");
	
	int udp_length = 8;
	int dhcp_length = 48 + 64 + 128;
	
	static volatile struct dhcp_options options[256];
	
	for(int i=dhcp_length;i<udp.packetsize - udp_length;i++) {
		if(udp.data[i] == 255) break;

		struct dhcp_options option;
		option.option = udp.data[i];
		option.length = udp.data[i + 1];
		
		//kprintf("Option %d Laenge %d: ", udp.data[i], udp.data[i + 1]);
		int j;
		option.data = (void*)pmm_alloc();
		for(j=0;j<udp.data[i + 1];j++) {
			option.data[j] = udp.data[i + 2 + j];
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

void ip_handle(struct ip_header ip, struct ether_header ether) {
	struct ip_header ip_orig = ip;
	/*kprintf("Typ: IP\n");
	kprintf("Version: 0x%x\n",ip.version); //40
	kprintf("Headerlaenge: %d\n",ip.headerlen * 4); //0x14 = 20 (5 * 4Bytes)
	kprintf("Prioritaet: 0x%x\n",ip.priority);
	kprintf("Paketlaenge: 0x%x - %d\n",ip.packetsize,ip.packetsize);
	kprintf("Paket-ID: 0x%x - %d\n",ip.id,ip.id);
	kprintf("Fragment Offset/Flags: 0x%x - %d\n",ip.fragment,ip.fragment);
	kprintf("TTL: 0x%x - %d\n",ip.ttl,ip.ttl);
	kprintf("Protokolltyp: 0x%x\n",ip.protocol);
	kprintf("Pruefsumme: 0x%x - %d\n",ip.checksum,ip.checksum);
	kprintf("Absender-IP: %d.%d.%d.%d\n",ip.sourceIP.ip1
										,ip.sourceIP.ip2
										,ip.sourceIP.ip3
										,ip.sourceIP.ip4);
	kprintf("Ziel-IP: %d.%d.%d.%d\n",ip.destinationIP.ip1
										,ip.destinationIP.ip2
										,ip.destinationIP.ip3
										,ip.destinationIP.ip4);*/
	
	if((ip.destinationIP.ip1 == my_ip.ip1 &&
			ip.destinationIP.ip2 == my_ip.ip2 &&
			ip.destinationIP.ip3 == my_ip.ip3 &&
			ip.destinationIP.ip4 == my_ip.ip4) || (
			ip.destinationIP.ip4 == broadcast_ip.ip4)) {
		if((ip.headerlen * 4) - 20 > 0) { //Optionen vorhanden
			
		}
		
		switch(ip.protocol) {
			case 0x1: //ICMP
				icmp_handle(ip, ether);
				break;
			case 0x6: //TCP
				break;
			case 0x11: //UDP
				udp_handle(ip, ether);
				break;
			default:
				kprintf("Protokoll nicht implementiert: 0%x\n",ip.protocol);
				break;
		}
		
		/*kprintf("Daten: ");
		for(int i=0;i < ip.data_length;i++) {
			kprintf("0x%x ",ip.data[i]);
		}
		kprintf("\n");*/
	}
}

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

void via_handle_intr(void) {
	
	if(init_i == 0) {
	
		if(pci_read_register(addr,base,0x6d) & 0x02) {
			kprintf("Link is down\n");
		} else {
			kprintf("Link is up\n");
			dhcp_get_ip();
		}

		init_i = 1;
		return;
	}
	
	uint32_t nic_status = pci_read_register_16(addr,base,0x0c);
	//if(nic_status & 0x0001) kprintf("Packet received successfull\n");
	//if(nic_status & 0x0002) kprintf("Packet transmitted successfull\n");
	if(nic_status & 0x0004) kprintf("Receive error\n");
	if(nic_status & 0x0008) kprintf("Transmit error (transmit aborted)\n");
	if(nic_status & 0x0010) kprintf("Transmit buffer underflow\n");
	if(nic_status & 0x0020) kprintf("Receive buffer link error\n");
	if(nic_status & 0x0040) kprintf("PCI bus error\n");
	if(nic_status & 0x0080) kprintf("CRC or Miss packet tally counter overflow\n");
	if(nic_status & 0x0100) kprintf("Early receive\n");
	if(nic_status & 0x0200) kprintf("Transmit fifo underflow\n");
	if(nic_status & 0x0400) kprintf("Receive fif overflow\n");
	if(nic_status & 0x0800) kprintf("Receive packet race\n");
	if(nic_status & 0x1000) kprintf("Receive buffer full\n");
	if(nic_status & 0x2000) kprintf("Transmit abort due to excessive collisions\n");
	if(nic_status & 0x4000) {
		//kprintf("Link changed\n");
		if(pci_read_register(addr,base,0x6d) & 0x02) {
			kprintf("Link is down\n");
		} else {
			kprintf("Link is up\n");
			dhcp_get_ip();
		}
	}
	if(nic_status & 0x8000) kprintf("General purpose interrupt\n");
	uint16_t recv_size;
	uint32_t status;
	for(int i=0;i<10;i++) {
		status = rx1[i]->status;
		if((status & 0x80000000) == 0) {
			recv_size = (status & 0xFFFF0000) >> 16;
			int b;
			for(b=0;b < recv_size;b++) {
				uint8_t bytes = ((uint8_t*)rx1[i]->addr)[b];
			}
			
			struct ether_header ether = {
				.receipt_mac.mac1 = ((uint8_t*)rx1[i]->addr)[0],
				.receipt_mac.mac2 = ((uint8_t*)rx1[i]->addr)[1],
				.receipt_mac.mac3 = ((uint8_t*)rx1[i]->addr)[2],
				.receipt_mac.mac4 = ((uint8_t*)rx1[i]->addr)[3],
				.receipt_mac.mac5 = ((uint8_t*)rx1[i]->addr)[4],
				.receipt_mac.mac6 = ((uint8_t*)rx1[i]->addr)[5],
				.sender_mac.mac1 = ((uint8_t*)rx1[i]->addr)[6],
				.sender_mac.mac2 = ((uint8_t*)rx1[i]->addr)[7],
				.sender_mac.mac3 = ((uint8_t*)rx1[i]->addr)[8],
				.sender_mac.mac4 = ((uint8_t*)rx1[i]->addr)[9],
				.sender_mac.mac5 = ((uint8_t*)rx1[i]->addr)[10],
				.sender_mac.mac6 = ((uint8_t*)rx1[i]->addr)[11],
				.type = HTONS(((uint16_t*)rx1[i]->addr)[6])
			};
			//ether_header_print(ether);
			if(ether.type > 0x0600) { //Ethernet II
				switch(ether.type) {
					case 0x0806: //ARP
						kprintf("");
						struct arp arp1 = {
							.hardware_addr_type = HTONS(((uint16_t*)rx1[i]->addr)[7]),
							.network_addr_type = HTONS(((uint16_t*)rx1[i]->addr)[8]),
							.hardware_addr_length = (((uint8_t*)rx1[i]->addr)[18]),
							.network_addr_length = (((uint8_t*)rx1[i]->addr)[19]),
							.operation = HTONS(((uint16_t*)rx1[i]->addr)[10]),
							.sender_mac.mac1 = (((uint8_t*)rx1[i]->addr)[22]),
							.sender_mac.mac2 = (((uint8_t*)rx1[i]->addr)[23]),
							.sender_mac.mac3 = (((uint8_t*)rx1[i]->addr)[24]),
							.sender_mac.mac4 = (((uint8_t*)rx1[i]->addr)[25]),
							.sender_mac.mac5 = (((uint8_t*)rx1[i]->addr)[26]),
							.sender_mac.mac6 = (((uint8_t*)rx1[i]->addr)[27]),
							.sender_ip.ip1 = (((uint8_t*)rx1[i]->addr)[28]),
							.sender_ip.ip2 = (((uint8_t*)rx1[i]->addr)[29]),
							.sender_ip.ip3 = (((uint8_t*)rx1[i]->addr)[30]),
							.sender_ip.ip4 = (((uint8_t*)rx1[i]->addr)[31]),
							.receipt_mac.mac1 = (((uint8_t*)rx1[i]->addr)[32]),
							.receipt_mac.mac2 = (((uint8_t*)rx1[i]->addr)[33]),
							.receipt_mac.mac3 = (((uint8_t*)rx1[i]->addr)[34]),
							.receipt_mac.mac4 = (((uint8_t*)rx1[i]->addr)[35]),
							.receipt_mac.mac5 = (((uint8_t*)rx1[i]->addr)[36]),
							.receipt_mac.mac6 = (((uint8_t*)rx1[i]->addr)[37]),
							.receipt_ip.ip1 = (((uint8_t*)rx1[i]->addr)[38]),
							.receipt_ip.ip2 = (((uint8_t*)rx1[i]->addr)[39]),
							.receipt_ip.ip3 = (((uint8_t*)rx1[i]->addr)[40]),
							.receipt_ip.ip4 = (((uint8_t*)rx1[i]->addr)[41])
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
							.headerlen = (((uint8_t*)rx1[i]->addr)[14]) & 0xF,
							.version = ((uint8_t*)rx1[i]->addr)[14] >> 4,
							.priority = ((uint8_t*)rx1[i]->addr)[15],
							.packetsize = HTONS(((uint16_t*)rx1[i]->addr)[8]),
							.id = HTONS(((uint16_t*)rx1[i]->addr)[9]),
							.fragment = HTONS(((uint16_t*)rx1[i]->addr)[10]),
							.ttl = ((uint8_t*)rx1[i]->addr)[22],
							.protocol = ((uint8_t*)rx1[i]->addr)[23],
							.checksum = HTONS(((uint16_t*)rx1[i]->addr)[12]),
							.sourceIP.ip1 = ((uint8_t*)rx1[i]->addr)[26],
							.sourceIP.ip2 = ((uint8_t*)rx1[i]->addr)[27],
							.sourceIP.ip3 = ((uint8_t*)rx1[i]->addr)[28],
							.sourceIP.ip4 = ((uint8_t*)rx1[i]->addr)[29],
							.destinationIP.ip1 = ((uint8_t*)rx1[i]->addr)[30],
							.destinationIP.ip2 = ((uint8_t*)rx1[i]->addr)[31],
							.destinationIP.ip3 = ((uint8_t*)rx1[i]->addr)[32],
							.destinationIP.ip4 = ((uint8_t*)rx1[i]->addr)[33]
						};
						ip.data_length = (ip.packetsize - (ip.headerlen * 4));
						
						uint8_t *daten;
						for(int z = 0; z < (ip.packetsize - (ip.headerlen * 4)); z++) {
							int offset = (ip.headerlen * 4) + sizeof(struct ether_header) + z;
							ip.data[z] = ((uint8_t*)rx1[i]->addr)[offset];
						}
						ip_handle(ip,ether);
						break;
					default:
						kprintf("Nicht behandeltes Protokoll: %x\n",ether.type);
				}
			} else if(ether.type <= 0x05DC) { //Ethernet I
				kprintf("Ethernet I Pakete werden nicht behandelt\n");
			}
			rx1[i]->status = 0x80000000;
		}
	}
	nic_status |= pci_read_register_16(addr,base,0x84) << 16;
	pci_write_register(addr,base,0x84,nic_status >> 16);
	pci_write_register_16(addr,base,0x0c,nic_status);
}