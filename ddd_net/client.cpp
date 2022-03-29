#include "include/ddd_net_client.h"
#include "defines.h"
#include <malloc.h>
#include <stdexcept>

#define RECEIVE_DUTY_CYCLE 500

DddNetClient::DddNetClient(IDddNetClientCallbacks* callbacks, size_t queue_count) : queue(queue_count), queue_incoming(queue_count)
{
	this->callbacks = callbacks;
	sock = 0;
	thread = 0;
	connected = false;
}

DddNetClient::~DddNetClient()
{
	//TODO
}

void DddNetClient::init(const char* ip, const char* port, const char* psk)
{
	//Copy to buffers
	strcpy_s(remote_ip, sizeof(remote_ip), ip);
	strcpy_s(remote_port, sizeof(remote_port), port);
	strcpy_s(remote_psk, sizeof(remote_psk), psk);

	//Log
	printf("ddd_net:INFO // Initializing client... target=%s:%s; protocol=%i; opcode=%i\n", remote_ip, remote_port, DDD_NET_PROTOCOL_VERSION, DDD_NET_OPCODES_VERSION);

	//Create worker thread
	DWORD id;
	thread = CreateThread(
		NULL,               // default security attributes
		0,                  // use default stack size  
		worker_static,		// thread function name
		this,				// argument to thread function 
		0,                  // use default creation flags 
		&id);				// returns the thread identifier 
}

void DddNetClient::enqueue_outgoing(DddNetEndpoint opcode, DddNetMsg& message)
{
	//Encode
	size_t len;
	void* payload = encode_packet(opcode, message, &len);

	//Log
	printf("ddd_net:INFO // Sending queued outgoing message of size %i...\n", len);

	//Queue (it'll be freed later)
	if (!queue.enqueue(payload, len)) {
		//Queue is full! We'll just free this now...
		printf("ddd_net:WARN // Attempted to queue message into full queue!\n");
		free(payload);
	}
}

bool DddNetClient::dequeue_incoming(DddNetEndpoint* opcode, DddNetMsg& message)
{
	//Attempt to dequeue something
	void* buffer;
	size_t len;
	if (queue_incoming.dequeue(&buffer, &len, 0)) {
		//At this point, safety checks have already been performed. Extract data...
		(*opcode) = (DddNetEndpoint)(((net_packet_header*)buffer)->opcode);
		uint8_t* payload = ((uint8_t*)buffer) + sizeof(net_packet_header);
		bool result = message.deserialize(payload, len - sizeof(net_packet_header));

		//Free buffer
		free(buffer);

		return result;
	}
	else {
		return false;
	}
}

void DddNetClient::send_outgoing_sync(DddNetEndpoint opcode, DddNetMsg& message)
{
	//Encode
	size_t len;
	void* payload = encode_packet(opcode, message, &len);

	//Send
	if (connected && send(sock, (const char*)payload, len, 0) == SOCKET_ERROR) {
		//Failed to transmit!
		connected = false;
	}

	//Clean up
	free(payload);
}

void* DddNetClient::encode_packet(DddNetEndpoint opcode, DddNetMsg& message, size_t* outLength)
{
	//Query size
	size_t messageLen = message.get_serialized_length();
	size_t packetLen = sizeof(net_packet_header) + messageLen;

	//Allocate buffer
	uint8_t* buffer = (uint8_t*)malloc(packetLen);
	if (buffer == 0)
		throw std::runtime_error("Failed to allocate packet buffer.");

	//Create header and write
	net_packet_header header;
	header.packet_length = packetLen;
	header.protocol_version = DDD_NET_PROTOCOL_VERSION;
	header.opcode_version = DDD_NET_OPCODES_VERSION;
	header.opcode = opcode;
	header.reserved = 0;
	memcpy(buffer, &header, sizeof(net_packet_header));

	//Serialize content
	message.serialize(&buffer[sizeof(net_packet_header)], messageLen);

	//Set output
	(*outLength) = packetLen;
	return buffer;
}

bool DddNetClient::receive_full_buffer(char* buffer, size_t buffer_length)
{
	int received;
	while (buffer_length > 0) {
		received = recv(sock, buffer, buffer_length, 0);
		if (received > 0) {
			//Got data...
			buffer += received;
			buffer_length -= received;
		}
		else {
			//Connection closed
			connected = false;
			return false;
		}
	}
	return true;	
}

