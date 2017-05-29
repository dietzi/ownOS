#ifndef REALTEK_H
#define REALTEK_H

void realtek_init(pci_bdf_t device);
void realtek_handle_intr(void);
void realtek_send_packet(uint8_t *data, int data_length);

#endif