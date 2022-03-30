﻿using System;
using System.Collections.Generic;
using System.Text;

namespace LibDddAdminTransport
{
	public enum GameMessageKey : ushort
	{
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
		MAX_FRAME_DURATION = 126
	}
}
