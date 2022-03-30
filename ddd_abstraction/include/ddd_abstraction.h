#pragma once

#include <ISmmAPI.h>
#include <iplayerinfo.h>

PLUGIN_GLOBALVARS();

#define MAX_PLAYER_SLOTS 64

class IDddAbsPlayer {

public:
	virtual bool is_valid() = 0;
	virtual int get_index() = 0;
	virtual int get_uniqueid() = 0;
	virtual IPlayerInfo* get_info() = 0;
	virtual CBaseEntity* get_entity() = 0;
	virtual const char* get_ip_address() = 0;
	virtual void kick(const char* message) = 0;

};

class IDddAbsWatchCallback {

public:
	virtual void on_value_changed(IDddAbsPlayer* player, void* value, size_t value_size) = 0;

};

class IDddAbsCvarHookCallback {

public:
	virtual void on_command(IDddAbsPlayer* player, const CCommand& command) = 0;

};

class IDddAbsCvar {

public:
	virtual bool hook(IDddAbsCvarHookCallback* callback) = 0;

};

class IDddAbsCore {

public:
	virtual const char* get_level_name() = 0;
	virtual const char* get_server_name() = 0;
	virtual int get_max_clients() = 0;
	virtual bool find_player_by_index(int index, IDddAbsPlayer** result) = 0;
	virtual bool enumerate_players(IDddAbsPlayer** cursor) = 0;
	virtual bool create_property_watch(const char* classname, const char* propertyname, IDddAbsWatchCallback* callback) = 0;
	virtual bool find_cvar(const char* name, IDddAbsCvar** result) = 0;
	virtual void reset_watches() = 0;
	virtual void run_command(const char* command) = 0;
	virtual void change_map(const char* map, const char* mode) = 0;

};

class IDddAbsCallbacks {

public:
	virtual void on_player_connect(IDddAbsPlayer* player) = 0;
	virtual void on_player_disconnect(IDddAbsPlayer* player) = 0;
	virtual void on_tick() = 0;
	virtual void on_level_init(const char* mapName) = 0;
	virtual void on_level_shutdown() = 0;

};

IDddAbsCore* dddabs_init(IDddAbsCallbacks* callbacks, ISmmAPI* ismm, char* error, size_t maxlen);