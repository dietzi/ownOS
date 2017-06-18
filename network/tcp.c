#include "includes.h"

/*
TODO:
	-> Timeout-Timer
*/

void sendTCPpacket(struct ether_header* ether, struct ip_header* ip, struct tcp_header* tcp, uint32_t options[], int options_count, uint8_t *data, int data_length);
bool register_tcp_listener(int port, void *callback_pointer);
void sendData(struct tcp_callback cb);
void closeCon(struct tcp_callback cb);
bool tcp_open_con(int port, void *callback_pointer);
bool check_tcp_flags(struct tcp_flags flags, uint8_t f);

uint32_t last_seq = 0;
uint32_t last_ack = 0;
bool con_est = false;

struct tcp_timer_args {
	uint8_t retry;
	uint32_t client_id;
	uint16_t port;
	struct ether_header* ether;
	struct ip_header* ip;
	struct tcp_header* tcp;
	struct timer* timer;
	struct tcp_timer_args* next;
};

struct tcp_timer_args* temp_args;

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
	struct clients **clients;
};

struct listeners listeners[65536];

struct server_con {
	int remote_port;
	bool con_est;
	bool in_use;
	uint32_t last_ack;
	uint32_t last_seq;
	uint32_t fin_seq;
	uint32_t fin_ack;
};

struct server_con connections[65536];

//bool listener_enabled[65536];
//struct tcp_callback tcp_listeners[65536][51];

void (*callback_func)(struct tcp_callback);

struct clients *find_client(uint32_t client_id, uint16_t port) {
	struct clients *client = listeners[port].clients;
	if(client == NULL) return NULL;
	while(client != NULL) {
		if(client->client_id == client_id) return client;
		client = client->next;
	}
	return NULL;
}

void show_clients(uint16_t port) {
	struct clients *client1 = listeners[port].clients;
	while(client1 != NULL) {
		kprintf("0x%x -> ",client1->client_id);
		client1 = client1->next;
	}
	kprintf("\n");
}

struct clients *add_client(uint32_t client_id, uint16_t port) {
	struct clients *client;
	client = pmm_alloc();
	client->client_id = client_id;
	client->con_est = false;
	client->last_ack = 0;
	client->last_seq = 0;
	client->fin_ack = 0;
	client->fin_seq = 0;
	client->next = NULL;
	struct clients *client1 = listeners[port].clients;
	if(client1 == NULL) {
		listeners[port].clients = client;
		return client;
	}
	while(client1->next != NULL) {
		client1 = client1->next;
	}
	client1->next = client;
	//show_clients(port);
	return client;
}

bool del_client(uint32_t client_id, uint16_t port) {
	struct clients *client = listeners[port].clients;
	if(client->client_id == client_id) {
		struct clients *client_temp = client;
		listeners[port].clients = client->next;
		pmm_free(client_temp);
		kprintf("Deleted: 0x%x\n",client_temp->client_id);
		return true;		
	}
	while(client->next != NULL) {
		if(client->next->client_id == client_id) {
			struct clients *client_temp = client->next;
			client->next = client->next->next;
			pmm_free(client_temp);
			kprintf("Deleted 1: 0x%x\n",client_temp->client_id);
			return true;			
		}
	}
	return false;
}

bool check_tcp_flags(struct tcp_flags flags, uint8_t f) {
	uint8_t flags1;
	flags1 |= flags.fin << 0;
	flags1 |= flags.syn << 1;
	flags1 |= flags.rst << 2;
	flags1 |= flags.psh << 3;
	flags1 |= flags.ack << 4;
	flags1 |= flags.urg << 5;
	flags1 |= flags.ece << 6;
	flags1 |= flags.cwr << 7;
	if((flags1 & f) == f) {
		return true;
	} else {
		return false;
	}
}

