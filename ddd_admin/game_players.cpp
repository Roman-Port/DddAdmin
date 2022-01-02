// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#include "game_players.h"

/* PLAYER */

ddd_game_player::ddd_game_player() {
	clear();
}

void ddd_game_player::init(edict_t* edict) {
	pEdict = edict;
	pInfo = playerinfomanager->GetPlayerInfo(edict);
	pEntity = edict->m_pNetworkable->GetBaseEntity();
	client_id = pInfo->GetUserID();
}

void ddd_game_player::clear() {
	client_id = 0;
	pEdict = 0;
	pInfo = 0;
	pEntity = 0;
}

uint8_t* ddd_game_player::get_property_data(ddd_game_property_search_result& data) {
	return ((uint8_t*)pEntity) + data.actual_offset;
}

/* COLLECTION */

SH_DECL_HOOK1_void(IServerGameClients, ClientFullyConnect, SH_NOATTRIB, 0, edict_t*);
SH_DECL_HOOK1_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, edict_t*);
SH_DECL_HOOK1_void(IServerGameDLL, Think, SH_NOATTRIB, 0, bool);

ddd_game_player_collection::ddd_game_player_collection() : periodic_wait(0) {
	//Initialize the players by clearing all of them
	for (int i = 0; i < DDD_GAME_PLAYER_MAX_CLIENTS; i++)
		players[i].clear();

	//Initialize watches by setting them all to zero
	memset(watches, 0, sizeof(watches));
	watches_count = 0;
}

void ddd_game_player_collection::init() {
	SH_ADD_HOOK(IServerGameDLL, Think, server, SH_MEMBER(this, &ddd_game_player_collection::on_server_tick), true);
	SH_ADD_HOOK(IServerGameClients, ClientFullyConnect, gameclients, SH_MEMBER(this, &ddd_game_player_collection::on_player_connect), true);
	SH_ADD_HOOK(IServerGameClients, ClientDisconnect, gameclients, SH_MEMBER(this, &ddd_game_player_collection::on_player_disconnect), true);
}

void ddd_game_player_collection::add_watch(ddd_game_player_watch* watch) {
	//Sanity check
	if (watches_count >= DDD_GAME_PLAYER_MAX_WATCHES) {
		printf("ddd_admin:ERROR // Too many property watches added! Consider bumping DDD_GAME_PLAYER_MAX_WATCHES...\n");
		return;
	}

	//Log
	printf("ddd_admin:DEBUG // Added property watch for %s (offset %i, length %i).\n", watch->prop_info.prop->GetName(), watch->prop_info.actual_offset, watch->prop_size);

	//Add
	watches[watches_count++] = watch;
}

bool ddd_game_player_collection::enumerate_players(ddd_game_player** cursor) {
	//If cursor is 0, initialize it to INDEX 0...otherwise add 1
	if ((*cursor) == 0)
		(*cursor) = players;
	else
		(*cursor) += 1;

	//Get the end of array
	ddd_game_player* end = &players[DDD_GAME_PLAYER_MAX_CLIENTS];

	//Dirty sanity check
	if ((*cursor) < players || (*cursor) >= end) {
		printf("ddd_admin:WARN // The ddd_game_player_collection enumeration pointer was not properly initialized to zero. Fixing, but this is a bug!\n");
		(*cursor) = players;
	}

	//Search
	while ((*cursor) < end) {
		//If this is valid, return it
		if ((*cursor)->is_valid())
			return true;

		//Advance
		(*cursor) += 1;
	}

	return false;
}

bool ddd_game_player_collection::find_player_by_edict(edict_t* query, ddd_game_player** result) {
	//Initialize cursor and enumerate
	(*result) = 0;
	while (enumerate_players(result)) {
		//Check if this matches
		if ((*result)->get_edict() == query)
			return true;
	}
	return false;
}

bool ddd_game_player_collection::find_player_by_client_id(int query, ddd_game_player** result) {
	//Initialize cursor and enumerate
	(*result) = 0;
	while (enumerate_players(result)) {
		//Check if this matches
		if ((*result)->get_client_id() == query)
			return true;
	}
	return false;
}

void ddd_game_player_collection::on_server_tick(bool finalTick) {
	//It's impractical to check EVERY frame, so only do it periodically...
	if (periodic_wait == DDD_PERIODIC_WAIT_PERIOD) {
		server_periodic_tick();
		periodic_wait = 0;
	}
	else {
		periodic_wait++;
	}
}

void ddd_game_player_collection::force_refresh_watches() {
	scan(true);
}

void ddd_game_player_collection::server_periodic_tick() {
	scan(false);
}

void ddd_game_player_collection::scan(bool forceRefresh) {
	//Loop through players...
	for (int i = 0; i < DDD_GAME_PLAYER_MAX_CLIENTS; i++) {
		//Skip unused slots
		if (!players[i].is_valid())
			continue;

		//Loop through watches
		for (int j = 0; j < watches_count; j++)
			watches[j]->process(&players[i], i, forceRefresh);
	}
}

void ddd_game_player_collection::on_player_connect(edict_t* player) {
	//Search for a free slot
	for (int i = 0; i < DDD_GAME_PLAYER_MAX_CLIENTS; i++) {
		if (!players[i].is_valid()) {
			//Fill in details
			players[i].init(player);

			//Notify consumers
			player_connected(&players[i]);

			//Force all watches to reset for properties
			for (int j = 0; j < watches_count; j++)
				watches[j]->process(&players[i], i, true);

			return;
		}
	}

	//No free slots!
	printf("ddd_admin:ERROR // Failed to find slot to put player in ddd_game_player_collection! This is a bug.\n");
}

void ddd_game_player_collection::on_player_disconnect(edict_t* player) {
	//Search for the slot this player was in
	for (int i = 0; i < DDD_GAME_PLAYER_MAX_CLIENTS; i++) {
		if (players[i].is_valid() && players[i].get_edict() == player) {
			//Log
			printf("ddd_admin:DEBUG // Removing disconnected player from slot %i.\n", i);

			//Notify consumers
			player_disconnected(&players[i]);

			//Reset
			players[i].clear();

			return;
		}
	}

	//Slot not found!
	printf("ddd_admin:ERROR // Failed to find disconnecting player in collection! This is a bug.\n");
}