#pragma once

#include "ddd_defines.h"
#include "net.h"
#include "game_players.h"

#define PROP_VALUE_GET_NUMBER(value, type) (*((type*)value))

class watch_impl : public ddd_game_player_watch {

public:
	watch_impl(ddd_game_property_search_result propInfo, ddd_msg_key key, size_t propSize, SendPropType expectedType) : ddd_game_player_watch(propInfo, propSize), key(key) {
		is_ready = propInfo.prop->GetType() == expectedType;
	}

protected:
	virtual void value_changed(ddd_game_player* player, SendProp* prop, void* value) override {
		ddd_packet_outgoing* packet = dddNetwork->msg_create(ddd_msg_endpoint::PLAYER_UPDATE);
		packet->put_int(ddd_msg_key::USER_ID, player->get_client_id());
		apply_prop(packet, key, value);
		dddNetwork->msg_send(packet);
	}

	virtual void apply_prop(ddd_packet_outgoing* packet, ddd_msg_key key, void* value) = 0;

private:
	ddd_msg_key key;
	bool is_ready;

};

class watch_impl_int : public watch_impl {

public:
	watch_impl_int(ddd_game_property_search_result propInfo, ddd_msg_key key, const char* debugName) : watch_impl(propInfo, key, sizeof(int), SendPropType::DPT_Int) {}

protected:
	virtual void apply_prop(ddd_packet_outgoing* packet, ddd_msg_key key, void* value) override {
		packet->put_int(key, PROP_VALUE_GET_NUMBER(value, int));
	}

};

class watch_impl_string : public watch_impl {

public:
	watch_impl_string(ddd_game_property_search_result propInfo, ddd_msg_key key, const char* debugName) : watch_impl(propInfo, key, sizeof(char*), SendPropType::DPT_String) {}

protected:
	virtual void apply_prop(ddd_packet_outgoing* packet, ddd_msg_key key, void* value) override {
		char* str = (char*)value;
		if (str == 0)
			packet->put_string(key, "<NULL>");
		else
			packet->put_string(key, str);
	}

};