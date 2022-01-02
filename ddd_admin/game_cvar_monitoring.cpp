#include "game_cvar_monitoring.h"

#ifdef DDD_ENABLE_CVAR_MONITORING

typedef ConCommand ConCommandMonitored;

SH_DECL_HOOK2_void(IServerGameClients, ClientCommand, SH_NOATTRIB, false, edict_t*, const CCommand&);
SH_DECL_HOOK1_void(ConVar, SetValue, SH_NOATTRIB, 0, const char*);
SH_DECL_HOOK1_void(ConVar, SetValue, SH_NOATTRIB, 1, float);
SH_DECL_HOOK1_void(ConVar, SetValue, SH_NOATTRIB, 2, int);
SH_DECL_HOOK1_void(ConVar, SetValue, SH_NOATTRIB, 3, Color);
SH_DECL_HOOK1_void(ConCommandMonitored, Dispatch, SH_NOATTRIB, 0, const CCommand&);

const char* CVAR_MONITOR_BLACKLIST[] = {
	"say",
	"say_team",
	"banid",
	"joe_equip_thompson",
	"joe_equip_greasegun",
	"ilona_equip_tokarev",
	"ilona_equip_nagant",
	"nigel_equip_piat",
	"nigel_equip_flamethrower",
	"hardgrave_equip_garand",
	"hardgrave_equip_carbine",
	"disable_auto_kill",
	"enable_auto_kill",
	0
};

void ddd_game_cvar_monitoring::init() {
	//Hook
	SH_ADD_HOOK(IServerGameClients, ClientCommand, gameclients, SH_MEMBER(this, &ddd_game_cvar_monitoring::on_client_command), false);

	//Iterate and bind to CVars...yuck!
	ICvar::Iterator iter(cvarmanager);
	ConCommandBase* cmdBase;
	const char* cmdName;
	bool blacklisted;
	for (iter.SetFirst(); iter.IsValid(); iter.Next()) {
		//Get
		cmdBase = iter.Get();
		cmdName = cmdBase->GetName();

		//Check if this is on the blacklist (for spammy ones)
		blacklisted = false;
		for (int i = 0; CVAR_MONITOR_BLACKLIST[i] != 0; i++)
			blacklisted = blacklisted || strcmp(cmdName, CVAR_MONITOR_BLACKLIST[i]) == 0;
		if (blacklisted)
			continue;

		//Bind depending on the type
		if (cmdBase->IsCommand()) {
			ConCommand* cmd = (ConCommand*)cmdBase;
			SH_ADD_HOOK(ConCommandMonitored, Dispatch, cmd, SH_MEMBER(this, &ddd_game_cvar_monitoring::on_cvar_dispatch), false);
		}
		else {
			ConVar* cmd = (ConVar*)cmdBase;
			SH_ADD_HOOK(ConVar, SetValue, cmd, SH_MEMBER(this, &ddd_game_cvar_monitoring::on_cvar_set_char), true);
			SH_ADD_HOOK(ConVar, SetValue, cmd, SH_MEMBER(this, &ddd_game_cvar_monitoring::on_cvar_set_float), true);
			SH_ADD_HOOK(ConVar, SetValue, cmd, SH_MEMBER(this, &ddd_game_cvar_monitoring::on_cvar_set_int), true);
			SH_ADD_HOOK(ConVar, SetValue, cmd, SH_MEMBER(this, &ddd_game_cvar_monitoring::on_cvar_set_color), true);
		}
	}
}

void ddd_game_cvar_monitoring::on_client_command(edict_t* pEntity, const CCommand& args) {
	printf("TEST ON CLIENT COMMAND: %s\n", args.GetCommandString());
}

void ddd_game_cvar_monitoring::on_cvar_dispatch(const CCommand& command) {
	printf("TEST DISPATCH: %s\n", command.GetCommandString());
}

void ddd_game_cvar_monitoring::on_cvar_set_char(const char* value) {
	ConVar* var = META_IFACEPTR(ConVar);
	printf("TEST on_cvar_set_char: %s -> %s\n", var->GetName(), value);
}

void ddd_game_cvar_monitoring::on_cvar_set_float(float value) {
	ConVar* var = META_IFACEPTR(ConVar);
	printf("TEST on_cvar_set_float: %s -> %f\n", var->GetName(), value);
}

void ddd_game_cvar_monitoring::on_cvar_set_int(int value) {
	ConVar* var = META_IFACEPTR(ConVar);
	printf("TEST on_cvar_set_int: %s -> %i\n", var->GetName(), value);
}

void ddd_game_cvar_monitoring::on_cvar_set_color(Color value) {
	ConVar* var = META_IFACEPTR(ConVar);
	printf("TEST on_cvar_set_color: %s\n", var->GetName());
}

#endif