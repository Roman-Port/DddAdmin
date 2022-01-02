// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#include "game.h"
#include "game_properties.h"
#include "game_watch_impl.h"
#include "util.h"

#define MAKE_WATCH_FROM_PROP_NAME(cls, type, propName, key) { ddd_game_property_search_result prop; if (ddd_game_property_find_prop(cls->m_pTable, propName, &prop)) { add_watch(new type(prop, key, #key)); } else { printf("ddd_admin:ERROR // Failed to find property %s.\n", propName); } }
#define ADD_CVAR_HOOK(cvarName, callback) { ConCommand* cmd = cvarmanager->FindCommand(cvarName); if (cmd == NULL) { printf("ddd_admin:ERROR // Failed to find hooked ConCommand %s.\n", #cvarName); } else { SH_ADD_HOOK(ConCommand, Dispatch, cmd, callback, false); } }
#define GET_CVAR_SENDER() ddd_game_player* player; if (!get_cvar_command_sender(&player)) { return; }

SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, char const*, char const*, char const*, char const*, bool, bool);
SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);
SH_DECL_HOOK1_void(ConCommand, Dispatch, SH_NOATTRIB, 0, const CCommand&);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, false, int);

ddd_game::ddd_game() : cvar_monitoring(this), last_command_client(-1), level_active(false) {

}

void ddd_game::init() {
	//Hook
	SH_ADD_HOOK(IServerGameDLL, LevelInit, server, SH_MEMBER(this, &ddd_game::on_level_init), true);
	SH_ADD_HOOK(IServerGameDLL, LevelShutdown, server, SH_MEMBER(this, &ddd_game::on_level_shutdown), false);

	//Init components
	ddd_game_player_collection::init();
	cvar_monitoring.init();

	//Hook console commands
	SH_ADD_HOOK(IServerGameClients, SetCommandClient, gameclients, SH_MEMBER(this, &ddd_game::on_cvar_set_client), false);
	ADD_CVAR_HOOK("say", SH_MEMBER(this, &ddd_game::on_chat_command));
	ADD_CVAR_HOOK("say_team", SH_MEMBER(this, &ddd_game::on_chat_command));

	//Find player classes to add watches for
	ServerClass* playerClass;
	if (ddd_game_property_find_class("CDDDPlayer", &playerClass)) {
		//debug_dump_class(playerClass);
		//MAKE_WATCH_FROM_PROP_NAME(playerClass, watch_impl_string, "m_iName", ddd_msg_key::NAME);
		MAKE_WATCH_FROM_PROP_NAME(playerClass, watch_impl_int, "m_iTeamNum", ddd_msg_key::TEAM);
		MAKE_WATCH_FROM_PROP_NAME(playerClass, watch_impl_int, "m_iPlayerClass", ddd_msg_key::CLASS);
		MAKE_WATCH_FROM_PROP_NAME(playerClass, watch_impl_int, "m_iKills", ddd_msg_key::FRAGS);
		MAKE_WATCH_FROM_PROP_NAME(playerClass, watch_impl_int, "m_iDeaths", ddd_msg_key::DEATHS);
		MAKE_WATCH_FROM_PROP_NAME(playerClass, watch_impl_int, "m_iGoatKills", ddd_msg_key::GOAT_KILLS);
		MAKE_WATCH_FROM_PROP_NAME(playerClass, watch_impl_int, "m_iHealPoints", ddd_msg_key::HEAL_POINTS);
	}
	else {
		printf("ddd_admin:ERROR // Failed to find player class.\n");
	}
}

void serialize_player(ddd_game_player* player, ddd_msg_writer* packet) {
	//General info
	packet->put_int(ddd_msg_key::USER_ID, player->get_client_id());
	packet->put_string(ddd_msg_key::NAME, player->get_info()->GetName());
	packet->put_string(ddd_msg_key::STEAM_ID, player->get_info()->GetNetworkIDString());
	packet->put_string(ddd_msg_key::IP_ADDRESS, "");

	//Props
	packet->put_int(ddd_msg_key::TEAM, player->get_info()->GetTeamIndex());
}

void ddd_game::server_periodic_tick() {
	ddd_game_player_collection::server_periodic_tick();

	//Check if the boot packet should be sent
	if (dddNetwork->is_awaiting_boot_msg()) {
		//Create and add general bits
		ddd_packet_outgoing* packet = dddNetwork->msg_create(ddd_msg_endpoint::BOOT);
		packet->put_string(ddd_msg_key::PLUGIN_VERSION, DDD_PLUGIN_VERSION);
		packet->put_string(ddd_msg_key::PLUGIN_COMPILE_DATE, __DATE__);
		packet->put_string(ddd_msg_key::NAME, get_server_name());
		packet->put_int(ddd_msg_key::MAX_PLAYERS, gpGlobals->maxClients);
		packet->put_string(ddd_msg_key::MAP_NAME, gpGlobals->mapname.ToCStr());

		//Add players and some basic data
		{
			ddd_game_player* cursor = 0;
			ddd_msg_writer_array playerDataArray;
			while (enumerate_players(&cursor)) {
				ddd_msg_writer playerData;
				serialize_player(cursor, &playerData);
				playerDataArray.put(playerData);
			}
			packet->put_array(ddd_msg_key::PLAYERS, playerDataArray);
		}

		//Attach all CVars
		{
			ddd_msg_writer_array cvarArray;
			serialize_cvars_list(&cvarArray);
			packet->put_array(ddd_msg_key::CVARS, cvarArray);
		}

		//Send
		dddNetwork->msg_send(packet, true);

		//Force refresh player props
		force_refresh_watches();
	}
}

