#include "watch.h"
#include <stdio.h>
#include <stdexcept>

DddAbsWatch::DddAbsWatch(IDddAbsWatchCallback* callback, SendProp* prop, int actual_offset) {
	//Set
	this->callback = callback;
	this->prop = prop;
	this->actual_offset = actual_offset;

	//Determine size
	switch (prop->GetType()) {
	case DPT_Int: prop_size = sizeof(int32_t); break;
	case DPT_Int64: prop_size = sizeof(int64_t); break;
	case DPT_Float: prop_size = sizeof(float); break;
	case DPT_String: prop_size = sizeof(char*); break;
	default: prop_size = 1; printf("WARN: Unknown type in DddAbsWatch watch! Unable to determine size, assuming 1...\n"); break;
	}

	//Create buffer
	previous = (uint8_t*)malloc(prop_size * MAX_PLAYER_SLOTS);

	//Set flags
	clear_all();
}

DddAbsWatch::~DddAbsWatch() {
	free(previous);
}

void DddAbsWatch::update(IDddAbsPlayer* player) {
	//Get index
	int index = player->get_index();

	//Get pointer to the payload
	uint8_t* src = ((uint8_t*)player->get_entity()) + actual_offset;

	//Get the challenge data
	uint8_t* dst = &previous[index * prop_size];

	//Compare
	if (!initialized[index] || memcmp(dst, src, prop_size) != 0) {
		//Run callback (so they can change it if wanted)
		callback->on_value_changed(player, src, prop_size);

		//Copy
		memcpy(dst, src, prop_size);

		//Update flag
		initialized[index] = true;
	}
}

void DddAbsWatch::clear(int index) {
	initialized[index] = false;
}

void DddAbsWatch::clear_all() {
	for (int i = 0; i < MAX_PLAYER_SLOTS; i++)
		initialized[i] = false;
}

bool find_watch_property(SendTable* pTable, const char* query, int offset, int* resultOffset, SendProp** resultProp) {
	int count = pTable->GetNumProps();
	SendProp* pProp;
	int pPropOffset;
	for (int i = 0; i < count; i++) {
		//Get property and the actual offset
		pProp = pTable->GetProp(i);
		pPropOffset = offset + pProp->GetOffset();

		//Check if this is the one
		if (strcmp(pProp->GetName(), query) == 0) {
			(*resultProp) = pProp;
			(*resultOffset) = pPropOffset;
			return true;
		}

		//Recursively search...
		if (pProp->GetDataTable() && find_watch_property(pProp->GetDataTable(), query, pPropOffset, resultOffset, resultProp))
			return true;
	}
	return false;
}

bool find_watch_property(const char* classname, const char* propname, int* resultOffset, SendProp** resultProp) {
	//Find the class
	ServerClass* pClass = server->GetAllServerClasses();
	while (true)
	{
		if (!pClass)
			return false;
		if (strcmp(pClass->m_pNetworkName, classname) == 0)
			break;
		pClass = pClass->m_pNext;
	}

	//Find the property
	return find_watch_property(pClass->m_pTable, propname, 0, resultOffset, resultProp);
}

bool DddAbsWatch::from_property(DddAbsWatch** result, IDddAbsWatchCallback* callback, const char* classname, const char* propname) {
	//Search for the property
	int offset;
	SendProp* prop;
	if (!find_watch_property(classname, propname, &offset, &prop))
		return false;

	//Wrap
	(*result) = new DddAbsWatch(callback, prop, offset);
	return true;
}