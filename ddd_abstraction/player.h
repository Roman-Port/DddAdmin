#pragma once

#include "include/ddd_abstraction.h"
#include "interfaces.h"
#include <inetchannel.h>
#include <iclient.h>

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
	virtual const char* get_ip_address() override;
	virtual void kick(const char* message) override;

private:
	edict_t* entity;

	void ensure_valid();
	INetChannel* get_net_channel();
	IClient* get_client();

};