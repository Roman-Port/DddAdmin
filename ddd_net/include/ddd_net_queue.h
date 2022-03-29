#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

struct ddd_net_queue_item {

	void* buffer;
	size_t buffer_size;

};

class DddNetQueue {

public:
	DddNetQueue(size_t size);
	~DddNetQueue();

	bool enqueue(void* buffer, size_t buffer_size);
	bool dequeue(void** buffer, size_t* buffer_size, int timeoutMs);
	int clear();

private:
	ddd_net_queue_item* items;
	int size;
	int use;
	int read;
	int write;

	CRITICAL_SECTION thread_lock;
	CONDITION_VARIABLE thread_cv;

};