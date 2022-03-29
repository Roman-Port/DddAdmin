#include "cvar.h"
#include "interfaces.h"

SH_DECL_HOOK1_void(ConCommand, Dispatch, SH_NOATTRIB, 0, const CCommand&);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, false, int);

DddAbsCvar::DddAbsCvar(IDddAbsCore* core, ConCommand* command)
{
	this->core = core;
	this->command = command;
	hooks_set = false;
	latest_command_client = 0;
	for (int i = 0; i < DDD_CVAR_MAX_HOOKS; i++)
		hook_callbacks[i] = 0;
}

DddAbsCvar::~DddAbsCvar()
{
	//Remove hooks
	if (hooks_set) {
		SH_REMOVE_HOOK(IServerGameClients, SetCommandClient, gameclients, SH_MEMBER(this, &DddAbsCvar::hook_command_client_changed), false);
		SH_REMOVE_HOOK(ConCommand, Dispatch, command, SH_MEMBER(this, &DddAbsCvar::hook_command_dispatched), false);
		hooks_set = false;
	}
}

bool DddAbsCvar::hook(IDddAbsCvarHookCallback* callback)
{
	//Add to collection in an empty space
	bool added = false;
	for (int slot = 0; slot < DDD_CVAR_MAX_HOOKS; slot++) {
		if (hook_callbacks[slot] == 0) {
			hook_callbacks[slot] = callback;
			added = true;
			break;
		}
	}

	//Add hooks if needed
	if (!hooks_set) {
		SH_ADD_HOOK(IServerGameClients, SetCommandClient, gameclients, SH_MEMBER(this, &DddAbsCvar::hook_command_client_changed), false);
		SH_ADD_HOOK(ConCommand, Dispatch, command, SH_MEMBER(this, &DddAbsCvar::hook_command_dispatched), false);
		hooks_set = true;
	}

	return added;
}

void DddAbsCvar::hook_command_client_changed(int client)
{
	latest_command_client = client + 1;
}

void DddAbsCvar::hook_command_dispatched(const CCommand& command)
{
	//Determine the player that sent it (or if it was the console)
	IDddAbsPlayer* player = 0;
	if (latest_command_client != 0 && !core->find_player_by_index(latest_command_client, &player)) {
		//Should be a player specified, but there isn't one!
		printf("ddd_abs:WARN // CVar hook failed to find player responsible. Client ID %i doesn't match any connected players.\n", latest_command_client);
		return;
	}

	//Notify
	for (int i = 0; i < DDD_CVAR_MAX_HOOKS; i++) {
		if (hook_callbacks[i] != 0)
			hook_callbacks[i]->on_command(player, command);
	}
}
