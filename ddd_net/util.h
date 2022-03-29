#pragma once

#include <cstring>

template <class T>
static void write_to(unsigned char* buffer, size_t* offset, T value) {
	memcpy(&buffer[(*offset)], &value, sizeof(T));
	(*offset) += sizeof(T);
}