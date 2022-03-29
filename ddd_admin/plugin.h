// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#pragma once

#include <ddd_abstraction.h>
#include <ddd_net_client.h>
#include "callbacks.h"
#include "config.h"

#define DDD_PLUGIN_VERSION "1.0.0.1"

class ddd_plugin : public ISmmPlugin, IDddAbsCallbacks, IDddNetClientCallbacks
{
public:
	ddd_plugin();
	bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late);
	bool Unload(char* error, size_t maxlen);
	bool Pause(char* error, size_t maxlen);
	bool Unpause(char* error, size_t maxlen);
	void AllPluginsLoaded();
public:
	virtual void on_player_connect(IDddAbsPlayer* player) override;
	virtual void on_player_disconnect(IDddAbsPlayer* player) override;
	virtual void on_tick() override;
	virtual void on_level_init(const char* mapName) override;
	virtual void on_level_shutdown() override;
	virtual void on_net_connected() override;
	virtual void on_net_disconnected() override;
public:
	const char* GetAuthor() {		return "RomanPort"; }
	const char* GetName() {			return "Dino D-Day Web Panel by RomanPort"; }
	const char* GetDescription() {	return "A plugin for controlling Dino D-Day from the web."; }
	const char* GetURL() {			return "https://dinotown.net/"; }
	const char* GetLicense() {		return "N/A"; };
	const char* GetVersion() {		return DDD_PLUGIN_VERSION; };
	const char* GetDate() {			return __DATE__; }
	const char* GetLogTag() {		return "ddd_admin"; }
private:
	void send_init_message();
private:
	ddd_admin_config_t config;

	IDddAbsCore* game;
	DddNetClient* client;

	ddd_hook_chat cb_chat;
	ddd_hook_watch cb_team;
	ddd_hook_watch cb_cls;
	ddd_hook_watch cb_kills;
	ddd_hook_watch cb_deaths;
	ddd_hook_watch cb_goat_kills;
	ddd_hook_watch cb_heal_points;

	IDddAbsCvar* cvar_say;
	IDddAbsCvar* cvar_sayteam;

};

extern ddd_plugin g_ddd_plugin;
