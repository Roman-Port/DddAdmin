#pragma once

#include "include/ddd_abstraction.h"
#include "interfaces.h"

class DddAbsPlayer : public IDddAbsPlayer {

public:
	DddAbsPlayer();

	void connect(edict_t* entity);
	void disconnect();

	/* API */

	virtual bool is_valid() override;
	virtual int get_index() override;
	virtual int get_uniqueid() override;
	virtual IPlayerInfo* get_info() override;
	virtual CBaseEntity* get_entity() override;
	virtual void kick(const char* message) override;

private:
	edict_t* entity;

	void ensure_valid();

};