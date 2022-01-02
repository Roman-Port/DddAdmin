// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#pragma once

#include <ISmmPlugin.h>
#include <igameevents.h>
#include <iplayerinfo.h>
#include <sh_vector.h>

/* SETTINGS */

//#define DDD_ENABLE_CVAR_MONITORING

#define DDD_PERIODIC_WAIT_PERIOD 50
#define DDD_PLUGIN_VERSION "1.0.0.1"

/* SOURCE ENGINE STUFF */

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

PLUGIN_GLOBALVARS();

#define INDEX_OF_EDICT(edict) ((int)((edict) - gpGlobals->pEdicts))
#define EDICT_OF_INDEX(index) (gpGlobals->pEdicts + (index))

/* DDD ADMIN STUFF */

class ddd_net;

extern ddd_net* dddNetwork;