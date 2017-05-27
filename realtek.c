#include "includes.h"

void realtek_init(pci_bdf_t device) {
	kprintf("Realtek...\n");
	kprintf("MAC: %x-",pci_read_register_8(device,0,0x00));
	kprintf("%x-",pci_read_register_8(device,0,0x01));
	kprintf("%x-",pci_read_register_8(device,0,0x02));
	kprintf("%x-",pci_read_register_8(device,0,0x03));
	kprintf("%x-",pci_read_register_8(device,0,0x04));
	kprintf("%x\n",pci_read_register_8(device,0,0x05));
}