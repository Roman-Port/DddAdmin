#include "player.h"
#include <stdexcept>

DddAbsPlayer::DddAbsPlayer()
{
	entity = 0;
}

void DddAbsPlayer::connect(edict_t* entity)
{
	//Set
	this->entity = entity;
}

void DddAbsPlayer::disconnect()
{
	//Clear
	this->entity = 0;
}

bool DddAbsPlayer::is_valid()
{
	return entity != 0;
}

int DddAbsPlayer::get_index()
{
	ensure_valid();
	return index_of_edict(entity);
}

int DddAbsPlayer::get_uniqueid()
{
	ensure_valid();
	return get_info()->GetUserID();
}

IPlayerInfo* DddAbsPlayer::get_info()
{
	ensure_valid();
	return playerinfomanager->GetPlayerInfo(entity);
}

CBaseEntity* DddAbsPlayer::get_entity()
{
	return entity->m_pNetworkable->GetBaseEntity();
}

const char* DddAbsPlayer::get_ip_address()
{
	return get_net_channel()->GetRemoteAddress().ToString();
}

void DddAbsPlayer::kick(const char* message)
{
	//https://github.com/alliedmodders/sourcemod/blob/ccc818d06e950cd99bdc51ce84500f686b1fa23e/core/PlayerManager.cpp#L2181
	get_client()->Disconnect("%s", message);
}

void DddAbsPlayer::ensure_valid()
{
	if (!is_valid())
		throw std::runtime_error("Player is not currently connected.");
}

INetChannel* DddAbsPlayer::get_net_channel()
{
	INetChannel* pNetChan = static_cast<INetChannel*>(engine->GetPlayerNetInfo(get_index()));
	if (!pNetChan)
		throw std::runtime_error("Could not get network channel!");
	return pNetChan;
}

IClient* DddAbsPlayer::get_client()
{
	IClient* pClient = static_cast<IClient*>(get_net_channel()->GetMsgHandler());
	if (!pClient)
		throw std::runtime_error("Could not get client!");
	return pClient;
}
