// (C) RomanPort, 2022. Only licensed for private LAN Dino D-Day servers. Use is prohibited on public servers.

#include "net.h"
#include <stdexcept>

#define NET_PORT "22037"
#define PROTOCOL_VERSION 1

#define NETFLAG_QUEUE_OVERFLOW (1 << 0)

bool send_data(SOCKET& sock, const void* data, int size) {
    int result = send(sock, (const char*)data, size, 0);
    if (result == size)
        return true;

    //Failed; check result
    if (result == SOCKET_ERROR)
        printf("ddd_admin:WARN // Link send failed with error: %d\n", WSAGetLastError());
    else
        printf("ddd_admin:WARN // Link send failed to send all requested bytes. Got %d, expected %d!\n", result, size);

    return false;
}

ddd_net::ddd_net() : queue_use(0), thread(0), awaiting_boot_msg(false) {
    //Init Windows stuff
    if (InitializeCriticalSectionAndSpinCount(&thread_lock, 0x00000400) == 0) {
        printf("ddd_admin:ERROR // Failed to initialize mutex. This'll result in errors...\n");
    }
    InitializeConditionVariable(&thread_cv);

    //Create worker thread
    DWORD id;
    thread = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        worker_static,       // thread function name
        this,          // argument to thread function 
        0,                      // use default creation flags 
        &id);   // returns the thread identifier 

    //Log
    printf("ddd_admin:INFO // Created network thread %i; Listening on port %s...\n", id, NET_PORT);
}

bool ddd_net::is_awaiting_boot_msg() {
    //Lock
    EnterCriticalSection(&thread_lock);

    //Get result
    bool result = awaiting_boot_msg;

    //Unlock
    LeaveCriticalSection(&thread_lock);

    return result;
}

ddd_packet_outgoing* ddd_net::msg_create(ddd_msg_endpoint endpoint) {
    return new ddd_packet_outgoing(endpoint);
}

void ddd_net::msg_send(ddd_packet_outgoing* writer, bool isBootMsg) {
    //Lock
    EnterCriticalSection(&thread_lock);

    //Enqueue; Boot messages are treated specially
    if (isBootMsg && awaiting_boot_msg) {
        //This is the boot message we're after! It ALWAYS gets placed in the first slot.
        queue[0] = writer;
        awaiting_boot_msg = false;
    }
    else if (isBootMsg && !awaiting_boot_msg) {
        //We got a boot message...when we weren't waiting for one
        printf("ddd_admin:WARN // Attempted to add boot message when we weren't waiting for one. Ignoring...\n");
    }
    else if (queue_use < DDD_NET_QUEUE_SIZE) {
        //Warn if we're still waiting, but add anyways
        if (awaiting_boot_msg)
            printf("ddd_admin:WARN // Enqueueing normal packet while waiting on a boot packet. It'll be delayed until a boot packet arrives. Consider checking more often.\n");

        //Queue normally
        queue[queue_use++] = writer;
    }
    else {
        //No more room!
        printf("ddd_admin:WARN // Attempted to enqueue an outgoing message on full queue. Silently dropping message... (this may cause errors)\n");
    }

    //Unlock
    LeaveCriticalSection(&thread_lock);

    //Notify
    WakeConditionVariable(&thread_cv);
}

void ddd_net::wait_dequeue_packet(ddd_packet_outgoing** result) {
    //Lock
    EnterCriticalSection(&thread_lock);

    //Wait until there's something in the queue and we aren't waiting for the boot message
    while (queue_use == 0 || awaiting_boot_msg)
        SleepConditionVariableCS(&thread_cv, &thread_lock, INFINITE);

    //Dequeue
    (*result) = queue[0];
    memcpy(&queue[0], &queue[1], sizeof(ddd_packet_outgoing*) * (DDD_NET_QUEUE_SIZE - 1));
    queue_use--;

    //Unlock
    LeaveCriticalSection(&thread_lock);
}

void ddd_net::request_boot_msg() {
    //Lock
    EnterCriticalSection(&thread_lock);

    //Only proceed if flag isn't already set
    if (!awaiting_boot_msg) {
        //Discard any items in the queue already
        for (int i = 0; i < queue_use; i++) {
            delete(queue[i]);
            queue[i] = 0;
        }

        //Set flags, reserving a slot for the boot message
        awaiting_boot_msg = true;
        queue_use = 1;
    }

    //Unlock
    LeaveCriticalSection(&thread_lock);
}

void ddd_net::worker_active(SOCKET& sock) {
    //Request the boot message
    request_boot_msg();

    //Enter loop
    ddd_packet_outgoing* msg;
    bool success;
    do {
        //Await packet
        wait_dequeue_packet(&msg);

        //Send over network
        success = msg->serialize(sock, 0);

        //Free
        delete(msg);
    } while (success);
}

void ddd_net::worker_main() {
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;

    //Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("ddd_admin:ERROR // WSAStartup failed with error: %d\n", iResult);
        return;
    }

    //Configure hints
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    //Resolve the server address and port
    iResult = getaddrinfo(NULL, NET_PORT, &hints, &result);
    if (iResult != 0) {
        printf("ddd_admin:ERROR // getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return;
    }

    //Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("ddd_admin:ERROR // Socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return;
    }

    //Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("ddd_admin:ERROR // Bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    //Free address info
    freeaddrinfo(result);

    //Listen for incoming sockets...
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("ddd_admin:ERROR // Listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    //Enter loop of waiting for incoming sockets...
    while (true) {
        //Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("ddd_admin:ERROR // Accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return;
        }

        //Connection established! Run the "active" worker
        printf("ddd_admin:INFO // Link established!\n");
        worker_active(ClientSocket);
        printf("ddd_admin:WARN // Link lost.\n");

        //Shut down client socket
        closesocket(ClientSocket);
    }
}

DWORD WINAPI ddd_net::worker_static(LPVOID ctx) {
    ((ddd_net*)ctx)->worker_main();
    return 0;
}

ddd_packet_outgoing::ddd_packet_outgoing(ddd_msg_endpoint endpoint) {
    this->endpoint = endpoint;
}

bool ddd_packet_outgoing::serialize(SOCKET& sock, uint8_t netFlags) {
    //Sanity check
    if (buffer_len > 4294967295)
        throw std::runtime_error("Payload is too large to send.");

    //Create header
    uint64_t header = 0;
    header |= (uint64_t)PROTOCOL_VERSION << 0;
    header |= (uint64_t)netFlags << 8;
    header |= (uint64_t)endpoint << 16;
    header |= (uint64_t)buffer_pos << 32;

    //Send header and then send payload
    if (!send_data(sock, &header, 8))
        return false;
    if (!send_data(sock, buffer, buffer_pos))
        return false;

    return true;
}