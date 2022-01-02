// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#pragma once
#include <stdint.h>
#include "game_properties.h"

#define DDD_GAME_PLAYER_MAX_CLIENTS 32
#define DDD_GAME_PLAYER_MAX_WATCHES 32

class ddd_game_player {

public:
	ddd_game_player();
	void init(edict_t* edict);
	void clear();

	inline bool is_valid() { return pEdict != 0; }
	inline int get_client_id() { return client_id; }
	inline edict_t* get_edict() { return pEdict; }
	inline IPlayerInfo* get_info() { return pInfo; }

	uint8_t* get_property_data(ddd_game_property_search_result& data);

private:
	int client_id;
	edict_t* pEdict;
	IPlayerInfo* pInfo;
	CBaseEntity* pEntity;

};

class ddd_game_player_watch {

public:
	ddd_game_player_watch(ddd_game_property_search_result propInfo, size_t propSize);
	void process(ddd_game_player* player, int playerIndex, bool forceUpdate = false);

	ddd_game_property_search_result prop_info;
	size_t prop_size;

protected:
	virtual void value_changed(ddd_game_player* player, SendProp* prop, void* value) = 0;

private:
	uint8_t* challenge; // size is prop_size * DDD_GAME_PLAYER_MAX_CLIENTS

};

class ddd_game_player_collection {

public:
	ddd_game_player_collection();

	virtual void init();
	void add_watch(ddd_game_player_watch* watch);
	void force_refresh_watches();

	bool enumerate_players(ddd_game_player** cursor); //Start cursor at 0 and call in a while loop
	bool find_player_by_edict(edict_t* query, ddd_game_player** result);
	bool find_player_by_client_id(int query, ddd_game_player** result);

protected:
	virtual void player_connected(ddd_game_player* player) {} //Consumers can override this if they want
	virtual void player_disconnected(ddd_game_player* player) {} //Consumers can override this if they want
	virtual void server_periodic_tick();

private:
	void scan(bool forceUpdate);
	void on_server_tick(bool finalTick);
	void on_player_connect(edict_t* player);
	void on_player_disconnect(edict_t* player);

	ddd_game_player players[DDD_GAME_PLAYER_MAX_CLIENTS];
	ddd_game_player_watch* watches[DDD_GAME_PLAYER_MAX_WATCHES];
	int watches_count;

	int periodic_wait;

};