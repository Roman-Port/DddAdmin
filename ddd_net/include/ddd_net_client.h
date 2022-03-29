#pragma once

#define WIN32_LEAN_AND_MEAN

#include "ddd_net_message.h"
#include "ddd_net_queue.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#define DDD_NET_PROTOCOL_VERSION 2

class IDddNetClientCallbacks {

public:
	virtual void on_net_connected() = 0;
	virtual void on_net_disconnected() = 0;

};

class DddNetClient {

public:
	DddNetClient(IDddNetClientCallbacks* callbacks, size_t queue_count);
	~DddNetClient();

	/// <summary>
	/// Starts the worker thread. Should only be called once.
	/// </summary>
	void init(const char* ip, const char* port, const char* psk);

	/// <summary>
	/// Queues the message for delivery. Callable on any thread and will not hang for very long.
	/// </summary>
	/// <param name="opcode"></param>
	/// <param name="message"></param>
	void enqueue_outgoing(DddNetEndpoint opcode, DddNetMsg& message);

	bool dequeue_incoming(DddNetEndpoint* opcode, DddNetMsg& message);

protected:
	/// <summary>
	/// Sends a message right now and hangs until it's sent. CAN ONLY BE RUN ON WORKER THREAD!
	/// </summary>
	/// <param name="opcode"></param>
	/// <param name="message"></param>
	void send_outgoing_sync(DddNetEndpoint opcode, DddNetMsg& message);

private:
	void* encode_packet(DddNetEndpoint opcode, DddNetMsg& message, size_t* outLength);
	bool receive_full_buffer(char* buffer, size_t buffer_length);
	bool receive_packet_raw(void** buffer, size_t* buffer_length);
	void worker();
	static DWORD WINAPI worker_static(LPVOID ctx);

	IDddNetClientCallbacks* callbacks;

	char remote_ip[64];
	char remote_port[8];
	char remote_psk[64];

	DddNetQueue queue;
	DddNetQueue queue_incoming;
	HANDLE thread;

	//Following should only be touched by worker thread
	SOCKET sock;
	bool connected;
};