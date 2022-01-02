// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#include "game_properties.h"

bool ddd_game_property_find_class(const char* query, ServerClass** result) {
	ServerClass* pClass = server->GetAllServerClasses();
	while (pClass)
	{
		if (strcmp(pClass->m_pNetworkName, query) == 0)
		{
			(*result) = pClass;
			return true;
		}
		pClass = pClass->m_pNext;
	}
	return false;
}

bool ddd_game_property_find_prop_internal(SendTable* pTable, const char* query, int offset, ddd_game_property_search_result* result) {
	int count = pTable->GetNumProps();
	SendProp* pProp;
	int pPropOffset;
	for (int i = 0; i < count; i++) {
		//Get property and the actual offset
		pProp = pTable->GetProp(i);
		pPropOffset = offset + pProp->GetOffset();

		//Check if this is the one
		if (strcmp(pProp->GetName(), query) == 0) {
			result->prop = pProp;
			result->actual_offset = pPropOffset;
			return true;
		}

		//Recursively search...
		if (pProp->GetDataTable() && ddd_game_property_find_prop_internal(pProp->GetDataTable(), query, pPropOffset, result))
			return true;
	}
	return false;
}

bool ddd_game_property_find_prop(SendTable* pTable, const char* query, ddd_game_property_search_result* result) {
	return ddd_game_property_find_prop_internal(pTable, query, 0, result);
}

/* DEBUG DUMP */

void debug_pad_console(int padding) {
	for (int j = 0; j < padding; j++)
		printf("|   ");
}

void debug_dump_table(SendTable* pTable, int padding) {
	//Print name
	debug_pad_console(padding);
	printf("[%s] **START**\n", pTable->GetName());

	//Loop
	int count = pTable->GetNumProps();
	SendProp* pProp;
	for (int i = 0; i < count; i++)
	{
		//Get
		pProp = pTable->GetProp(i);

		//Print
		debug_pad_console(padding);
		printf("(%i) %s\n", pProp->GetType(), pProp->GetName());

		//Print next
		if (pProp->GetDataTable())
			debug_dump_table(pProp->GetDataTable(), padding + 1);
	}

	//Print END name
	debug_pad_console(padding);
	printf("[%s] **END**\n", pTable->GetName());
}

void debug_dump_class(ServerClass* cls) {
	printf("*** ddd_admin CLASS DUMP BEGIN ***\n");
	debug_dump_table(cls->m_pTable, 0);
	printf("*** ddd_admin CLASS DUMP END ***\n");
}