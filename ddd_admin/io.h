// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#pragma once

#include <stdint.h>

enum ddd_msg_endpoint {

	BOOT,
	SERVER_START,
	SERVER_CVAR,
	SERVER_CVAR_REGISTER,
	SERVER_CVAR_REGISTER_LATE,
	
	MAP_START,
	MAP_END,

	PLAYER_CONNECT,
	PLAYER_DISCONNECT,
	PLAYER_CHAT,
	PLAYER_UPDATE,

};

enum ddd_msg_key {

	DDDTYPE_SHORT,
	DDDTYPE_INT,
	DDDTYPE_FLOAT,
	DDDTYPE_STRING,
	DDDTYPE_MESSAGE,
	DDDTYPE_MESSAGE_ARRAY,

	USER_ID,
	FLAGS,
	STEAM_ID,
	IP_ADDRESS,
	NAME,
	REASON,
	MESSAGE,
	TEAM_ONLY,
	MAX_PLAYERS,
	OS,
	DEDICATED,
	OFFICIAL,
	PASSWORD_PROTECTED,
	VALUE,
	HELP,
	MAX,
	MIN,
	TEAM,
	CLASS,
	FRAGS,
	DEATHS,
	GOAT_KILLS,
	HEAL_POINTS,
	MAP_NAME,
	MAP_MODE,
	PLAYERS,
	CVARS,
	PLUGIN_VERSION,
	PLUGIN_COMPILE_DATE

};