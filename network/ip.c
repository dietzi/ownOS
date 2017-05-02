#include "includes.h"

void ip_handle(struct ip_header ip, struct ether_header ether) {
	struct ip_header ip_orig = ip;
	/*kprintf("Typ: IP\n");
	kprintf("Version: 0x%x\n",ip.version); //40
	kprintf("Headerlaenge: %d\n",ip.headerlen * 4); //0x14 = 20 (5 * 4Bytes)
	kprintf("Prioritaet: 0x%x\n",ip.priority);
	kprintf("Paketlaenge: 0x%x - %d\n",ip.packetsize,ip.packetsize);
	kprintf("Paket-ID: 0x%x - %d\n",ip.id,ip.id);
	kprintf("Fragment Offset/Flags: 0x%x - %d\n",ip.fragment,ip.fragment);
	kprintf("TTL: 0x%x - %d\n",ip.ttl,ip.ttl);
	kprintf("Protokolltyp: 0x%x\n",ip.protocol);
	kprintf("Pruefsumme: 0x%x - %d\n",ip.checksum,ip.checksum);
	kprintf("Absender-IP: %d.%d.%d.%d\n",ip.sourceIP.ip1
										,ip.sourceIP.ip2
										,ip.sourceIP.ip3
										,ip.sourceIP.ip4);
	kprintf("Ziel-IP: %d.%d.%d.%d\n",ip.destinationIP.ip1
										,ip.destinationIP.ip2
										,ip.destinationIP.ip3
										,ip.destinationIP.ip4);*/
	
	if((ip.destinationIP.ip1 == my_ip.ip1 &&
			ip.destinationIP.ip2 == my_ip.ip2 &&
			ip.destinationIP.ip3 == my_ip.ip3 &&
			ip.destinationIP.ip4 == my_ip.ip4) || (
			ip.destinationIP.ip4 == broadcast_ip.ip4)) {
		if((ip.headerlen * 4) - 20 > 0) { //Optionen vorhanden
			
		}
		
		switch(ip.protocol) {
			case 0x1: //ICMP
				icmp_handle(ip, ether);
				break;
			case 0x6: //TCP
				tcp_handle(ip, ether);
				break;
			case 0x11: //UDP
				udp_handle(ip, ether);
				break;
			default:
				kprintf("Protokoll nicht implementiert: 0%x\n",ip.protocol);
				break;
		}
		
		/*kprintf("Daten: ");
		for(int i=0;i < ip.data_length;i++) {
			kprintf("0x%x ",ip.data[i]);
		}
		kprintf("\n");*/
	}
}
