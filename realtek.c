#include "includes.h"

pci_bdf_t addr;
uint8_t irq = 0;

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
	kprintf("Status: %b\n",pci_read_register_16(addr,0,0x3E));	
	pci_write_register_16(addr,0,0x3E,pci_read_register_16(addr,0,0x3E));
}