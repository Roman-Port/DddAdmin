#include "callbacks.h"

void ddd_hook_chat::on_command(IDddAbsPlayer* player, const CCommand& command)
{
	if (player != 0) {
		//Get fields...
		const char* cmd = command.Arg(0);
		const char* message = command.Arg(1);
		if (cmd == 0 || message == 0)
			return;

		//Prepare flags
		uint16_t flags = 0;
		flags |= strcmp(cmd, "say_team") == 0;

		//Send
		DddNetMsg packet;
		packet.put_int(DddNetOpcode::USER_ID, player->get_index());
		packet.put_short(DddNetOpcode::FLAGS, flags);
		packet.put_string(DddNetOpcode::MESSAGE, message);
		client->enqueue_outgoing(DddNetEndpoint::PLAYER_CHAT, packet);
	}
}

void ddd_hook_watch::on_value_changed(IDddAbsPlayer* player, void* value, size_t value_size)
{
	//Create header
	DddNetMsg packet;
	packet.put_int(DddNetOpcode::USER_ID, player == 0 ? -1 : player->get_index());
	packet.put_short(DddNetOpcode::KEY, key);

	//Determine type from size
	switch (value_size) {
	case 2: packet.put_short(DddNetOpcode::VALUE, *((int16_t*)value)); break;
	case 4: packet.put_int(DddNetOpcode::VALUE, *((int32_t*)value)); break;
	default: printf("ddd_admin:WARN // One of the player variable watches is misconfigured. %i is not a known size.\n", value_size); break;
	}

	//Send
	client->enqueue_outgoing(DddNetEndpoint::PLAYER_UPDATE, packet);
}