#include "includes.h"

pci_bdf_t addr;
uint8_t irq = 0;

struct rx_desc {
	int own : 1; // if 1 owned by nic / else owned by host
	int eor : 1; // if set end of ring
	uint16_t reserved : 16;
	uint16_t buffer_size : 14;
	uint32_t vlan;
	uint32_t addr_low;
	uint32_t addr_high;
};

struct rx_desc_status {
	int own : 1; // if 1 owned by nic / else owned by host
	int eor : 1; // if set end of ring
	int fs : 1; // if set this is the first segment of packet
	int ls : 1; // if set this is the last segment of packet
	int mar : 1; // multicast packet received
	int pam : 1; // physical packet received
	int bar : 1; // broadcast packet received
	int reserved : 2; // always 0x01
	int rwt : 1; // packet bigger than 8192 bytes
	int res : 1; // if set and ls=1 -> error (crc,runt,rwt,fae)
	int runt : 1; // packet smaller than 64 bytes
	int crc : 1; // if set -> crc-error
	int pid : 2; // protocol-ID:
				/*
				00 = IP
				01 = TCP/IP
				10 = UDP/IP
				11 = IP
				*/
	int ipf : 1; // if set -> ip checksum failure
	int udpf : 1; // if set -> udp checksum failure
	int tcpf : 1; // if set -> tcp checksum failure
	uint16_t frame_length : 14; // if own = 0 and ls = 1 -> packet length incl. crc in bytes
	uint32_t vlan;
	uint32_t addr_low;
	uint32_t addr_high;
};

struct tx_desc {
	int own : 1; // if 1 owned by nic / else owned by host
	int eor : 1; // if set end of ring
	int fs : 1; // if set this is the first segment of packet
	int ls : 1; // if set this is the last segment of packet
	int lgsen : 1; // Large-send
	int reserved : 8; // Reserved
	int ipcs : 1; // if set -> auto checksum
	int udpcs : 1; // if set -> auto checksum
	int tcpcs : 1; // if set -> auto checksum
	uint16_t frame_length : 16; // if own = 0 and ls = 1 -> packet length incl. crc in bytes
	uint32_t vlan;
	uint32_t addr_low;
	uint32_t addr_high;
};

struct rx_desc* rx_descs[10];
struct tx_desc* tx_descs[10];
uint8_t *rx_buf;
uint8_t *tx_buf;

void realtek_init(pci_bdf_t device) {
	addr = device;
	kprintf("Realtek...\n");
	irq = pci_config_read_8(addr,0x3C);
	kprintf("Registerig IRQ %d\n",irq);
	rx_buf = pmm_alloc();
	tx_buf = pmm_alloc();
	for(int i = 0; i < 10; i++) {
		rx_descs[i] = pmm_alloc();
		rx_descs[i]->own = 1;
		rx_descs[i]->eor = 0;
		rx_descs[i]->buffer_size = 0x1000;
		rx_descs[i]->addr_low = rx_buf;
		
		tx_descs[i] = pmm_alloc();
		tx_descs[i]->own = 0;
		tx_descs[i]->eor = 0;
		tx_descs[i]->fs = 1;
		tx_descs[i]->ipcs = 1;
		tx_descs[i]->udpcs = 1;
		tx_descs[i]->tcpcs = 1;
		tx_descs[i]->frame_length = 0x1000;
		tx_descs[i]->addr_low = tx_buf;
}
	rx_descs[9]->addr_high = 0;
	rx_descs[9]->eor = 1;
	tx_descs[9]->addr_high = 0;
	tx_descs[9]->eor = 1;
	
	pci_write_register_16(addr,0,0x3E,pci_read_register_16(addr,0,0x3E)); //Status zurücksetzen
	
	pci_write_register_32(addr,0,0x44,0x0000E70F);
	pci_write_register_8(addr,0,0x37,0x04); // Enable TX
	pci_write_register_32(addr,0,0x40,0x03000700);
	pci_write_register_16(addr,0,0xDA,0x0FFF); // Maximal 8kb-Pakete
	pci_write_register_8(addr,0,0xEC,0x3F); // No early transmit
	
	pci_write_register_32(addr,0,0x20,tx_descs);
	pci_write_register_32(addr,0,0xE4,rx_descs);
	
	pci_write_register_16(addr,0,0x3C,0x43FF); //Activating all Interrupts
	pci_write_register_8(addr,0,0x37,0x0C); // Enabling receive and transmit
	//pci_write_register_16(addr,0,0xE0,0x0);
	//pci_write_register_32(addr,0,0xE8,descs[0]->addr_high);
	
	kprintf("MAC: %x-",pci_read_register_8(addr,0,0x00));
	kprintf("%x-",pci_read_register_8(addr,0,0x01));
	kprintf("%x-",pci_read_register_8(addr,0,0x02));
	kprintf("%x-",pci_read_register_8(addr,0,0x03));
	kprintf("%x-",pci_read_register_8(addr,0,0x04));
	kprintf("%x\n",pci_read_register_8(addr,0,0x05));
	//kprintf("0x00E4: 0x%x - 0x%x\n",pci_read_register_32(addr,0,0xE4),pci_read_register_32(addr,0,0xE8));
	//kprintf("0x0020: 0x%x - 0x%x\n",pci_read_register_32(addr,0,0x20),pci_read_register_32(addr,0,0x24));
	//realtek_handle_intr();
	//sleep(1000);
	kprintf("Sending Test-Packet\n");
	realtek_send_packet();
}

