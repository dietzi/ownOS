#include "includes.h"

void arp(struct arp* arp_val, struct ether_header* ether);

void arp(struct arp* arp_val, struct ether_header* ether) {
	//kprintf("ARP\n");
	struct arp* arp_temp = pmm_alloc();
	kprintf("Receipt-Mac: %x-%x-%x-%x-%x-%x\n",arp_val->receipt_mac.mac1,
												arp_val->receipt_mac.mac2,
												arp_val->receipt_mac.mac3,
												arp_val->receipt_mac.mac4,
												arp_val->receipt_mac.mac5,
												arp_val->receipt_mac.mac6);
	kprintf("Receipt-IP: %x-%x-%x-%x-%x-%x\n",arp_val->receipt_ip.ip1,
												arp_val->receipt_ip.ip2,
												arp_val->receipt_ip.ip3,
												arp_val->receipt_ip.ip4);
	kprintf("Sender-Mac: %x-%x-%x-%x-%x-%x\n",arp_val->sender_mac.mac1,
												arp_val->sender_mac.mac2,
												arp_val->sender_mac.mac3,
												arp_val->sender_mac.mac4,
												arp_val->sender_mac.mac5,
												arp_val->sender_mac.mac6);
	kprintf("Sender-IP: %x-%x-%x-%x-%x-%x\n",arp_val->sender_ip.ip1,
												arp_val->sender_ip.ip2,
												arp_val->sender_ip.ip3,
												arp_val->sender_ip.ip4);
	ether->receipt_mac.mac1 = my_mac.mac1;
	ether->receipt_mac.mac2 = my_mac.mac2;
	ether->receipt_mac.mac3 = my_mac.mac3;
	ether->receipt_mac.mac4 = my_mac.mac4;
	ether->receipt_mac.mac5 = my_mac.mac5;
	ether->receipt_mac.mac6 = my_mac.mac6;
	
	arp_temp->hardware_addr_type = HTONS(arp_val->hardware_addr_type);
	arp_temp->network_addr_type = HTONS(arp_val->network_addr_type);
	arp_temp->hardware_addr_length = arp_val->hardware_addr_length;
	arp_temp->network_addr_length = arp_val->network_addr_length;
	arp_temp->operation = HTONS(0x0002);
	
	arp_temp->receipt_mac.mac1 = arp_val->sender_mac.mac1;
	arp_temp->receipt_mac.mac2 = arp_val->sender_mac.mac2;
	arp_temp->receipt_mac.mac3 = arp_val->sender_mac.mac3;
	arp_temp->receipt_mac.mac4 = arp_val->sender_mac.mac4;
	arp_temp->receipt_mac.mac5 = arp_val->sender_mac.mac5;
	arp_temp->receipt_mac.mac6 = arp_val->sender_mac.mac6;
	
	arp_temp->receipt_ip.ip1 = arp_val->sender_ip.ip1;
	arp_temp->receipt_ip.ip2 = arp_val->sender_ip.ip2;
	arp_temp->receipt_ip.ip3 = arp_val->sender_ip.ip3;
	arp_temp->receipt_ip.ip4 = arp_val->sender_ip.ip4;
	
	arp_temp->sender_ip.ip1 = my_ip.ip1;
	arp_temp->sender_ip.ip2 = my_ip.ip2;
	arp_temp->sender_ip.ip3 = my_ip.ip3;
	arp_temp->sender_ip.ip4 = my_ip.ip4;
	
	arp_temp->sender_mac.mac1 = my_mac.mac1;
	arp_temp->sender_mac.mac2 = my_mac.mac2;
	arp_temp->sender_mac.mac3 = my_mac.mac3;
	arp_temp->sender_mac.mac4 = my_mac.mac4;
	arp_temp->sender_mac.mac5 = my_mac.mac5;
	arp_temp->sender_mac.mac6 = my_mac.mac6;
	
	//union arp_test* tester1 = pmm_alloc();
	//tester1->arp_val1 = (arp_temp);
	//kprintf("Test: Offset 2: 0x%x\n",arp_val.hardware_addr_length);
	sendPacket(ether,(uint8_t*)arp_temp, 26);
	//pmm_free(tester1);
	pmm_free(arp_temp);
}
