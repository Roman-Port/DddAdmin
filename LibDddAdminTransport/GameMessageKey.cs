using System;
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
		TYPE_STRING = 5,
		TYPE_OBJECT = 6,
		TYPE_OBJECT_ARR = 7,

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
		PLUGIN_COMPILE_DATE = 119
	}
}
