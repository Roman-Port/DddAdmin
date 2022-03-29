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
	cb_heal_points(client, DddNetOpcode::HEAL_POINTS)
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
	//Attempt to dequeue a message
	DddNetMsg packet;
	DddNetEndpoint endpoint;
	if (client->dequeue_incoming(&endpoint, packet)) {
		switch (endpoint) {
		case DddNetEndpoint::CONNECTION_INIT: send_init_message(); break;
		}
	}
}

void ddd_plugin::on_player_connect(IDddAbsPlayer* player)
{
	//Send info to server
	DddNetMsg packet;
	packet.put_int(DddNetOpcode::USER_ID, player->get_index());
	packet.put_string(DddNetOpcode::NAME, player->get_info()->GetName());
	packet.put_string(DddNetOpcode::STEAM_ID, player->get_info()->GetNetworkIDString());
	packet.put_string(DddNetOpcode::IP_ADDRESS, "");
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

bool ddd_plugin::Pause(char* error, size_t maxlen)
{
	return true;
}

bool ddd_plugin::Unpause(char* error, size_t maxlen)
{
	return true;
}