void realtek_send_packet(void) {
	tx_descs[0]->fs = 1;
	tx_descs[0]->ls = 1;
	tx_descs[0]->own = 1;
	tx_descs[0]->frame_length = 1;
	pci_write_register_8(addr,0,0x38,0x40);
}

bool printed = false;
bool printed1 = false;

void realtek_handle_intr(void) {
	uint16_t status = pci_read_register_16(addr,0,0x3E);
	//kprintf("Status: %b\n",status);
	if(status & 0x0001) kprintf("Receive succesfull\n");
	if(status & 0x0002) kprintf("Receive error\n");
	if(status & 0x0004) kprintf("Transmit succesfull\n");
	if(status & 0x0008) kprintf("Transmit error\n");
	if(status & 0x0010) {
		if(printed == false) {
			kprintf("Receive descriptor unavailable\n");
			printed = true;
		}
	}
	if(status & 0x0020) {
		if(pci_read_register_8(addr,0,0x6C) & 0x02) {
			kprintf("Link is up with ");
			if(pci_read_register_8(addr,0,0x6C) & 0x04) kprintf("10 Mbps and ");
			if(pci_read_register_8(addr,0,0x6C) & 0x08) kprintf("100 Mbps and ");
			if(pci_read_register_8(addr,0,0x6C) & 0x10) kprintf("1000 Mbps and ");
			if(pci_read_register_8(addr,0,0x6C) & 0x01) kprintf("Full-duplex\n");
			else kprintf("Half-duplex\n");
		} else {
			kprintf("Link is down\n");
		}
	}
	if(status & 0x0040) {
		if(printed1 == false) {
			kprintf("Receive FIFO overflow\n");
			printed1 = true;
		}
	}
	if(status & 0x0080) kprintf("Transmit descriptor unavailable\n");
	if(status & 0x0100) kprintf("Software Interrupt\n");
	if(status & 0x0200) kprintf("Receive FIFO empty\n");
	if(status & 0x0400) kprintf("Unknown Status (reserved Bit 11)\n");
	if(status & 0x0800) kprintf("Unknown Status (reserved Bit 12)\n");
	if(status & 0x1000) kprintf("Unknown Status (reserved Bit 13)\n");
	if(status & 0x2000) kprintf("Unknown Status (reserved Bit 14)\n");
	if(status & 0x4000) kprintf("Timeout\n");
	if(status & 0x8000) kprintf("Unknown Status (reserved Bit 16)\n");
	
	if(status & 0x0001) {
		
	}
	
	pci_write_register_16(addr,0,0x3E,pci_read_register_16(addr,0,0x3E));
}