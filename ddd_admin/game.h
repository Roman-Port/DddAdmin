// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#pragma once

#include "net.h"
#include "io.h"
#include "ddd_defines.h"
#include "game_cvar_monitoring.h"
#include "game_players.h"

class ddd_game : public ddd_game_player_collection {

public:
	ddd_game();

	virtual void init() override;

	bool get_cvar_command_sender(ddd_game_player** result);
	
protected:
	virtual void player_connected(ddd_game_player* player) override;
	virtual void player_disconnected(ddd_game_player* player) override;
	virtual void server_periodic_tick() override;

private:
	void serialize_cvars_list(ddd_msg_writer_array* result);
	bool on_level_init(char const* pMapName, char const* pMapEntities, char const* pOldLevel, char const* pLandmarkName, bool loadGame, bool background);
	void on_level_shutdown();
	void on_cvar_set_client(int client);

	void on_chat_command(const CCommand& command);

private:
	ddd_game_cvar_monitoring cvar_monitoring;
	int last_command_client;
	bool level_active;

};

