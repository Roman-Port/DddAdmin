// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#include <stdio.h>
#include <sourcehook/sourcehook.h>
#include "plugin.h"

#define MAX_QUEUE_SIZE 256

ddd_plugin g_ddd_plugin;

ddd_plugin::ddd_plugin() :
	client(new DddNetClient(this, MAX_QUEUE_SIZE)),
	cb_chat(client),
	cb_team(client, DddNetOpcode::TEAM),
	cb_cls(client, DddNetOpcode::PLAYER_CLASS),
	cb_kills(client, DddNetOpcode::KILLS),
	cb_deaths(client, DddNetOpcode::DEATHS),
	cb_goat_kills(client, DddNetOpcode::GOAT_KILLS),
	cb_heal_points(client, DddNetOpcode::HEAL_POINTS),
	stat_max_frame_time(0),
	stat_elapsed_frames(0)
{

}

PLUGIN_EXPOSE(ddd_plugin, g_ddd_plugin);
bool ddd_plugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	//Log
	printf("ddd_admin:INFO // Initializing plugin...\n");

	//Attempt to read config
	if (!ddd_load_admin_config(ismm, &config)) {
		strcpy_s(error, maxlen, "Failed to read config file.");
		return false;
	}

	//Create game
	game = dddabs_init(this, ismm, error, maxlen);
	if (!game)
		return false;

	//Add watches
	game->create_property_watch("CDDDPlayer", "m_iTeamNum", &cb_team);
	game->create_property_watch("CDDDPlayer", "m_iPlayerClass", &cb_cls);
	game->create_property_watch("CDDDPlayer", "m_iKills", &cb_kills);
	game->create_property_watch("CDDDPlayer", "m_iDeaths", &cb_deaths);
	game->create_property_watch("CDDDPlayer", "m_iGoatKills", &cb_goat_kills);
	game->create_property_watch("CDDDPlayer", "m_iHealPoints", &cb_heal_points);

	//Find say CVar
	if (game->find_cvar("say", &cvar_say))
		cvar_say->hook(&cb_chat);
	else
		printf("ddd_admin:WARN // Failed to find CVar \"say\".\n");

	//Find say_team CVar
	if (game->find_cvar("say_team", &cvar_sayteam))
		cvar_sayteam->hook(&cb_chat);
	else
		printf("ddd_admin:WARN // Failed to find CVar \"say_team\".\n");

	//Go online
	client->init(config.server_ip, config.server_port, config.server_psk);

	return true;
}

bool ddd_plugin::Unload(char* error, size_t maxlen)
{
	//Log
	printf("ddd_admin:INFO // Unloading plugin...\n");
	return false;
}

void ddd_plugin::AllPluginsLoaded()
{

}

void ddd_plugin::on_tick()
{
	//Record the time since the last frame
	double lastFrameTime = stat_last_frame.elapsed_seconds();
	stat_last_frame.reset();
	stat_elapsed_frames++;
	if (lastFrameTime > stat_max_frame_time)
		stat_max_frame_time = lastFrameTime;

	//Periodically send stats
	double lastStatTime = stat_last_update.elapsed_seconds();
	if (lastStatTime > 60) {
		//Create and send message
		DddNetMsg packet;
		packet.put_double(DddNetOpcode::AVG_FRAME_DURATION, lastStatTime / stat_elapsed_frames);
		packet.put_double(DddNetOpcode::MAX_FRAME_DURATION, stat_max_frame_time);
		client->enqueue_outgoing(DddNetEndpoint::SERVER_REPORT_STAT, packet);

		//Reset
		stat_last_update.reset();
		stat_max_frame_time = 0;
		stat_elapsed_frames = 0;
	}

	//Attempt to dequeue a message
	try {
		DddNetMsg packet;
		DddNetEndpoint endpoint;
		if (client->dequeue_incoming(&endpoint, packet)) {
			switch (endpoint) {
			case DddNetEndpoint::CONNECTION_INIT: send_init_message(); break;
			case DddNetEndpoint::PLAYER_KICK: handle_kick(&packet); break;
			case DddNetEndpoint::MAP_CHANGE: handle_map_change(&packet); break;
			case DddNetEndpoint::SERVER_COMMAND: handle_console_command(&packet); break;
			}
		}
	}
	catch (std::exception ex) {
		printf("ddd_admin:WARN // Got exception handling tick: %s\n", ex.what());
	}
}

void ddd_plugin::on_player_connect(IDddAbsPlayer* player)
{
	//Send info to server
	DddNetMsg packet;
	packet.put_int(DddNetOpcode::USER_ID, player->get_index());
	packet.put_string(DddNetOpcode::NAME, player->get_info()->GetName());
	packet.put_string(DddNetOpcode::STEAM_ID, player->get_info()->GetNetworkIDString());
	packet.put_string(DddNetOpcode::IP_ADDRESS, player->get_ip_address());
	packet.put_int(DddNetOpcode::TEAM, player->get_info()->GetTeamIndex());
	client->enqueue_outgoing(DddNetEndpoint::PLAYER_CONNECT, packet);
}