uint8_t convert_flags(struct tcp_flags flags) {
	uint8_t flags1;
	flags1 |= flags.fin << 0;
	flags1 |= flags.syn << 1;
	flags1 |= flags.rst << 2;
	flags1 |= flags.psh << 3;
	flags1 |= flags.ack << 4;
	flags1 |= flags.urg << 5;
	flags1 |= flags.ece << 6;
	flags1 |= flags.cwr << 7;
	return flags1;
}

void retry_send(void* arguments) {
	kprintf("Retrying...\n");
	last_message = "def";
	struct tcp_timer_args* args_temp = temp_args;
	struct tcp_timer_args* prev_args;
	while(args_temp != NULL) {
		if(args_temp == arguments) {
			kprintf("tcp.c: 162\n");
			if(args_temp->retry >= 2) {
				kprintf("tcp.c: 164\n");
				uint32_t socketID = (args_temp->ip->sourceIP.ip1) +
									(args_temp->ip->sourceIP.ip2) +
									(args_temp->ip->sourceIP.ip3) +
									(args_temp->ip->sourceIP.ip4) +
									(HTONS(args_temp->tcp->destination_port)) +
									checksum(args_temp->ip->sourceIP,4) +
									checksum(args_temp->tcp->destination_port,2);
				struct clients *client = find_client(socketID,HTONS(args_temp->tcp->destination_port));
				if(del_client(client->client_id, HTONS(args_temp->tcp->destination_port))) {
					kprintf("Delete OK\n");
				} else {
					kprintf("Delete Error\n");
					kprintf("Client-ID: 0x%x\n", client->client_id);
					kprintf("Dest-Port: %d\n", HTONS(args_temp->tcp->destination_port));
					show_clients(HTONS(args_temp->tcp->destination_port));
					sleep(100);
				}
				kprintf("tcp.c: 172\n");
				kprintf("Trying to unregister Timer at 0x%x\n",args_temp->timer);
				if(unregister_timer(args_temp->timer)) {
					kprintf("Unregister OK\n");
				} else {
					kprintf("Unregister Error\n");
					show_timers();
				}
				if(args_temp == temp_args) {
					temp_args = temp_args->next;
				} else {
					struct tcp_timer_args* temp1 = temp_args;
					while(temp1->next != NULL) {
						if(temp1->next == args_temp) {
							temp1->next = temp1->next->next;
							break;
						}
						temp1 = temp1->next;
					}
				}
			} else {
				//set ack and seq number
				sendTCPpacket(args_temp->ether, args_temp->ip, args_temp->tcp, args_temp->tcp->options, 0, args_temp->tcp->data, 0);
				args_temp->retry++;
			}
		}
		prev_args = args_temp;
		args_temp = args_temp->next;
	}
	kprintf("Retry end\n");
}

int con_id = 0;