/// <summary>
/// Fetches a whole packet and waits until either it's recieved, we time out, or there's an error.
/// Returns true on success, setting buffer and buffer_length. Caller has to free buffer.
/// Returns false on timeout or failure, caller shouldn't touch buffer or buffer_length.
/// </summary>
bool DddNetClient::receive_packet_raw(void** buffer, size_t* buffer_length)
{
	//Set up file descriptor set
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	//Set up timeout
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = RECEIVE_DUTY_CYCLE * 1000;

	//Wait for data (or timeout) and recieve the header
	int result = select(sock, &fds, NULL, NULL, &timeout);
	if (result == 1) {
		//Data is ready! Receive the header
		net_packet_header header;
		if (receive_full_buffer((char*)&header, sizeof(header))) {
			//Calculate size
			(*buffer_length) = header.packet_length;
			size_t payloadLength = header.packet_length - sizeof(net_packet_header);

			//Validate
			if (payloadLength < 0 || payloadLength > MAX_PACKET_SIZE) {
				printf("ddd_net:WARN // Got message claiming to be %i bytes in size. Dropping connection...\n", header.packet_length);
				connected = false;
				return false;
			}
			if (header.protocol_version != DDD_NET_PROTOCOL_VERSION) {
				printf("ddd_net:WARN // Network protocol version mismatch; dropping connection... (expected %i, got %i)\n", DDD_NET_PROTOCOL_VERSION, header.protocol_version);
				connected = false;
				return false;
			}
			if (header.opcode_version != DDD_NET_OPCODES_VERSION) {
				printf("ddd_net:WARN // Network opcode version mismatch; dropping connection... (expected %i, got %i)\n", DDD_NET_OPCODES_VERSION, header.opcode_version);
				connected = false;
				return false;
			}

			//Allocate
			(*buffer) = (char*)malloc((*buffer_length));
			if ((*buffer) == 0)
				throw std::runtime_error("Failed to allocate buffer to store recieved packet.");

			//Copy header
			memcpy((*buffer), &header, sizeof(net_packet_header));

			//Receive payload
			if (receive_full_buffer((char*)(*buffer) + sizeof(net_packet_header), payloadLength)) {
				//OK!
				return true;
			}
			else {
				//Failed; cleanup...
				free((*buffer));
				return false;
			}
		}
	}
	else if (result < 0) {
		//Error
		connected = false;
	}
	return false;
}

void DddNetClient::worker()
{
	//Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		throw std::runtime_error("Failed to initialize WinSock: WSAStartup returned error.");

	//Configure
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//Resolve the server address and port
	addrinfo* result;
	if (getaddrinfo(remote_ip, remote_port, &hints, &result) != 0) {
		WSACleanup();
		throw std::runtime_error("Failed to initialize WinSock: getaddrinfo returned error.");
	}

	//Enter connection loop
	sock = INVALID_SOCKET;
	while (true) {
		//Create a SOCKET for connecting to server
		sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (sock == INVALID_SOCKET) {
			WSACleanup();
			throw std::runtime_error("Failed to initialize WinSock: could not create socket.");
		}

		//Connect to server
		if (connect(sock, result->ai_addr, (int)result->ai_addrlen) != SOCKET_ERROR) {
			//Connected! Update state
			int removedItems = queue.clear();
			printf("ddd_net:INFO // Connection established! (removed %i items from queue)\n", removedItems);
			connected = true;

			//Send event
			callbacks->on_net_connected();

			//Send login command
			printf("ddd_net:INFO // Sending login command...\n");
			{
				DddNetMsg login;
				login.put_string(DddNetOpcode::PSK, remote_psk);
				send_outgoing_sync(DddNetEndpoint::CONNECTION_LOGIN, login);
			}

			//Enter loop as long as we have a connection
			void* payload;
			size_t payload_len;
			int received;
			net_packet_header received_header;
			while (connected) {
				//Try to dequeue something from the queue
				if (connected && queue.dequeue(&payload, &payload_len, RECEIVE_DUTY_CYCLE)) {
					//Send
					if (send(sock, (const char*)payload, payload_len, 0) == SOCKET_ERROR) {
						//Failed to transmit!
						connected = false;
					}

					//Clean up
					free(payload);
				}

				//Try to receive any data from the socket
				if (connected && receive_packet_raw(&payload, &payload_len)) {
					//Push into queue
					if (!queue_incoming.enqueue(payload, payload_len)) {
						printf("ddd_net:WARN // Attempted to push incoming packet to queue, but it was full!\n");
						free(payload);
					}
				}
			}
		}

		//Log errors
		if (connected)
			printf("ddd_net:WARN // Connection was dropped. Retrying shortly...\n");
		else
			printf("ddd_net:WARN // Failed to connect. Retrying shortly...\n");

		//Properly close the socket
		connected = false;
		closesocket(sock);
		sock = INVALID_SOCKET;

		//Send event
		callbacks->on_net_disconnected();

		//Wait to start again
		Sleep(NETWORK_RETRY_DELAY);
	}
}

DWORD WINAPI DddNetClient::worker_static(LPVOID ctx)
{
	try {
		((DddNetClient*)ctx)->worker();
	}
	catch (std::exception ex) {
		printf("ddd_net:FATAL // Failed to run network server: %s\n", ex.what());
	}
	return 0;
}
