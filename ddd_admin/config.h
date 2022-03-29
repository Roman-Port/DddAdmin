#pragma once

#include <ISmmAPI.h>

struct ddd_admin_config_t {

	char server_ip[64];
	char server_port[16];
	char server_psk[64];

};

bool ddd_load_admin_config(ISmmAPI* plugin, ddd_admin_config_t* result);