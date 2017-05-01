#include "includes.h"

int base=0;
int next_tx = 0;
void *tx_addr;

int init_i = 0;

pci_bdf_t addr = {
	.bus=0,
	.dev=18,
	.func=0
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
} __attribute__((packed));

struct tx_desc {
	uint32_t status;
	uint32_t length;
	uint32_t addr;
	uint32_t next;
} __attribute__((packed));

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
	pci_write_register(addr,base,0x20,0xE0); //store & forward = 0x07 //0x20

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

void via_send(uint8_t *data, int data_length) {
	/*kprintf("VIA send begin\n");
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
		while(data_length - j > 0) {
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
		kprintf("VIA send: %d\n",i);*/
last_message = "via_send...1";
	int i=0;
	for(i=0;i<data_length;i++) {
		((uint8_t*)tx1[next_tx]->addr)[i] = data[i];
	}
	tx1[next_tx]->length = ((uint32_t)i) | 0x600000;
	tx1[next_tx]->status |= 0x80000000;
	next_tx++;
	pci_write_register_16(addr,base,0x08,pci_read_register_16(addr,base,0x08) | 0x20); //poll TX
last_message = "via_send...2";
	return;
}

void via_handle_intr(void) {
	
	uint32_t nic_status = pci_read_register_16(addr,base,0x0c);
	if(init_i == 0) {
	
		if(pci_read_register(addr,base,0x6d) & 0x02) {
			kprintf("Link is down\n");
		} else {
			kprintf("Link is up\n");
		}

		nic_status |= pci_read_register_16(addr,base,0x84) << 16;
		pci_write_register(addr,base,0x84,nic_status >> 16);
		pci_write_register_16(addr,base,0x0c,nic_status);
		init_i = 1;
		return;
	}
	
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
			dhcp_get_ip();
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
						kprintf("ARP1\n");
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
						kprintf("Receipt-IP: %d.%d.%d.%d\n", arp1.receipt_ip.ip1,
																arp1.receipt_ip.ip2,
																arp1.receipt_ip.ip3,
																arp1.receipt_ip.ip4);
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