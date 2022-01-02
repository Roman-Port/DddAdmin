// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#include "game_players.h"

ddd_game_player_watch::ddd_game_player_watch(ddd_game_property_search_result propInfo, size_t propSize) :
	prop_info(propInfo),
	prop_size(propSize),
	challenge(0)
{
	//Allocate and initialize the challenge data
	challenge = (uint8_t*)malloc(propSize * DDD_GAME_PLAYER_MAX_CLIENTS);
	memset(challenge, 0, propSize * DDD_GAME_PLAYER_MAX_CLIENTS);
}

void ddd_game_player_watch::process(ddd_game_player* player, int playerIndex, bool forceUpdate) {
	//Get the source data
	uint8_t* src = player->get_property_data(prop_info);

	//Get the challenge data
	uint8_t* dst = &challenge[prop_size * playerIndex];

	//Compare
	bool matching = memcmp(dst, src, prop_size) == 0;

	//Copy
	memcpy(dst, src, prop_size);

	//Notify if changed
	if (!matching || forceUpdate)
		value_changed(player, prop_info.prop, src);
}