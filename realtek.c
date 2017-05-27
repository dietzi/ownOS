#include "includes.h"

pci_bdf_t addr;

void realtek_init(pci_bdf_t device) {
	addr = device;
	kprintf("Realtek...\n");
	kprintf("MAC: %x-",pci_read_register_8(addr,0,0x00));
	kprintf("%x-",pci_read_register_8(addr,0,0x01));
	kprintf("%x-",pci_read_register_8(addr,0,0x02));
	kprintf("%x-",pci_read_register_8(addr,0,0x03));
	kprintf("%x-",pci_read_register_8(addr,0,0x04));
	kprintf("%x\n",pci_read_register_8(addr,0,0x05));
	kprintf("IRL: %b\n",pci_config_read_8(addr,0x3C));
	//pci_write_register_16(addr,0,0x3C,0x43FF);
	kprintf("Status: %b\n",pci_read_register_16(addr,0,0x3E));
}