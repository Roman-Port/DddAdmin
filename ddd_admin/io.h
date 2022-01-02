// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#pragma once

#include <stdint.h>

enum ddd_msg_endpoint {

	BOOT,
	SERVER_START,
	SERVER_CVAR,
	SERVER_CVAR_REGISTER,
	SERVER_CVAR_REGISTER_LATE,
	
	MAP_START,
	MAP_END,

	PLAYER_CONNECT,
	PLAYER_DISCONNECT,
	PLAYER_CHAT,
	PLAYER_UPDATE,

};

enum ddd_msg_key {

	DDDTYPE_SHORT,
	DDDTYPE_INT,
	DDDTYPE_FLOAT,
	DDDTYPE_STRING,
	DDDTYPE_MESSAGE,
	DDDTYPE_MESSAGE_ARRAY,

	USER_ID,
	FLAGS,
	STEAM_ID,
	IP_ADDRESS,
	NAME,
	REASON,
	MESSAGE,
	TEAM_ONLY,
	MAX_PLAYERS,
	OS,
	DEDICATED,
	OFFICIAL,
	PASSWORD_PROTECTED,
	VALUE,
	HELP,
	MAX,
	MIN,
	TEAM,
	CLASS,
	FRAGS,
	DEATHS,
	GOAT_KILLS,
	HEAL_POINTS,
	MAP_NAME,
	MAP_MODE,
	PLAYERS,
	CVARS,
	PLUGIN_VERSION,
	PLUGIN_COMPILE_DATE

};

class ddd_msg_writer_base {

public:
	ddd_msg_writer_base(size_t length = 512);
	~ddd_msg_writer_base();

	uint8_t* buffer;
	size_t buffer_len;
	size_t buffer_pos;

protected:
	void write(const void* src, size_t len);

};

class ddd_msg_writer_array;
class ddd_msg_writer : public ddd_msg_writer_base {

public:
	ddd_msg_writer(size_t length = 512) : ddd_msg_writer_base(length) {}

	void put_short(ddd_msg_key key, int16_t value);
	void put_int(ddd_msg_key key, int32_t value);
	void put_float(ddd_msg_key key, float value);
	void put_string(ddd_msg_key key, const char* value, int maxLength = 65536);
	void put_msg(ddd_msg_key key, ddd_msg_writer& writer);
	void put_array(ddd_msg_key key, ddd_msg_writer_array& writer);

private:
	void write_property(ddd_msg_key key, ddd_msg_key type, size_t length, const void* src);

};

class ddd_msg_writer_array : public ddd_msg_writer_base {

public:
	ddd_msg_writer_array(size_t length = 512);
	void put(ddd_msg_writer& writer);

};