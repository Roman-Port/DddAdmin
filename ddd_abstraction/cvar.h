#pragma once

#include "include/ddd_abstraction.h"

#define DDD_CVAR_MAX_HOOKS 8

class DddAbsCvar : public IDddAbsCvar {

public:
	DddAbsCvar(IDddAbsCore* core, ConCommand* command);
	~DddAbsCvar();
	virtual bool hook(IDddAbsCvarHookCallback* callback) override;

private:
	IDddAbsCore* core;
	ConCommand* command;
	bool hooks_set;
	int latest_command_client;
	IDddAbsCvarHookCallback* hook_callbacks[DDD_CVAR_MAX_HOOKS];

	void hook_command_client_changed(int client);
	void hook_command_dispatched(const CCommand& command);

};