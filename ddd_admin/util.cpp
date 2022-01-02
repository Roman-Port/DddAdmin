// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#include "util.h"

const char* get_server_name() {
	ConVar* var = cvarmanager->FindVar("hostname");
	if (var == NULL)
		return "";
	return var->GetString();
}