void tcp_handle(struct ip_header* ip, struct ether_header* ether) {
	struct tcp_header* tcp = pmm_alloc();
	for(int i=0;i<20/*ip->data_length*/;i++) {
		((uint8_t*)tcp)[i] = ip->data[i];
	}
	for(int i=20;i<(tcp->headerlen * 4);i++) {
		tcp->options[i - 20] = ip->data[i];
	}
	uint8_t *tcp_data = pmm_alloc();
	for(int i=(tcp->headerlen * 4);i< (tcp->headerlen * 4) + (ip->packetsize - (ip->headerlen * 4) - (tcp->headerlen * 4));i++) {
		tcp_data[i - (tcp->headerlen * 4)] = ip->data[i];
	}
	
	uint16_t temp_port = tcp->destination_port;
	tcp->destination_port = tcp->source_port;
	tcp->source_port = temp_port;

	ip->destinationIP = ip->sourceIP;
	ip->sourceIP = my_ip;
	ip->fragment = HTONS(ip->fragment);
	ip->id = HTONS(ip->id);
	
	last_message = "socketID tcp_handle";
	uint32_t socketID = (ip->sourceIP.ip1) +
						(ip->sourceIP.ip2) +
						(ip->sourceIP.ip3) +
						(ip->sourceIP.ip4) +
						(HTONS(tcp->destination_port)) +
						checksum(ip->sourceIP,4) +
						checksum(tcp->destination_port,2);
	if(listeners[HTONS(temp_port)].tcp_listener.enabled) {
		listeners[HTONS(temp_port)].tcp_listener.data = tcp_data;
		listeners[HTONS(temp_port)].tcp_listener.data_length = ip->packetsize - (ip->headerlen * 4) - (tcp->headerlen * 4);
		listeners[HTONS(temp_port)].tcp_listener.tcp = tcp;
		listeners[HTONS(temp_port)].tcp_listener.ip = ip;
		listeners[HTONS(temp_port)].tcp_listener.ether = ether;
		listeners[HTONS(temp_port)].tcp_listener.new_con = false;
		
		struct clients *client = find_client(socketID,HTONS(temp_port));
		con_id++;
		if(client == NULL) { //no socketID
			if(check_tcp_flags(tcp->flags, syn)) { //asking for connection
				tcp->flags.syn = 1;
				tcp->flags.ack = 1;
				tcp->ack_number = HTONL(HTONL(tcp->sequence_number) + 1);
				tcp->sequence_number = HTONL(tcp->sequence_number);
				client = add_client(socketID,HTONS(temp_port));
				client->last_seq = HTONL(tcp->sequence_number);
				client->last_ack = HTONL(tcp->ack_number);
				sendTCPpacket(ether, ip, tcp, tcp->options, 0, tcp->data, 0);
			}
		} else {
			if(client->con_est) {
				switch(convert_flags(tcp->flags)) {
					case fin | ack: //got fin
						if(tcp->ack_number != HTONL(client->fin_seq + 1) && tcp->sequence_number != HTONL(client->fin_ack)) {
							tcp->flags.fin = 1;
							tcp->flags.ack = 1;
							uint32_t ack_temp = tcp->ack_number;
							tcp->ack_number = HTONL(HTONL(tcp->sequence_number) + 1);
							tcp->sequence_number = ack_temp;
							client->fin_seq = HTONL(tcp->sequence_number);
							client->fin_ack = HTONL(tcp->ack_number);
							sendTCPpacket(ether, ip, tcp, tcp->options, 0, tcp->data, 0);
						}
						break;
					case ack: //got fin-ack
						if(tcp->ack_number == HTONL(client->fin_seq + 1) && tcp->sequence_number == HTONL(client->fin_ack)) {
							client->fin_seq = 0;
							client->fin_ack = 0;
							uint32_t ack_temp = tcp->ack_number;
							tcp->ack_number = HTONL(HTONL(tcp->sequence_number) + 1);
							tcp->sequence_number = ack_temp;
							client->con_est = false;
							if(del_client(client->client_id,HTONS(temp_port))) {
								//kprintf("closed\n");
							} else {
								//kprintf("error\n");
							}
							sendTCPpacket(ether, ip, tcp, tcp->options, 0, tcp->data, 0);
						}
						break;
					case ack | psh: //got packet
						tcp->flags.psh = 0;
						uint32_t ack_temp = tcp->ack_number;
						tcp->ack_number = HTONL(HTONL(tcp->sequence_number) + listeners[HTONS(temp_port)].tcp_listener.data_length);
						tcp->sequence_number = ack_temp;
						client->last_seq = HTONL(tcp->sequence_number);
						client->last_ack = HTONL(tcp->ack_number);
						sendTCPpacket(ether, ip, tcp, tcp->options, 0, listeners[HTONS(temp_port)].tcp_listener.data, 0);
						
						struct tcp_timer_args* tempa = temp_args;
						struct tcp_timer_args* tempb;
						while(tempa != NULL) {
							if(tempa->tcp == tcp &&
									tempa->ip == ip &&
									tempa->ether == ether) {
								tempb = tempa;
								break;
							}
						}
						kprintf("tcp.c: 287\n");//sleep(100);
						last_message = "abc";
						unregister_timer(tempb->timer);
						
						callback_func = listeners[HTONS(temp_port)].tcp_listener.callback_pointer;
						callback_func(listeners[HTONS(temp_port)].tcp_listener);
						break;
					default:
						kprintf("");
						break;
				}
			} else {
				if(check_tcp_flags(tcp->flags, ack) && !client->con_est) { //ack connection
					if(client->last_ack == HTONL(tcp->sequence_number) && HTONL(tcp->ack_number) == client->last_seq + 1) {
						client->con_est = true;
						listeners[HTONS(temp_port)].tcp_listener.data = tcp_data;
						listeners[HTONS(temp_port)].tcp_listener.new_con = true;						
						listeners[HTONS(temp_port)].tcp_listener.data_length = 0;
						callback_func = listeners[HTONS(temp_port)].tcp_listener.callback_pointer;
						callback_func(listeners[HTONS(temp_port)].tcp_listener);
					}
				}
			}
		}
	}
	pmm_free(tcp_data);
	pmm_free(tcp);
}

