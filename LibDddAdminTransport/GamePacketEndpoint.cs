using System;
using System.Collections.Generic;
using System.Text;

namespace LibDddAdminTransport
{
	public enum GamePacketEndpoint : ushort
	{
		CONNECTION_LOGIN = 1,
		CONNECTION_INIT = 2,

		PLAYER_CONNECT = 11,
		PLAYER_DISCONNECT = 12,
		PLAYER_CHAT = 13,
		PLAYER_WATCH_VAR = 14,

		MAP_START = 21,
		MAP_END = 22,
	}
}
