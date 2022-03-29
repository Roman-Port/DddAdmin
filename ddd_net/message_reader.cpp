#include "include/ddd_net_message.h"
#include "defines.h"
#include <stdexcept>
#include <stdio.h>
#include <cassert>

template <class T>
bool read_from(const uint8_t** buffer, size_t* buffer_size, T* result) {
	//Make sure enough space is available
	if ((*buffer_size) < sizeof(T))
		return false;

	//Read
	memcpy(result, (*buffer), sizeof(T));

	//Update state
	(*buffer) += sizeof(T);
	(*buffer_size) -= sizeof(T);
}

bool DddNetMsg::deserialize(const uint8_t* buffer, size_t buffer_size)
{
	//Read the number of properties
	uint16_t propCount;
	if (!read_from(&buffer, &buffer_size, &propCount))
		return false;

	//Read each property
	ddd_net_msg_item_header header;
	for (int i = 0; i < propCount; i++) {
		//Read header
		if (!read_from(&buffer, &buffer_size, &header))
			return false;

		//Validate payload size
		if (header.payload_length > MAX_PACKET_SIZE)
			return false;

		//Ensure that there is space to read the payload
		if (buffer_size < header.payload_length)
			return false;

		//Allocate buffer
		void* payload = malloc(header.payload_length);
		if (payload == 0)
			throw std::runtime_error("Failed to allocate data.");

		//Copy into buffer
		memcpy(payload, buffer, header.payload_length);
		buffer += header.payload_length;
		buffer_size -= header.payload_length;

		//Allocate item struct
		ddd_net_msg_item* info = (ddd_net_msg_item*)malloc(sizeof(ddd_net_msg_item));
		if (info == 0)
			throw std::runtime_error("Failed to allocate struct.");

		//Set values in struct
		info->header = header;
		info->payload = payload;
		info->next = next;

		//Apply
		next = info;
		property_payload_total_length += header.payload_length;
		property_count++;
	}

	//We should be at the end of the packet
	return buffer_size == 0;
}

bool DddNetMsg::find(DddNetOpcode key, ddd_net_msg_item** item)
{
	(*item) = next;
	while ((*item) != 0) {
		if ((*item)->header.key == key)
			return true;
	}
	return false;
}

bool DddNetMsg::get_fixed(DddNetOpcode key, DddNetOpcode type, void* outputData, size_t outputDataLength)
{
	//Find the item
	ddd_net_msg_item* item;
	if (!find(key, &item))
		return false;

	//Make sure types and lengths match
	if (item->header.type != type || item->header.payload_length != outputDataLength)
		return false;

	//Copy
	memcpy(outputData, item->payload, outputDataLength);

	return true;
}

bool DddNetMsg::get_short(DddNetOpcode key, int16_t* value)
{
	return get_fixed(key, DddNetOpcode::TYPE_INT16, value, sizeof(int16_t));
}

bool DddNetMsg::get_int(DddNetOpcode key, int32_t* value)
{
	return get_fixed(key, DddNetOpcode::TYPE_INT32, value, sizeof(int32_t));
}

bool DddNetMsg::get_long(DddNetOpcode key, int64_t* value)
{
	return get_fixed(key, DddNetOpcode::TYPE_INT64, value, sizeof(int64_t));
}

bool DddNetMsg::get_string(DddNetOpcode key, char* output, size_t output_max)
{
	//Find the item
	ddd_net_msg_item* item;
	if (!find(key, &item))
		return false;

	//Make sure types and lengths match
	if (item->header.type != DddNetOpcode::TYPE_STRING || item->header.payload_length >= output_max + 1)
		return false;

	//Copy
	memcpy(output, item->payload, item->header.payload_length);

	//Set null terminator
	output[item->header.payload_length] = 0;

	return true;
}

bool DddNetMsg::get_msg(DddNetOpcode key, DddNetMsg* msg)
{
	//Find the item
	ddd_net_msg_item* item;
	if (!find(key, &item))
		return false;

	//Make sure types match
	if (item->header.type != DddNetOpcode::TYPE_OBJECT)
		return false;

	//Deserialize
	return msg->deserialize((uint8_t*)item->payload, item->header.payload_length);
}
