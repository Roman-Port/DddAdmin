#include "serialization.h"

static char general_purpose_string_buffer[256];

static void copy_to_string_buffer(const char* src, int len) {
	//Cap the length to fit and still have space for NULL terminator
	if (len < 0)
		len = 0;
	if (len > 255)
		len = 255;

	//Transfer
	memcpy(general_purpose_string_buffer, src, sizeof(char) * len);

	//Set NULL terminator
	general_purpose_string_buffer[len] = 0;
}

//CONVAR FLAGS:
// 00 | Is command
// 01 | <RESERVED>
// 02 | VCVAR_CHEAT
// 02 | VCVAR_CHEAT
// 03 | FCVAR_REPLICATED
// 04 | FCVAR_NOTIFY
// 05 | FCVAR_HIDDEN
// 06 | FCVAR_SPONLY
// 07 | FCVAR_NEVER_AS_STRING

void serialize_convar(DddNetMsg* packet, ConCommandBase* convar) {
	// Create flags for all types
	uint16_t flags = 0;
	if (convar->IsCommand())
		flags |= (1 << 0);
	if (convar->IsFlagSet(FCVAR_CHEAT))
		flags |= (1 << 2);
	if (convar->IsFlagSet(FCVAR_REPLICATED))
		flags |= (1 << 3);
	if (convar->IsFlagSet(FCVAR_NOTIFY))
		flags |= (1 << 4);
	if (convar->IsFlagSet(FCVAR_HIDDEN))
		flags |= (1 << 5);
	if (convar->IsFlagSet(FCVAR_SPONLY))
		flags |= (1 << 6);
	if (convar->IsFlagSet(FCVAR_NEVER_AS_STRING))
		flags |= (1 << 7);

	//Do more for non-commands
	if (!convar->IsCommand())
	{
		//Variable
		ConVar* var = (ConVar*)convar;

		//Attempt to get min/max
		if (var->HasMax())
			packet->put_float(DddNetOpcode::MAX, var->GetMaxValue());
		if (var->HasMin())
			packet->put_float(DddNetOpcode::MIN, var->GetMinValue());

		//Attach string value
		copy_to_string_buffer(var->GetRawValue().m_pszString, var->GetRawValue().m_StringLength);
		packet->put_string(DddNetOpcode::VALUE_STRING, general_purpose_string_buffer);

		//Attach other values
		packet->put_int(DddNetOpcode::VALUE_INT, var->GetRawValue().m_nValue);
		packet->put_float(DddNetOpcode::VALUE_FLOAT, var->GetRawValue().m_fValue);
	}

	//Finally, attach global items
	packet->put_string(DddNetOpcode::NAME, convar->GetName());
	packet->put_string(DddNetOpcode::HELP_TEXT, convar->GetHelpText());
	packet->put_short(DddNetOpcode::FLAGS, (int16_t)flags);
}