#pragma once

#include <ddd_abstraction.h>
#include <ddd_net_client.h>
#include "config.h"

void serialize_convar(DddNetMsg* packet, ConCommandBase* var);