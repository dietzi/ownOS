#include "includes.h"

void sendPacket(struct ether_header ether, uint8_t data[], int data_length);

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

void sendPacket(struct ether_header ether, uint8_t *data, int data_length) {
	uint8_t buffer[data_length + 20];
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

		while(i < 14) {
			buffer[i] = ether_union.data[i];
			i++;
		}
		while(data_length - j > 0) {
			buffer[i] = data[j];
			i++;
			j++;
		}
		while(j < 46) {
			buffer[i] = 0x0;
			i++;
			j++;
		}
last_message = "via_send...";
		via_send(buffer,i);
	}
}