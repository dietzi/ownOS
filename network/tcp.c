#include "includes.h"

void tcp_handle(struct ip_header ip, struct ether_header ether) {
	struct tcp_header tcp;
	uint8_t *p = &tcp;
}