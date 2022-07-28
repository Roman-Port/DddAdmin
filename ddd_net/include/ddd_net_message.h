#pragma once

#include <stdint.h>
#include "ddd_net_opcodes.h"

struct ddd_net_msg_item_header {

	uint16_t key;
	uint16_t type;
	uint32_t payload_length;

};

struct ddd_net_msg_item {

	ddd_net_msg_item_header header;
	void* payload;
	ddd_net_msg_item* next;

};

class DddNetMsg {

public:
	DddNetMsg();
	~DddNetMsg();

	size_t get_serialized_length();
	size_t serialize(uint8_t* buffer, size_t buffer_size);
	bool deserialize(const uint8_t* buffer, size_t buffer_size);

	void clear();

	void put_short(DddNetOpcode key, int16_t value);
	void put_int(DddNetOpcode key, int32_t value);
	void put_long(DddNetOpcode key, int64_t value);
	void put_float(DddNetOpcode key, float value);
	void put_double(DddNetOpcode key, double value);
	void put_string(DddNetOpcode key, const char* value);
	void put_msg(DddNetOpcode key, DddNetMsg* msg);
	void put_msg_array(DddNetOpcode key, DddNetMsg** msgs, size_t count);

	bool get_short(DddNetOpcode key, int16_t* value);
	bool get_int(DddNetOpcode key, int32_t* value);
	bool get_long(DddNetOpcode key, int64_t* value);
	bool get_float(DddNetOpcode key, float* value);
	bool get_double(DddNetOpcode key, int64_t* value);
	bool get_string(DddNetOpcode key, char* output, size_t output_max);
	bool get_msg(DddNetOpcode key, DddNetMsg* msg);

private:
	void put(DddNetOpcode key, DddNetOpcode type, const void* data, size_t length);
	bool get_fixed(DddNetOpcode key, DddNetOpcode type, void* outputData, size_t outputDataLength);
	bool find(DddNetOpcode key, ddd_net_msg_item** item);

	ddd_net_msg_item* next;
	size_t property_payload_total_length; // Only payload length, not the header of entries!
	uint16_t property_count;

};