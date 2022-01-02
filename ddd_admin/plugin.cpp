// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#include <stdio.h>
#include <sourcehook/sourcehook.h>
#include "plugin.h"

IServerGameDLL* server = NULL;
IServerGameClients* gameclients = NULL;
IVEngineServer* engine = NULL;
IServerPluginHelpers* helpers = NULL;
IGameEventManager2* gameevents = NULL;
IGameEventManager* gameevents_legacy = NULL;
IPlayerInfoManager* playerinfomanager = NULL;
ICvar* cvarmanager = NULL;
CGlobalVars* gpGlobals = NULL;
IServerGameEnts* gameEntities = NULL;
ddd_plugin g_ddd_plugin;

ddd_net* dddNetwork = NULL;

ddd_plugin::ddd_plugin() {

}

PLUGIN_EXPOSE(ddd_plugin, g_ddd_plugin);
bool ddd_plugin::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	//Log
	printf("ddd_admin:INFO // Initializing plugin...\n");

	//Get parts
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_ANY(GetServerFactory, gameEntities, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_ANY(GetEngineFactory, cvarmanager, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_CURRENT(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, playerinfomanager, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);
	gpGlobals = ismm->GetCGlobals();

	//Create network
	dddNetwork = new ddd_net();

	//Bind
	game.init();

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

bool ddd_plugin::Pause(char* error, size_t maxlen)
{
	return true;
}

bool ddd_plugin::Unpause(char* error, size_t maxlen)
{
	return true;
}