void ddd_plugin::on_player_disconnect(IDddAbsPlayer* player)
{
	//Send info to server
	DddNetMsg packet;
	packet.put_int(DddNetOpcode::USER_ID, player->get_index());
	client->enqueue_outgoing(DddNetEndpoint::PLAYER_DISCONNECT, packet);
}

void ddd_plugin::on_level_init(const char* mapName)
{
	//Send info to server
	DddNetMsg packet;
	packet.put_string(DddNetOpcode::MAP_NAME, mapName);
	client->enqueue_outgoing(DddNetEndpoint::MAP_START, packet);
}

void ddd_plugin::on_level_shutdown()
{
	//Send info to server
	DddNetMsg packet;
	client->enqueue_outgoing(DddNetEndpoint::MAP_END, packet);
}

void ddd_plugin::on_net_connected()
{
}

void ddd_plugin::on_net_disconnected()
{
}

void ddd_plugin::send_init_message()
{
	//Create and send init message
	DddNetMsg packet;
	packet.put_string(DddNetOpcode::PLUGIN_VERSION, DDD_PLUGIN_VERSION);
	packet.put_string(DddNetOpcode::PLUGIN_COMPILE_DATE, __DATE__);
	packet.put_string(DddNetOpcode::NAME, game->get_server_name());
	packet.put_int(DddNetOpcode::MAX_PLAYERS, game->get_max_clients());
	packet.put_string(DddNetOpcode::MAP_NAME, game->get_level_name());
	client->enqueue_outgoing(DddNetEndpoint::CONNECTION_INIT, packet);

	//Send connect events for all clients
	IDddAbsPlayer* cursor = 0;
	while (game->enumerate_players(&cursor)) {
		on_player_connect(cursor);
	}

	//Reset watches so they get resent
	game->reset_watches();
}

void ddd_plugin::handle_kick(DddNetMsg* message)
{
	//Get the info
	int ackId;
	int clientId;
	char clientSteamId[64];
	char kickMessage[512];
	if (!message->get_int(DddNetOpcode::ACK_ID, &ackId) ||
		!message->get_int(DddNetOpcode::USER_ID, &clientId) ||
		!message->get_string(DddNetOpcode::STEAM_ID, clientSteamId, sizeof(clientSteamId)) ||
		!message->get_string(DddNetOpcode::KICK_REASON, kickMessage, sizeof(kickMessage)))
	{
		printf("ddd_admin:WARN // Failed to get required properties in incoming PLAYER_KICK message.\n");
		return;
	}

	//Get the player
	int result;
	IDddAbsPlayer* player;
	if (!game->find_player_by_index(clientId, &player)) {
		printf("ddd_admin:WARN // Failed to find player to kick from incoming kick message!\n");
		result = 1;
	}
	else {
		//Sanity check, just to make sure
		if (strcmp(clientSteamId, player->get_info()->GetNetworkIDString()) != 0) {
			printf("ddd_admin:WARN // Failed to sanity check player Steam ID to kick! Expected %s, got %s!\n", clientSteamId, player->get_info()->GetNetworkIDString());
			result = 2;
		}
		else {
			//Perform the kick
			player->kick(kickMessage);
			result = 0;
		}
	}

	//Send ACK back
	send_ack(ackId, result);
}

void ddd_plugin::handle_map_change(DddNetMsg* message)
{
	//Get the info
	int ackId;
	char mapName[64];
	char mapMode[16];
	if (!message->get_int(DddNetOpcode::ACK_ID, &ackId) ||
		!message->get_string(DddNetOpcode::MAP_NAME, mapName, sizeof(mapName)) ||
		!message->get_string(DddNetOpcode::MAP_MODE, mapMode, sizeof(mapMode)))
	{
		printf("ddd_admin:WARN // Failed to get required properties in incoming MAP_CHANGE message.\n");
		return;
	}

	//Update
	game->change_map(mapName, mapMode);

	//Send ACK
	send_ack(ackId, 0);
}

void ddd_plugin::handle_console_command(DddNetMsg* message)
{
	//Get the info
	int ackId;
	char command[512];
	if (!message->get_int(DddNetOpcode::ACK_ID, &ackId) ||
		!message->get_string(DddNetOpcode::COMMAND, command, sizeof(command)))
	{
		printf("ddd_admin:WARN // Failed to get required properties in incoming SERVER_COMMAND message.\n");
		return;
	}

	//Update
	game->run_command(command);

	//Send ACK
	send_ack(ackId, 0);
}

void ddd_plugin::send_ack(int ackId, int result)
{
	DddNetMsg packet;
	packet.put_int(DddNetOpcode::ACK_ID, ackId);
	packet.put_int(DddNetOpcode::ACK_RESULT, result);
	client->enqueue_outgoing(DddNetEndpoint::CONNECTION_REQUEST_ACK, packet);
}

bool ddd_plugin::Pause(char* error, size_t maxlen)
{
	return true;
}

bool ddd_plugin::Unpause(char* error, size_t maxlen)
{
	return true;
}