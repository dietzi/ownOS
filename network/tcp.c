#include "includes.h"

void sendTCPpacket(struct ether_header ether, struct ip_header ip, struct tcp_header tcp, uint32_t options[], int options_count, uint8_t *data, int data_length);
bool register_tcp_listener(int port, void *callback_pointer);
void sendData(struct tcp_callback cb);
void closeCon(struct tcp_callback cb);

uint32_t last_seq = 0;
uint32_t last_ack = 0;
bool con_est = false;

struct clients {
	uint32_t client_id;
	bool con_est;
	uint32_t last_ack;
	uint32_t last_seq;
	uint32_t fin_seq;
	uint32_t fin_ack;
	struct clients *next;
};

struct listeners {
	struct tcp_callback tcp_listener;
	struct clients *clients;
};

struct listeners listeners[65536];

bool listener_enabled[65536];
struct tcp_callback tcp_listeners[65536][51];

void (*callback_func)(struct tcp_callback);

void find_client(uint32_t client_id, uint16_t port) {
	struct clients *client = listeners[port].clients;
}

void tcp_handle(struct ip_header ip, struct ether_header ether) {
	struct tcp_header tcp;
	union tcpU {
		struct tcp_header tcp;
		uint8_t data[20];
	};
	union tcpU tcpU;
	for(int i=0;i<20/*ip.data_length*/;i++) {
		tcpU.data[i] = ip.data[i];
	}
	tcp = tcpU.tcp;
	for(int i=20;i<(tcp.headerlen * 4);i++) {
		tcp.options[i - 20] = ip.data[i];
	}
	uint8_t *tcp_data;
	for(int i=(tcp.headerlen * 4);i< (tcp.headerlen * 4) + (ip.packetsize - (ip.headerlen * 4) - (tcp.headerlen * 4));i++) {
		tcp_data[i - (tcp.headerlen * 4)] = ip.data[i];
	}
	
	uint16_t temp_port = tcp.destination_port;
	tcp.destination_port = tcp.source_port;
	tcp.source_port = temp_port;

	ip.destinationIP = ip.sourceIP;
	ip.sourceIP = my_ip;
	ip.fragment = HTONS(ip.fragment);
	ip.id = HTONS(ip.id);
	
	uint32_t socketID = (ip.sourceIP.ip1) + (ip.sourceIP.ip2 * 2) + (ip.sourceIP.ip3 * 3) + (ip.sourceIP.ip4 * 4) + (HTONS(tcp.destination_port) * 5);
	
	if(listener_enabled[HTONS(temp_port)]) {
		tcp_listeners[HTONS(temp_port)][socketID].data = tcp_data;
		tcp_listeners[HTONS(temp_port)][socketID].data_length = ip.packetsize - (ip.headerlen * 4) - (tcp.headerlen * 4);
		tcp_listeners[HTONS(temp_port)][socketID].tcp = tcp;
		tcp_listeners[HTONS(temp_port)][socketID].ip = ip;
		tcp_listeners[HTONS(temp_port)][socketID].ether = ether;
		
		/*if(tcp.flags.ack) {
			kprintf("ACK: %d - %d\n",tcp.ack_number,(tcp_listeners[HTONS(temp_port)][socketID].fin_seq + 1));
			kprintf("SEQ: %d - %d\n",tcp.sequence_number,(tcp_listeners[HTONS(temp_port)][socketID].fin_ack));
		}*/
		
		if(tcp.flags.fin && tcp.flags.ack &&
					tcp.ack_number != HTONL(tcp_listeners[HTONS(temp_port)][socketID].fin_seq + 1) &&
					tcp.sequence_number != HTONL(tcp_listeners[HTONS(temp_port)][socketID].fin_ack)) {
			tcp.flags.fin = 1;
			tcp.flags.ack = 1;
			uint32_t ack_temp = tcp.ack_number;
			tcp.ack_number = HTONL(HTONL(tcp.sequence_number) + 1);
			tcp.sequence_number = ack_temp;
			tcp_listeners[HTONS(temp_port)][socketID].fin_seq = HTONL(tcp.sequence_number);
			tcp_listeners[HTONS(temp_port)][socketID].fin_ack = HTONL(tcp.ack_number);
			sendTCPpacket(ether, ip, tcp, tcp.options, 0, tcp.data, 0);
		} else if((!tcp.flags.fin && tcp.flags.ack &&
					tcp.ack_number == HTONL(tcp_listeners[HTONS(temp_port)][socketID].fin_seq + 1) &&
					tcp.sequence_number == HTONL(tcp_listeners[HTONS(temp_port)][socketID].fin_ack)) || tcp.flags.rst) {
			tcp_listeners[HTONS(temp_port)][socketID].fin_seq = 0;
			tcp_listeners[HTONS(temp_port)][socketID].fin_ack = 0;
			uint32_t ack_temp = tcp.ack_number;
			tcp.ack_number = HTONL(HTONL(tcp.sequence_number) + 1);
			tcp.sequence_number = ack_temp;
			sendTCPpacket(ether, ip, tcp, tcp.options, 0, tcp.data, 0);
			tcp_listeners[HTONS(temp_port)][socketID].con_est = false;
		} else {
			if(tcp_listeners[HTONS(temp_port)][socketID].con_est) { //connection established
				if(tcp.flags.ack && tcp.flags.psh) { //got packet
					//ACK received packet
					tcp.flags.psh = 0;
					uint32_t ack_temp = tcp.ack_number;
					tcp.ack_number = HTONL(HTONL(tcp.sequence_number) + tcp_listeners[HTONS(temp_port)][socketID].data_length);
					tcp.sequence_number = ack_temp;
					tcp_listeners[HTONS(temp_port)][socketID].last_seq = HTONL(tcp.sequence_number);
					tcp_listeners[HTONS(temp_port)][socketID].last_ack = HTONL(tcp.ack_number);
					sendTCPpacket(ether, ip, tcp, tcp.options, 0, tcp_listeners[HTONS(temp_port)][socketID].data, 0);
					
					callback_func = tcp_listeners[HTONS(temp_port)][socketID].callback_pointer;
					callback_func(tcp_listeners[HTONS(temp_port)][socketID]);
				}
			} else { //no connection
				if(tcp.flags.syn && !tcp.flags.ack) { //asking for connection
					tcp.flags.syn = 1;
					tcp.flags.ack = 1;
					tcp.ack_number = HTONL(HTONL(tcp.sequence_number) + 1);
					tcp.sequence_number = HTONL(tcp.sequence_number);
					tcp_listeners[HTONS(temp_port)][socketID].last_seq = HTONL(tcp.sequence_number);
					tcp_listeners[HTONS(temp_port)][socketID].last_ack = HTONL(tcp.ack_number);
					sendTCPpacket(ether, ip, tcp, tcp.options, 0, tcp.data, 0);
				}
				if(!tcp.flags.syn && tcp.flags.ack) { //ack connection
					if(tcp_listeners[HTONS(temp_port)][socketID].last_ack == HTONL(tcp.sequence_number) && HTONL(tcp.ack_number) == tcp_listeners[HTONS(temp_port)][socketID].last_seq + 1) {
						tcp_listeners[HTONS(temp_port)][socketID].con_est = true;
						tcp_data[0] = 0xff;
						tcp_data[1] = 0xff;
						tcp_data[2] = 0xff;
						tcp_listeners[HTONS(temp_port)][socketID].data = tcp_data;
						tcp_listeners[HTONS(temp_port)][socketID].data_length = 3;
						callback_func = tcp_listeners[HTONS(temp_port)][socketID].callback_pointer;
						callback_func(tcp_listeners[HTONS(temp_port)][socketID]);
					}
				}
			}
		}
	}
}

