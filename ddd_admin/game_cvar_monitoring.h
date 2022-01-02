#pragma once
#include "ddd_defines.h"

class ddd_game;

class ddd_game_cvar_monitoring {

public:
	ddd_game_cvar_monitoring(ddd_game* game) : game(game) {}

private:
	ddd_game* game;

#ifdef DDD_ENABLE_CVAR_MONITORING
public:
	void init();

private:
	void on_client_command(edict_t* pEntity, const CCommand& args);
	void on_cvar_dispatch(const CCommand& command);
	void on_cvar_set_char(const char* value);
	void on_cvar_set_float(float value);
	void on_cvar_set_int(int value);
	void on_cvar_set_color(Color value);
#endif
#ifndef DDD_ENABLE_CVAR_MONITORING
public:
	void init() {}
#endif 

};