bool register_tcp_listener(int port, void *callback_pointer) {
	if(listeners[port].tcp_listener.enabled == true) {
		return false;
	} else {
		struct tcp_callback cb = {
			.callback_pointer = callback_pointer,
			.port = port,
			.con_est = false,
			.enabled = true,
			.data = pmm_alloc()
		};
		if(cb.callback_pointer == NULL) {
			kprintf("Callback not set\n");
			return false;
		}
		listeners[port].tcp_listener = cb;
		listeners[port].tcp_listener.enabled = true;
		return true;
	}
}

bool show_close = false;

void closeCon(struct tcp_callback cb) {
	last_message = "socketID closeCon";
	uint32_t socketID = (cb.ip->sourceIP.ip1) +
						(cb.ip->sourceIP.ip2) +
						(cb.ip->sourceIP.ip3) +
						(cb.ip->sourceIP.ip4) +
						(HTONS(cb.tcp->destination_port)) +
						checksum(cb.ip->sourceIP,4) +
						checksum(cb.tcp->destination_port,2);
	if(find_client(socketID,cb.port) != NULL) {
		struct clients *client = find_client(socketID,cb.port);
		if(listeners[cb.port].tcp_listener.enabled && client->con_est) {
			//sleep(1000);
			uint32_t ack_temp = cb.tcp->ack_number;
			uint32_t seq_temp = cb.tcp->sequence_number;
			cb.tcp->sequence_number = seq_temp;
			cb.tcp->ack_number = ack_temp;
			cb.tcp->flags.ack = 1;
			cb.tcp->flags.psh = 0;
			cb.tcp->flags.rst = 0;
			cb.tcp->flags.syn = 0;
			cb.tcp->flags.fin = 1;
			cb.tcp->flags.urg = 0;
			cb.tcp->flags.ece = 0;
			cb.tcp->flags.cwr = 0;
			cb.fin_ack = HTONL(ack_temp);
			cb.fin_seq = HTONL(seq_temp);
			client->fin_ack = cb.fin_ack;
			client->fin_seq = cb.fin_seq;
			show_close = true;
			sendTCPpacket(cb.ether, cb.ip, cb.tcp, cb.tcp->options, 0, cb.data, 0);
		}
	}
}

void sendData(struct tcp_callback cb) {
	last_message = "socketID sendData";
	uint32_t socketID = (cb.ip->sourceIP.ip1) +
						(cb.ip->sourceIP.ip2) +
						(cb.ip->sourceIP.ip3) +
						(cb.ip->sourceIP.ip4) +
						(HTONS(cb.tcp->destination_port)) +
						checksum(cb.ip->sourceIP,4) +
						checksum(cb.tcp->destination_port,2);
	if(find_client(socketID,cb.port) != NULL) {
		struct clients *client = find_client(socketID,cb.port);
		if(listeners[cb.port].tcp_listener.enabled && client->con_est) {
			uint32_t ack_temp = cb.tcp->sequence_number;
			uint32_t seq_temp = HTONL(HTONL(cb.tcp->ack_number));
			cb.tcp->sequence_number = seq_temp;
			cb.tcp->ack_number = ack_temp;
			cb.tcp->flags.ack = 1;
			cb.tcp->flags.psh = 1;
			cb.tcp->flags.rst = 0;
			cb.tcp->flags.syn = 0;
			cb.tcp->flags.fin = 0;
			cb.tcp->flags.urg = 0;
			cb.tcp->flags.ece = 0;
			cb.tcp->flags.cwr = 0;
			sendTCPpacket(cb.ether, cb.ip, cb.tcp, cb.tcp->options, 0, cb.data, cb.data_length);
		}
	}
}

