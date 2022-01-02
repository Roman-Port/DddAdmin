// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#include "io.h"
#include <malloc.h>
#include <stdexcept>

/* *** MESSAGE WRITER BASE *** */

ddd_msg_writer_base::ddd_msg_writer_base(size_t length) {
	buffer = (uint8_t*)malloc(sizeof(uint8_t) * length);
	buffer_len = length;
	buffer_pos = 0;
	if (buffer == 0)
		throw new std::runtime_error("Failed to allocate buffer.");
}

ddd_msg_writer_base::~ddd_msg_writer_base() {
	if (buffer != 0) {
		free(buffer);
		buffer = 0;
	}
}

void ddd_msg_writer_base::write(const void* src, size_t len) {
	//Expand the buffer if needed
	if (buffer_pos + len >= buffer_len) {
		//Determine how much we'll need to extend by
		size_t newLength = buffer_len;
		while (buffer_pos + len >= newLength)
			newLength *= 2;

		//Create the new buffer and copy
		uint8_t* newBuffer = (uint8_t*)malloc(sizeof(uint8_t) * newLength);
		memcpy(newBuffer, buffer, sizeof(uint8_t) * buffer_len);

		//Destroy old buffer and replace
		free(buffer);
		buffer = newBuffer;
		buffer_len = newLength;
	}

	//Copy
	memcpy(buffer + buffer_pos, src, len);
	buffer_pos += len;
}

/* *** MESSAGE WRITER *** */

#define WRITE_AS(value, type) {type _temp = (type)value; write(&_temp, sizeof(type));}
#define WRITE_AS_SHORT(value) WRITE_AS(value, uint16_t)
#define WRITE_AS_INT(value) WRITE_AS(value, uint32_t)

void ddd_msg_writer::write_property(ddd_msg_key key, ddd_msg_key type, size_t length, const void* src) {
	//Sanity check
	if (length > 4294967295) {
		throw new std::runtime_error("Attempted to write a LOT of data to a message writer. Something has probably gone wrong.");
	}

	//Write header
	WRITE_AS_SHORT(key);
	WRITE_AS_SHORT(type);
	WRITE_AS_INT(length);

	//Write payload
	write(src, length);
}

void ddd_msg_writer::put_short(ddd_msg_key key, int16_t value) {
	write_property(key, ddd_msg_key::DDDTYPE_SHORT, sizeof(int16_t), &value);
}

void ddd_msg_writer::put_int(ddd_msg_key key, int32_t value) {
	write_property(key, ddd_msg_key::DDDTYPE_INT, sizeof(int32_t), &value);
}

void ddd_msg_writer::put_float(ddd_msg_key key, float value) {
	write_property(key, ddd_msg_key::DDDTYPE_FLOAT, sizeof(float), &value);
}

void ddd_msg_writer::put_string(ddd_msg_key key, const char* value, int maxLength) {
	//Make sure it's a valid pointer
	if (value == 0) {
		printf("ddd_admin:WARN // Attempted to write string to message, but the string was a null pointer!\n");
		return;
	}

	//Get the length and verify
	size_t length = strnlen_s(value, maxLength);

	//Write
	write_property(key, ddd_msg_key::DDDTYPE_STRING, length, value);
}

void ddd_msg_writer::put_msg(ddd_msg_key key, ddd_msg_writer& writer) {
	write_property(key, ddd_msg_key::DDDTYPE_MESSAGE, writer.buffer_pos, writer.buffer);
}

void ddd_msg_writer::put_array(ddd_msg_key key, ddd_msg_writer_array& writer) {
	write_property(key, ddd_msg_key::DDDTYPE_MESSAGE_ARRAY, writer.buffer_pos, writer.buffer);
}

/* *** ARRAY WRITER *** */

ddd_msg_writer_array::ddd_msg_writer_array(size_t length) : ddd_msg_writer_base(length) {
	//Reserve the first few bytes for the element count
	int count = 0;
	write(&count, sizeof(int));
}

void ddd_msg_writer_array::put(ddd_msg_writer& writer) {
	//Write the length of the element
	int length = writer.buffer_pos;
	write(&length, sizeof(int));

	//Write the element payload
	write(writer.buffer, length);

	//Update the count at the beginning
	int* ref = (int*)buffer;
	(*ref) += 1;
}