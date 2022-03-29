#pragma once

#include "include/ddd_abstraction.h"
#include "watch.h"
#include "player.h"

#define MAX_WATCH_SLOTS 8
#define FRAMES_PER_WATCH 128

class DddAbsCore : public IDddAbsCore {

public:
	DddAbsCore(IDddAbsCallbacks* callbacks);
	~DddAbsCore();

	/* API */

	virtual const char* get_server_name() override;
	virtual const char* get_level_name() override;
	virtual int get_max_clients() override;
	virtual bool find_player_by_index(int index, IDddAbsPlayer** result) override;
	virtual bool enumerate_players(IDddAbsPlayer** cursor) override;
	virtual bool create_property_watch(const char* classname, const char* propertyname, IDddAbsWatchCallback* callback) override;
	virtual bool find_cvar(const char* name, IDddAbsCvar** result) override;
	virtual void reset_watches() override;

private:
	IDddAbsCallbacks* callbacks;
	DddAbsPlayer players[MAX_PLAYER_SLOTS];
	DddAbsWatch* watches[MAX_WATCH_SLOTS];
	int watches_count;
	int watch_timer;
	bool level_active;
	char level_name[64];

	void server_tick(bool finalTick);
	void player_connect(edict_t* player);
	void player_disconnect(edict_t* player);
	bool level_init(char const* pMapName, char const* pMapEntities, char const* pOldLevel, char const* pLandmarkName, bool loadGame, bool background);
	void level_shutdown();

};