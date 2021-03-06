#pragma once

#define DDD_NET_OPCODES_VERSION 3

enum DddNetOpcode {

	TYPE_INT16 = 1,
	TYPE_INT32 = 2,
	TYPE_INT64 = 3,
	TYPE_FLOAT = 4,
	TYPE_DOUBLE = 5,
	TYPE_STRING = 6,
	TYPE_OBJECT = 7,
	TYPE_OBJECT_ARR = 8,

	USER_ID = 101,
	FLAGS = 102,
	MESSAGE = 103,
	MAP_NAME = 104,
	NAME = 105,
	STEAM_ID = 106,
	IP_ADDRESS = 107,
	TEAM = 108,
	KEY = 109,
	VALUE = 110,
	PLAYER_CLASS = 111,
	KILLS = 112,
	DEATHS = 113,
	GOAT_KILLS = 114,
	HEAL_POINTS = 115,
	PSK = 116,
	MAX_PLAYERS = 117,
	PLUGIN_VERSION = 118,
	PLUGIN_COMPILE_DATE = 119,
	ACK_ID = 120,
	ACK_RESULT = 121,
	KICK_REASON = 122,
	MAP_MODE = 123,
	COMMAND = 124,
	AVG_FRAME_DURATION = 125,
	MAX_FRAME_DURATION = 126,
	HELP_TEXT = 127,
	MIN = 128,
	MAX = 129,
	VALUE_STRING = 130,
	VALUE_INT = 131,
	VALUE_FLOAT = 132,
	OFFSET = 133,
	ITEMS = 134,
	ITEMS_REMAINING = 135

};

enum DddNetEndpoint {

	CONNECTION_LOGIN = 1,
	CONNECTION_INIT = 2,
	CONNECTION_REQUEST_ACK = 3,

	PLAYER_CONNECT = 11,
	PLAYER_DISCONNECT = 12,
	PLAYER_CHAT = 13,
	PLAYER_UPDATE = 14,
	PLAYER_KICK = 15,

	MAP_START = 21,
	MAP_END = 22,
	MAP_CHANGE = 23,

	SERVER_COMMAND = 31,
	SERVER_REPORT_STAT = 32,

	SERVER_CVAR_PAGE = 33,
	SERVER_CVAR_QUERY = 34,
	SERVER_CVAR_SET = 35

};