﻿using System;
using System.Collections.Generic;
using System.Text;

namespace LibDddAdminTransport
{
	public enum GameMessageKey : ushort
	{
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
	}
}