bool register_tcp_listener(int port, void *callback_pointer) {
	if(listener_enabled[port] == true) {
		return false;
	} else {
		listener_enabled[port] = true;
		for(int i=0;i<330226;i++) {
			struct tcp_callback cb = {
				.callback_pointer = callback_pointer,
				.port = port,
				.con_est = false,
				.enabled = true,
			};
			if(cb.callback_pointer == NULL) {
				kprintf("Callback not set\n");
				return false;
			}
			tcp_listeners[port][i] = cb;
			tcp_listeners[port][i].enabled = true;
		}
		return true;
	}
}

void closeCon(struct tcp_callback cb) {
	if(tcp_listeners[cb.port][cb.socketID].enabled && tcp_listeners[cb.port][cb.socketID].con_est) {
		sleep(1000);
		uint32_t ack_temp = cb.tcp.sequence_number;
		uint32_t seq_temp = HTONL(HTONL(cb.tcp.ack_number) + cb.data_length);
		cb.tcp.sequence_number = seq_temp;
		cb.tcp.ack_number = ack_temp;
		cb.tcp.flags.ack = 1;
		cb.tcp.flags.psh = 0;
		cb.tcp.flags.rst = 0;
		cb.tcp.flags.syn = 0;
		cb.tcp.flags.fin = 1;
		cb.tcp.flags.urg = 0;
		cb.tcp.flags.ece = 0;
		cb.tcp.flags.cwr = 0;
		cb.fin_ack = HTONL(ack_temp) + 2;
		cb.fin_seq = HTONL(seq_temp);
		tcp_listeners[cb.port][cb.socketID].fin_ack = cb.fin_ack;
		tcp_listeners[cb.port][cb.socketID].fin_seq = cb.fin_seq;
		sendTCPpacket(cb.ether, cb.ip, cb.tcp, cb.tcp.options, 0, cb.data, 0);
	}	
}

