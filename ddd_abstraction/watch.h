#pragma once

#include "include/ddd_abstraction.h"
#include "player.h"
#include <server_class.h>
#include <stdint.h>

class DddAbsWatch {

public:
	DddAbsWatch(IDddAbsWatchCallback* callback, SendProp* prop, int actual_offset);
	~DddAbsWatch();

	static bool from_property(DddAbsWatch** result, IDddAbsWatchCallback* callback, const char* classname, const char* propname);

	void update(IDddAbsPlayer* player);
	void clear(int index);
	void clear_all();

private:
	IDddAbsWatchCallback* callback;
	size_t prop_size;
	SendProp* prop;
	int actual_offset;
	uint8_t* previous;
	bool initialized[MAX_PLAYER_SLOTS];

};