#include "include/ddd_net_message.h"
#include "util.h"
#include <stdexcept>
#include <stdio.h>
#include <cassert>

size_t DddNetMsg::get_serialized_length()
{
	return sizeof(uint16_t) + property_payload_total_length + (sizeof(ddd_net_msg_item_header) * property_count);
}

size_t DddNetMsg::serialize(uint8_t* buffer, size_t buffer_size)
{
	//Validate
	if (buffer_size < get_serialized_length())
		throw std::runtime_error("Output serialization buffer does not have enough space!");

	//Write header
	size_t offset = 0;
	write_to(buffer, &offset, property_count);

	//Write properties
	ddd_net_msg_item* item = next;
	while (item) {
		//Write header
		write_to(buffer, &offset, item->header);

		//Write paylod
		memcpy(&buffer[offset], item->payload, item->header.payload_length);
		offset += item->header.payload_length;
		item = item->next;
	}

	//Sanity check
	assert(offset == get_serialized_length());

	return offset;
}

void DddNetMsg::clear()
{
	//Find and clean up all values
	ddd_net_msg_item* temp;
	while (next) {
		//Save next after
		temp = next->next;

		//Free the buffer
		free(next->payload);

		//Free the struct
		free(next);

		//Advance cursor
		next = temp;
	}

	//Reset
	property_count = 0;
	property_payload_total_length = 0;
}

void DddNetMsg::put_short(DddNetOpcode key, int16_t value)
{
	put(key, DddNetOpcode::TYPE_INT16, &value, sizeof(int16_t));
}

void DddNetMsg::put_int(DddNetOpcode key, int32_t value)
{
	put(key, DddNetOpcode::TYPE_INT32, &value, sizeof(int32_t));
}

void DddNetMsg::put_long(DddNetOpcode key, int64_t value)
{
	put(key, DddNetOpcode::TYPE_INT64, &value, sizeof(int64_t));
}

void DddNetMsg::put_float(DddNetOpcode key, float value)
{
	put(key, DddNetOpcode::TYPE_FLOAT, &value, sizeof(float));
}

void DddNetMsg::put_double(DddNetOpcode key, double value)
{
	put(key, DddNetOpcode::TYPE_DOUBLE, &value, sizeof(double));
}

void DddNetMsg::put_string(DddNetOpcode key, const char* value)
{
	put(key, DddNetOpcode::TYPE_STRING, value, value == 0 ? 0 : strlen(value));
}

void DddNetMsg::put_msg(DddNetOpcode key, DddNetMsg* msg)
{
	//Query length and allocate a buffer of that size
	size_t length = msg->get_serialized_length();
	void* buffer = malloc(length);

	//Serialize
	msg->serialize((uint8_t*)buffer, length);

	//Write
	put(key, DddNetOpcode::TYPE_OBJECT, buffer, length);

	//Clean up
	free(buffer);
}

void DddNetMsg::put_msg_array(DddNetOpcode key, DddNetMsg** msgs, size_t count)
{
	//Sanity check
	if (count < 0 || count > 2147483647)
		throw std::runtime_error("Attempted to write too many messages in one array!");

	//Query total length
	size_t totalLength = 0;
	for (size_t i = 0; i < count; i++)
		totalLength += msgs[i]->get_serialized_length();

	//Allocate buffer and write header
	uint8_t* buffer = (uint8_t*)malloc(totalLength + sizeof(uint32_t));
	size_t offset = 0;
	write_to(buffer, &offset, (uint32_t)count);

	//Serialize
	for (size_t i = 0; i < count; i++)
		offset += msgs[i]->serialize(&buffer[offset], totalLength - offset);

	//Sanity check
	assert(totalLength == offset);

	//Write
	put(key, DddNetOpcode::TYPE_OBJECT_ARR, buffer, totalLength);

	//Clean up
	free(buffer);
}

void DddNetMsg::put(DddNetOpcode key, DddNetOpcode type, const void* data, size_t length)
{
	//Validate
	if (length > 2147483647 || length < 0)
		throw std::runtime_error("Attempted to write a property with an invalid length!");
	if (data == 0 && length != 0)
		throw std::runtime_error("Attempted to write from NULL pointer.");
	if (property_count == 65535)
		throw std::runtime_error("Attempted to write more properties when the maximum number is already reached!");

	//Allocate buffer
	void* payload = malloc(length);
	if (payload == 0)
		throw std::runtime_error("Failed to allocate data.");

	//Copy into buffer
	memcpy(payload, data, length);

	//Allocate item struct
	ddd_net_msg_item* info = (ddd_net_msg_item*)malloc(sizeof(ddd_net_msg_item));
	if (info == 0)
		throw std::runtime_error("Failed to allocate struct.");

	//Set values in struct
	info->header.key = (uint16_t)key;
	info->header.type = (uint16_t)type;
	info->header.payload_length = (uint32_t)length;
	info->payload = payload;
	info->next = next;

	//Apply
	next = info;
	property_payload_total_length += length;
	property_count++;
}