void sendData(struct tcp_callback cb) {
	if(tcp_listeners[cb.port][cb.socketID].enabled && tcp_listeners[cb.port][cb.socketID].con_est) {
		uint32_t ack_temp = cb.tcp.sequence_number;
		uint32_t seq_temp = HTONL(HTONL(cb.tcp.ack_number));
		cb.tcp.sequence_number = seq_temp;
		cb.tcp.ack_number = ack_temp;
		cb.tcp.flags.ack = 1;
		cb.tcp.flags.psh = 1;
		cb.tcp.flags.rst = 0;
		cb.tcp.flags.syn = 0;
		cb.tcp.flags.fin = 0;
		cb.tcp.flags.urg = 0;
		cb.tcp.flags.ece = 0;
		cb.tcp.flags.cwr = 0;
		sendTCPpacket(cb.ether, cb.ip, cb.tcp, cb.tcp.options, 0, cb.data, cb.data_length);
	}
}

void sendTCPpacket(struct ether_header ether, struct ip_header ip, struct tcp_header tcp, uint32_t options[], int options_count, uint8_t *data, int data_length) {
	int packetsize = 20 + 20 + options_count + data_length;
	int pos = 0;
	int pos1 = 0;
	uint8_t *temp;
	
	ip.checksum = 0;
	ip.packetsize = HTONS((uint16_t)packetsize);
	ip.headerlen = 5;
	ip.version = 4;
	tcp.checksum = 0;
	tcp.headerlen = (packetsize - data_length - 20) / 4;
	
	ip.checksum = HTONS(checksum(&ip,20));
	
	struct tcp_pseudo_header head = {
		.sourceIP = ip.sourceIP,
		.destinationIP = ip.destinationIP,
		.protocol = 6,
		.tcp_length = HTONS((uint16_t)packetsize - 20)
	};
	
	uint8_t tcpChecksum[12 + packetsize - 20];
	temp = &head;
	for(int i = 0; i < 12; i++) {
		tcpChecksum[pos1] = temp[i];
		pos1++;
	}
	temp = &tcp;
	for(int i = 0; i < 20; i++) { //tcp_header
		tcpChecksum[pos1] = temp[i];
		pos1++;
	}
	temp = &options;
	for(int i = 0; i < options_count; i++) { //tcp_options
		tcpChecksum[pos1] = temp[i];
		pos1++;
	}
	//temp = &data;
	for(int i = 0; i < data_length; i++) { //tcp_data
		tcpChecksum[pos1] = data[i];
		pos1++;
	}
	//pos1--;
	tcp.checksum = HTONS(checksum(&tcpChecksum, pos1));
	
	uint8_t buffer[packetsize];
	temp = &ip;
	for(int i = 0; i < 20; i++) { //ip_header
		buffer[pos] = temp[i];
		pos++;
	}
	temp = &tcp;
	for(int i = 0; i < 20; i++) { //tcp_header
		buffer[pos] = temp[i];
		pos++;
	}
	temp = &options;
	for(int i = 0; i < options_count; i++) { //tcp_options
		buffer[pos] = temp[i];
		pos++;
	}
	//temp = &data;
	for(int i = 0; i < data_length; i++) { //tcp_data
		buffer[pos] = data[i];
		pos++;
	}
	//pos--;
	sendPacket(ether,buffer,pos);
}