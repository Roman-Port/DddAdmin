using System;
using System.Collections.Generic;
using System.Text;

namespace LibDddAdminTransport
{
	public enum GamePacketEndpoint : ushort
	{
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
	}
}
