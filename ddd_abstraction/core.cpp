#include "core.h"
#include "interfaces.h"
#include "cvar.h"
#include <cassert>

namespace DddAbsCoreHooks {
	SH_DECL_HOOK1_void(IServerGameClients, ClientFullyConnect, SH_NOATTRIB, 0, edict_t*);
	SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t*);
	SH_DECL_HOOK1_void(IServerGameDLL, Think, SH_NOATTRIB, 0, bool);
	SH_DECL_HOOK6(IServerGameDLL, LevelInit, SH_NOATTRIB, 0, bool, char const*, char const*, char const*, char const*, bool, bool);
	SH_DECL_HOOK0_void(IServerGameDLL, LevelShutdown, SH_NOATTRIB, 0);
}
using namespace DddAbsCoreHooks;

DddAbsCore::DddAbsCore(IDddAbsCallbacks* callbacks)
{
	//Set
	this->callbacks = callbacks;
	watches_count = 0;
	watch_timer = 0;
	level_active = false;
	level_name[0] = 0;

	//Add hooks
	SH_ADD_HOOK(IServerGameDLL, Think, server, SH_MEMBER(this, &DddAbsCore::server_tick), true);
	SH_ADD_HOOK(IServerGameClients, ClientFullyConnect, gameclients, SH_MEMBER(this, &DddAbsCore::player_connect), true);
	SH_ADD_HOOK(IServerGameClients, ClientDisconnect, gameclients, SH_MEMBER(this, &DddAbsCore::player_disconnect), true);
	SH_ADD_HOOK(IServerGameDLL, LevelInit, server, SH_MEMBER(this, &DddAbsCore::level_init), true);
	SH_ADD_HOOK(IServerGameDLL, LevelShutdown, server, SH_MEMBER(this, &DddAbsCore::level_shutdown), false);
}

DddAbsCore::~DddAbsCore()
{
}

IDddAbsCore* dddabs_init(IDddAbsCallbacks* callbacks, ISmmAPI* ismm, char* error, size_t maxlen)
{
	if (!init_interfaces(ismm, error, maxlen))
		return 0;
	return new DddAbsCore(callbacks);
}

void DddAbsCore::server_tick(bool finalTick)
{
	//Check if we need to update watches
	if (watch_timer++ >= FRAMES_PER_WATCH) {
		watch_timer = 0;
		IDddAbsPlayer* player = NULL;
		while (enumerate_players(&player)) {
			for (int i = 0; i < watches_count; i++)
				watches[i]->update(player);
		}
	}

	//Run user callback
	callbacks->on_tick();
}

void DddAbsCore::player_connect(edict_t* entity)
{
	//Get the client index
	int client = index_of_edict(entity);
	assert(client >= 0 && client < MAX_PLAYER_SLOTS);

	//Initialize
	players[client].connect(entity);

	//Send events
	callbacks->on_player_connect(&players[client]);
}

void DddAbsCore::player_disconnect(edict_t* entity)
{
	//Get the client index
	int client = index_of_edict(entity);
	assert(client >= 0 && client < MAX_PLAYER_SLOTS);

	//Send events
	callbacks->on_player_disconnect(&players[client]);

	//Destroy
	players[client].disconnect();

	//Reset watches
	for (int i = 0; i < watches_count; i++)
		watches[i]->clear(client);
}

bool DddAbsCore::level_init(char const* pMapName, char const* pMapEntities, char const* pOldLevel, char const* pLandmarkName, bool loadGame, bool background) {
	//Set flag
	level_active = true;

	//Copy level name
	strcpy_s(level_name, sizeof(level_name), pMapName);

	//Call callback
	callbacks->on_level_init(pMapName);

	RETURN_META_VALUE(MRES_IGNORED, false);
}

void DddAbsCore::level_shutdown() {
	//For some reason, this seems to be called twice. Janky hack to make it only happen once per level, but it works...
	if (!level_active)
		return;

	//Set flag
	level_active = false;
	
	//Call callback
	callbacks->on_level_shutdown();
}

const char* DddAbsCore::get_server_name()
{
	ConVar* var = cvarmanager->FindVar("hostname");
	if (var == NULL)
		return "";
	return var->GetString();
}

const char* DddAbsCore::get_level_name()
{
	return level_name;
}

int DddAbsCore::get_max_clients()
{
	return gpGlobals->maxClients;
}

bool DddAbsCore::find_player_by_index(int index, IDddAbsPlayer** result)
{
	(*result) = NULL;
	while (enumerate_players(result)) {
		if ((*result)->get_index() == index)
			return true;
	}
	return false;
}

bool DddAbsCore::enumerate_players(IDddAbsPlayer** cursor_raw)
{
	//Cast as the implimentation so we advance the correct number of bytes on each. This might be undefined behavior...
	DddAbsPlayer** cursor = (DddAbsPlayer**)cursor_raw;

	//Get last slot
	DddAbsPlayer* last = &players[MAX_PLAYER_SLOTS];

	//Initialize cursor
	if ((*cursor) == 0)
		(*cursor) = players;
	else
		(*cursor)++;

	//Find next
	while ((*cursor) < last) {
		if ((*cursor)->is_valid())
			return true;
		(*cursor)++;
	}

	return false;
}

bool DddAbsCore::create_property_watch(const char* classname, const char* propertyname, IDddAbsWatchCallback* callback) {
	if (watches_count < MAX_WATCH_SLOTS && DddAbsWatch::from_property(&watches[watches_count], callback, classname, propertyname)) {
		//Update counter
		watches_count++;

		return true;
	}
	else
	{
		return false;
	}
}

bool DddAbsCore::find_cvar(const char* name, IDddAbsCvar** result)
{
	//Find the command
	ConCommand* cmd = cvarmanager->FindCommand(name);
	if (cmd == NULL)
		return false;

	//Wrap
	(*result) = new DddAbsCvar(this, cmd);
	return true;
}

void DddAbsCore::reset_watches()
{
	for (int i = 0; i < watches_count; i++)
		watches[i]->clear_all();
}
