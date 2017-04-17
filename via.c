#include "includes.h"

int base=0;

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
	kprintf("Chip reset complete\n");
}

void via_reload_eeprom(void) {
	pci_write_register(addr,base,0x74,0x20);
	kprintf("Reload EEPROM complete\n");
}

void via_power_init(void) {
	uint16_t wolstat;
	pci_write_register(addr,base,0x83,pci_read_register(addr,base,0x83) & 0xFC);
	pci_write_register(addr,base,0xA7,0x80);
	pci_write_register(addr,base,0xA4,0xFF);
	pci_write_register(addr,base,0xA4,0xFF);
	wolstat = pci_read_register(addr,base,0xA8);
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
	kprintf("Power init complete\n");
}

struct rx_desc {
	uint32_t status;
	uint32_t length;
	uint32_t addr;
	uint32_t next;
};

struct rx_desc* rx1[10];

void start_nic(void) {
	pci_config_write_8(addr,base,0x04,0x07);
	kprintf("Config: %b\n",pci_config_read_8(addr,base,0x04));
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
	pci_write_register(addr,base,0x07,0x20);

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
	
	pci_write_register_16(addr,base,0x08,0x02 | 0x10 | 0x08 | (0x08 << 8));
	kprintf("NIC init complete\n");
}

void via_handle_intr(void) {
	uint32_t nic_status = pci_read_register_16(addr,base,0x0c);
	kprintf("NIC_STATUS: %x - %b\n",nic_status,nic_status);
	if(nic_status & 0x0001) kprintf("Packet received successfull\n");
	if(nic_status & 0x0002) kprintf("Packet transmitted successfull\n");
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
	if(nic_status & 0x4000) kprintf("Link changed\n");
	if(nic_status & 0x8000) kprintf("General purpose interrupt\n");
	uint16_t recv_size;
	uint32_t status;
	for(int i=0;i<10;i++) {
		status = rx1[i]->status;
		if((status & 0x80000000) == 0) {
			recv_size = (status & 0xFFFF0000) >> 16;
			for(int b=0;b < recv_size;b++) {
				//kprintf("%d:%d: %x ",i,b,((uint32_t*)rx1[i]->addr)[b]);
			}
			//uint32_t *addr11=(uint32_t*)(rx1[i]->addr);
			//kprintf("%d: %s ",recv_size,addr11);
			rx1[i]->status |= 0x80000000;
		}
	}
	kprintf("\n");
	kprintf("Receive: %b\n",pci_read_register_32(addr,base,0x20));
	//uint32_t nic_status = pci_read_register_16(addr,base,0x0c);
	nic_status |= pci_read_register_16(addr,base,0x84) << 16;
	//nic_status = nic_status & RHINE_EVENT_SLOW;
	//kprintf("NIC_STATUS: %b\n",nic_status);
	pci_write_register(addr,base,0x84,nic_status >> 16);
	pci_write_register_16(addr,base,0x0c,nic_status);
	//pci_write_register_16(addr,base,0x08,0x02 | 0x10 | 0x08 | (0x08 << 8)); //Restart NIC
	//kprintf("NIC_STATUS: %b\n",nic_status);
}