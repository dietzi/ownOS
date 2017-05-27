#include "includes.h"

pci_bdf_t addr;
uint8_t irq = 0;

struct rx_desc {
	int own : 1;
	int eor : 1;
	uint16_t reserved : 14;
};

void realtek_init(pci_bdf_t device) {
	addr = device;
	kprintf("Realtek...\n");
	irq = pci_config_read_8(addr,0x3C);
	kprintf("Registerig IRQ %d\n",irq);
	kprintf("MAC: %x-",pci_read_register_8(addr,0,0x00));
	kprintf("%x-",pci_read_register_8(addr,0,0x01));
	kprintf("%x-",pci_read_register_8(addr,0,0x02));
	kprintf("%x-",pci_read_register_8(addr,0,0x03));
	kprintf("%x-",pci_read_register_8(addr,0,0x04));
	kprintf("%x\n",pci_read_register_8(addr,0,0x05));
	pci_write_register_16(addr,0,0x3E,pci_read_register_16(addr,0,0x3E)); //Status zurücksetzen
	pci_write_register_16(addr,0,0x3C,0x43FF); //Activating all Interrupts
	//pci_write_register_16(addr,0,0x3C,0x20); //Nur Link-Change überwachen
}

void realtek_handle_intr(void) {
	uint16_t status = pci_read_register_16(addr,0,0x3E);
	kprintf("Status: %b\n",status);
	if(status & 0x0001) kprintf("Receive succesfull\n");
	if(status & 0x0002) kprintf("Receive error\n");
	if(status & 0x0004) kprintf("Transmit succesfull\n");
	if(status & 0x0008) kprintf("Transmit error\n");
	if(status & 0x0010) kprintf("Receive descriptor unavailable\n");
	if(status & 0x0020) kprintf("Link changed\n");
	if(status & 0x0040) kprintf("Receive FIFO overflow\n");
	if(status & 0x0080) kprintf("Transmit descriptor unavailable\n");
	if(status & 0x0100) kprintf("Software Interrupt\n");
	if(status & 0x0200) kprintf("Receive FIFO empty\n");
	if(status & 0x0400) kprintf("Unknown Status (reserved Bit 11)\n");
	if(status & 0x0800) kprintf("Unknown Status (reserved Bit 12)\n");
	if(status & 0x1000) kprintf("Unknown Status (reserved Bit 13)\n");
	if(status & 0x2000) kprintf("Unknown Status (reserved Bit 14)\n");
	if(status & 0x4000) kprintf("Timeout\n");
	if(status & 0x8000) kprintf("Unknown Status (reserved Bit 16)\n");
	pci_write_register_16(addr,0,0x3E,pci_read_register_16(addr,0,0x3E));
}