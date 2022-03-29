#include "interfaces.h"

#include <ISmmAPI.h>

IServerGameDLL* server = NULL;
IServerGameClients* gameclients = NULL;
IVEngineServer* engine = NULL;
IServerPluginHelpers* helpers = NULL;
IGameEventManager2* gameevents = NULL;
IGameEventManager* gameevents_legacy = NULL;
IPlayerInfoManager* playerinfomanager = NULL;
ICvar* cvarmanager = NULL;
IServerGameEnts* gameEntities = NULL;
CGlobalVars* gpGlobals = NULL;

bool init_interfaces(ISmmAPI* ismm, char* error, size_t maxlen) {
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);
	GET_V_IFACE_CURRENT(GetEngineFactory, helpers, IServerPluginHelpers, INTERFACEVERSION_ISERVERPLUGINHELPERS);
	GET_V_IFACE_ANY(GetServerFactory, gameEntities, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_ANY(GetEngineFactory, cvarmanager, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_CURRENT(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetServerFactory, playerinfomanager, IPlayerInfoManager, INTERFACEVERSION_PLAYERINFOMANAGER);
	gpGlobals = ismm->GetCGlobals();
	return true;
}

int index_of_edict(edict_t* entity)
{
	return (int)(entity - gpGlobals->pEdicts);
}