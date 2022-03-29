#pragma once

#include <stdint.h>

#define MAX_PACKET_SIZE 8388608 // ~8 MB
#define NETWORK_RETRY_DELAY 10000 // In Milliseconds

struct net_packet_header {

	uint32_t packet_length;
	uint16_t protocol_version;
	uint16_t opcode_version;
	uint16_t opcode;
	uint16_t reserved;

};