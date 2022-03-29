#include "include/ddd_net_message.h"

DddNetMsg::DddNetMsg()
{
	next = 0;
	property_count = 0;
	property_payload_total_length = 0;
}

DddNetMsg::~DddNetMsg()
{
	clear();
}