void sendTCPpacket(struct ether_header* ether, struct ip_header* ip, struct tcp_header* tcp, uint32_t options[], int options_count, uint8_t *data, int data_length) {
	uint16_t packetsize = 20 + 20 + options_count + data_length;
	int pos = 0;
	int pos1 = 0;
	uint8_t *temp;
	
	ip->checksum = 0;
	ip->packetsize = HTONS(packetsize);
	ip->headerlen = 5;
	ip->version = 4;
	tcp->checksum = 0;
	tcp->headerlen = (packetsize - data_length - 20) / 4;
	
	last_message = "sendTCPpacket ip";
	ip->checksum = HTONS(checksum(ip,20));
	
	struct tcp_pseudo_header head = {
		.sourceIP = ip->sourceIP,
		.destinationIP = ip->destinationIP,
		.protocol = 6,
		.tcp_length = HTONS((uint16_t)packetsize - 20)
	};
	
	uint8_t tcpChecksum[12 + packetsize - 20];
	temp = &head;
	for(int i = 0; i < 12; i++) {
		tcpChecksum[pos1] = temp[i];
		pos1++;
	}
	temp = tcp;
	for(int i = 0; i < 20; i++) { //tcp_header
		tcpChecksum[pos1] = temp[i];
		pos1++;
	}
	temp = &options;
	for(int i = 0; i < options_count; i++) { //tcp_options
		tcpChecksum[pos1] = temp[i];
		pos1++;
	}
	//temp = data;
	for(int i = 0; i < data_length; i++) { //tcp_data
		tcpChecksum[pos1] = data[i];
		pos1++;
	}
	//pos1--;
	last_message = "sendTCPpacket tcp";
	tcp->checksum = HTONS(checksum(&tcpChecksum, pos1));
	
	//uint8_t buffer[packetsize];
	uint8_t *buffer = pmm_alloc();
	temp = ip;
	for(int i = 0; i < 20; i++) { //ip_header
		buffer[pos] = temp[i];
		pos++;
	}
	temp = tcp;
	for(int i = 0; i < 20; i++) { //tcp_header
		buffer[pos] = temp[i];
		pos++;
	}
	temp = &options;
	for(int i = 0; i < options_count; i++) { //tcp_options
		buffer[pos] = temp[i];
		pos++;
	}
	//temp = data;
	for(int i = 0; i < data_length; i++) { //tcp_data
		buffer[pos] = data[i];
		pos++;
	}
	//pos--;
	struct tcp_timer_args* args = pmm_alloc();
	args->retry = 0;
	args->ether = ether;
	args->ip = ip;
	args->tcp = tcp;
	args->next = NULL;
	struct tcp_timer_args* tempa = temp_args;
	if(tempa != NULL) {
		while(tempa->next != NULL) {
			tempa = tempa->next;
		}
		tempa->next = args;
	} else {
		temp_args = args;
	}
	sendPacket(ether,buffer,pos);
	
	args->timer = register_timer(retry_send, 2000, false, args);
	pmm_free(buffer);
	kprintf("tcp.c: 495\n");
}

/*bool tcp_open_con(int port, void *callback_pointer) {
	int i;
	for(i = 60000; i < 65536; i++) {
		if(connections[i].in_use == false) {
			break;
		}
	}
	connections[i].in_use = true;
	connections[i].remote_port = port;
	register_tcp_listener(port,callback_pointer);
}*/