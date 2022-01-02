// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#pragma once
#include "ddd_defines.h"
#include <server_class.h>

struct ddd_game_property_search_result {

	SendProp* prop;
	int actual_offset;

};

void debug_dump_class(ServerClass* cls);

bool ddd_game_property_find_class(const char* query, ServerClass** result);

// Searches for a game property within the table. Upon finding the property, results are written to the result struct.
bool ddd_game_property_find_prop(SendTable* pTable, const char* query, ddd_game_property_search_result* result);