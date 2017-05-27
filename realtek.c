#include "includes.h"

void realtek_init(pci_bdf_t device) {
	kprintf("Realtek...\n");
	pci_config_bar_analyze(device);
}