bool ddd_game::on_level_init(char const* pMapName, char const* pMapEntities, char const* pOldLevel, char const* pLandmarkName, bool loadGame, bool background) {
	level_active = true;
	ddd_packet_outgoing* packet = dddNetwork->msg_create(ddd_msg_endpoint::MAP_START);
	packet->put_string(ddd_msg_key::MAP_NAME, pMapName);
	//pLandmarkName specifies the mode for the first time...but not after that?
	dddNetwork->msg_send(packet);
	RETURN_META_VALUE(MRES_IGNORED, false);
}

void ddd_game::on_level_shutdown() {
	//For some reason, this seems to be called twice. Janky hack to make it only happen once per level, but it works...
	if (!level_active)
		return;

	//Set state and send
	level_active = false;
	ddd_packet_outgoing* packet = dddNetwork->msg_create(ddd_msg_endpoint::MAP_END);
	dddNetwork->msg_send(packet);
}

void ddd_game::player_connected(ddd_game_player* player) {
	ddd_packet_outgoing* packet = dddNetwork->msg_create(ddd_msg_endpoint::PLAYER_CONNECT);
	serialize_player(player, packet);
	dddNetwork->msg_send(packet);
}

void ddd_game::player_disconnected(ddd_game_player* player) {
	ddd_packet_outgoing* packet = dddNetwork->msg_create(ddd_msg_endpoint::PLAYER_DISCONNECT);
	packet->put_int(ddd_msg_key::USER_ID, player->get_client_id());
	dddNetwork->msg_send(packet);
}

void ddd_game::on_cvar_set_client(int client) {
	last_command_client = client + 1; //I don't know why +1 is needed, but it is...
}

bool ddd_game::get_cvar_command_sender(ddd_game_player** result) {
	//If the last client was this, it was the console. Silent ignore it...
	if (last_command_client == 0)
		return false;

	//Find sender
	if (!find_player_by_edict(EDICT_OF_INDEX(last_command_client), result)) {
		printf("ddd_admin:ERROR // Failed to find ConCommand sender! This is a bug. index=%i\n", last_command_client);
		return false;
	}

	return true;
}

void ddd_game::on_chat_command(const CCommand& command) {
	GET_CVAR_SENDER();

	//Get fields...
	const char* cmd = command.Arg(0);
	const char* message = command.Arg(1);
	if (cmd == 0 || message == 0)
		return;

	//Check if this is a team command or not
	bool isTeam = strcmp(cmd, "say_team") == 0;

	//Send
	ddd_packet_outgoing* packet = dddNetwork->msg_create(ddd_msg_endpoint::PLAYER_CHAT);
	packet->put_int(ddd_msg_key::USER_ID, player->get_client_id());
	packet->put_int(ddd_msg_key::TEAM_ONLY, isTeam);
	packet->put_string(ddd_msg_key::MESSAGE, message);
	dddNetwork->msg_send(packet);
}

void ddd_game::serialize_cvars_list(ddd_msg_writer_array* result) {
	ICvar::Iterator iter(cvarmanager);
	ConCommandBase* convar;
	for (iter.SetFirst(); iter.IsValid(); iter.Next()) {
		//Get writer and convar
		ddd_msg_writer writer;
		convar = iter.Get();

		//Add generals and start making flags
		writer.put_string(ddd_msg_key::NAME, convar->GetName());
		writer.put_string(ddd_msg_key::HELP, convar->GetHelpText());
		uint16_t flags = convar->IsCommand(); //0: IsCommand; 2: HasMax; 3: HasMin

		//Switch depending on type
		if (convar->IsCommand()) {
			//Treat as command and do special stuff
		}
		else {
			//Treat a variable and do special stuff
			ConVar* var = (ConVar*)convar;

			writer.put_string(ddd_msg_key::VALUE, var->GetString());
			if (var->HasMax()) {
				flags |= (1 << 1);
				writer.put_float(ddd_msg_key::MAX, var->GetMaxValue());
			}
			if (var->HasMin()) {
				flags |= (1 << 2);
				writer.put_float(ddd_msg_key::MIN, var->GetMinValue());
			}
		}

		//Finally, put flags and put it into the buffer
		writer.put_short(ddd_msg_key::FLAGS, flags);
		result->put(writer);
	}
}