#pragma once

#include <ddd_abstraction.h>
#include <ddd_net_client.h>

class ddd_hook_chat : public IDddAbsCvarHookCallback {
public:
	ddd_hook_chat(DddNetClient* client) { this->client = client; }
	virtual void on_command(IDddAbsPlayer* player, const CCommand& command);
private:
	DddNetClient* client;
};

class ddd_hook_watch : public IDddAbsWatchCallback {

public:
	ddd_hook_watch(DddNetClient* client, DddNetOpcode key) { this->client = client; this->key = key; }
	virtual void on_value_changed(IDddAbsPlayer* player, void* value, size_t value_size) override;
private:
	DddNetClient* client;
	DddNetOpcode key;

};

class ddd_hook_watch_aprilfools : public IDddAbsWatchCallback {

public:
	ddd_hook_watch_aprilfools(IDddAbsCore* game) { this->game = game; }
	virtual void on_value_changed(IDddAbsPlayer* player, void* value, size_t value_size) override;
	IDddAbsCore* game;

};