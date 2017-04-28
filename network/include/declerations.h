#ifndef DECLERATIONS_H
#define DECLERATIONS_H

#include "includes.h"

#define HTONS(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#define NTOHS(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))

#define HTONL(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                  ((((unsigned long)(n) & 0xFF00)) << 8) | \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))

#define NTOHL(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                  ((((unsigned long)(n) & 0xFF00)) << 8) | \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))

static uint16_t packet_id = 0x0000;

struct ip_addr {
	uint8_t ip1;
	uint8_t ip2;
	uint8_t ip3;
	uint8_t ip4;
} __attribute__((packed));

struct mac {
	uint8_t mac1;
	uint8_t mac2;
	uint8_t mac3;
	uint8_t mac4;
	uint8_t mac5;
	uint8_t mac6;
} __attribute__((packed));

static struct ip_addr my_ip = {
	.ip1 = 10,
	.ip2 = 0,
	.ip3 = 0,
	.ip4 = 114
};

static struct ip_addr broadcast_ip = {
	.ip1 = 255,
	.ip2 = 255,
	.ip3 = 255,
	.ip4 = 255
};

static struct mac my_mac = {
	.mac1 = 0x00,
	.mac2 = 0xE0,
	.mac3 = 0xC5,
	.mac4 = 0x52,
	.mac5 = 0xD2,
	.mac6 = 0x54
};

static struct mac broadcast_mac = {
	.mac1 = 0xFF,
	.mac2 = 0xFF,
	.mac3 = 0xFF,
	.mac4 = 0xFF,
	.mac5 = 0xFF,
	.mac6 = 0xFF
};

struct ether_header {
	struct mac receipt_mac;
	struct mac sender_mac;
	uint16_t type;
} __attribute__((packed));

struct ip_header {
  unsigned headerlen : 4; //Vertauscht, da h�herwertige Bits zuerst kommen (Big Endian wird nur f�r die Bytereihenfolge verwendet)
  unsigned version : 4;
  uint8_t  priority;
  uint16_t packetsize;
  uint16_t id;
  uint16_t fragment;
  uint8_t  ttl;
  uint8_t  protocol;
  uint16_t checksum;
  struct ip_addr sourceIP;
  struct ip_addr destinationIP;
  uint8_t *data;
  uint32_t data_length;
} __attribute__((packed));

struct udp_header {
	uint16_t source_port;
	uint16_t destination_port;
	uint16_t packetsize;
	uint16_t checksum;
	uint8_t *data;
} __attribute__((packed));

struct dhcp_options {
	uint8_t index;
	uint8_t length;
	uint8_t *data;
} __attribute__((packed));

struct dhcp_packet {
	uint8_t operation;
	uint8_t network_type;
	uint8_t network_addr_length;
	uint8_t relay_agents;
	uint32_t connection_id;
	uint16_t seconds_start;
	uint16_t flags;
	struct ip_addr client_ip;
	struct ip_addr own_ip;
	struct ip_addr server_ip;
	struct ip_addr relay_ip;
	struct mac client_mac;
	uint8_t mac_padding[10];
	uint8_t server_name[64];
	uint8_t file_name[128];
	uint32_t magic_cookie;
	struct dhcp_options options[255];
} __attribute__((packed));

struct dhcp_packet_created {
	uint32_t length;
	struct ether_header ether;
	uint8_t *data;
};

struct arp {
	uint16_t hardware_addr_type;
	uint16_t network_addr_type;
	uint8_t hardware_addr_length;
	uint8_t network_addr_length;
	uint16_t operation;
	struct mac sender_mac;
	struct ip_addr sender_ip;
	struct mac receipt_mac;
	struct ip_addr receipt_ip;
} __attribute__((packed));

struct icmp_echo_packet {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
    uint16_t echo_id;
    uint16_t echo_seq;
} __attribute__((packed));

union arp_test {
	struct arp arp_val1;
	uint8_t data[sizeof(struct arp)];
};

union ether_test {
	struct ether_header ether_val1;
	uint8_t data[sizeof(struct ether_header)];
};

union ip_union {
	struct ip_header ip;
	uint8_t data[sizeof(struct ip_header)];
};

union icmp_ping_union {
	struct icmp_echo_packet icmp;
	uint8_t data[sizeof(struct icmp_echo_packet)];
};

#endif