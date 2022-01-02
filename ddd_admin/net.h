// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#pragma once

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "io.h"

#define DDD_NET_QUEUE_SIZE 128

class ddd_packet_outgoing : public ddd_msg_writer {

public:
	ddd_packet_outgoing(ddd_msg_endpoint endpoint);
	bool serialize(SOCKET& sock, uint8_t netFlags);

	ddd_msg_endpoint endpoint;

};

class ddd_net {

public:
	ddd_net();

	bool is_awaiting_boot_msg();
	ddd_packet_outgoing* msg_create(ddd_msg_endpoint endpoint);
	void msg_send(ddd_packet_outgoing* writer, bool isBootMsg = false);

private:
	HANDLE thread;
	CRITICAL_SECTION thread_lock;
	CONDITION_VARIABLE thread_cv;

	// BEGIN CS PROTECTED
	ddd_packet_outgoing* queue[DDD_NET_QUEUE_SIZE];
	int queue_use;
	bool awaiting_boot_msg;
	// END CS PROTECTED

	void request_boot_msg();
	void wait_dequeue_packet(ddd_packet_outgoing** result);
	void worker_active(SOCKET& sock);
	void worker_main();
	static DWORD WINAPI worker_static(LPVOID ctx);

};