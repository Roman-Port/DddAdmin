// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#pragma once

#include "game.h"
#include "ddd_defines.h"

class ddd_plugin : public ISmmPlugin
{
public:
	ddd_plugin();
	bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late);
	bool Unload(char* error, size_t maxlen);
	bool Pause(char* error, size_t maxlen);
	bool Unpause(char* error, size_t maxlen);
	void AllPluginsLoaded();
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
	ddd_game game;

};

extern ddd_plugin g_ddd_plugin;
