#pragma once

#include <ISmmPlugin.h>
#include <igameevents.h>
#include <iplayerinfo.h>
#include <sh_vector.h>

extern IServerGameDLL* server;
extern IServerGameClients* gameclients;
extern IVEngineServer* engine;
extern IServerPluginHelpers* helpers;
extern IGameEventManager2* gameevents;
extern IGameEventManager* gameevents_legacy;
extern IPlayerInfoManager* playerinfomanager;
extern ICvar* cvarmanager;
extern IServerGameEnts* gameEntities;
extern CGlobalVars* gpGlobals;

bool init_interfaces(ISmmAPI* ismm, char* error, size_t maxlen);
int index_of_edict(edict_t* entity);