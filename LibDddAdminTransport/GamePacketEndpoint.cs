using System;
using System.Collections.Generic;
using System.Text;

namespace LibDddAdminTransport
{
	public enum GamePacketEndpoint : ushort
	{
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
	}
}
