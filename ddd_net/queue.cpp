#include "include/ddd_net_queue.h"
#include <malloc.h>
#include <stdexcept>

DddNetQueue::DddNetQueue(size_t size)
{
	//(Re)set
	this->size = size;
	use = 0;
	read = 0;
	write = 0;

	//Allocate
	items = (ddd_net_queue_item*)malloc(sizeof(ddd_net_queue_item) * size);
	if (!items)
		throw std::runtime_error("Failed to allocate internal buffer.");

	//Init critical section
	if (!InitializeCriticalSectionAndSpinCount(&thread_lock, 0x00000400))
		throw std::runtime_error("Failed to initialize critical section!");

	//Init condition variable
	InitializeConditionVariable(&thread_cv);
}

DddNetQueue::~DddNetQueue()
{
	//TODO
}

bool DddNetQueue::enqueue(void* buffer, size_t buffer_size)
{
	//Lock
	EnterCriticalSection(&thread_lock);

	//Make sure there's room to write
	bool success = use < size;
	if (success) {
		//Get output and write to it
		ddd_net_queue_item* result = &items[write];
		result->buffer = buffer;
		result->buffer_size = buffer_size;

		//Update state
		use++;
		write = (write + 1) % size;
	}

	//Unlock
	LeaveCriticalSection(&thread_lock);

	//Notify
	WakeConditionVariable(&thread_cv);

	return success;
}

bool DddNetQueue::dequeue(void** buffer, size_t* buffer_size, int timeoutMs)
{
	//Lock
	EnterCriticalSection(&thread_lock);

	//Wait until there's something in the queue or we timeout
	if (use == 0 && timeoutMs > 0)
		SleepConditionVariableCS(&thread_cv, &thread_lock, timeoutMs);

	//Dequeue if there was something there
	bool success = use > 0;
	if (success) {
		//Dequeue
		ddd_net_queue_item result = items[read];
		(*buffer) = result.buffer;
		(*buffer_size) = result.buffer_size;

		//Update state
		use--;
		read = (read + 1) % size;
	}

	//Unlock
	LeaveCriticalSection(&thread_lock);

	return success;
}

int DddNetQueue::clear()
{
	//Lock
	EnterCriticalSection(&thread_lock);

	//Find all items
	int count = 0;
	while (use > 0) {
		//Dequeue and free buffer
		ddd_net_queue_item result = items[read];
		free(result.buffer);

		//Update state
		use--;
		read = (read + 1) % size;
		count++;
	}

	//Unlock
	LeaveCriticalSection(&thread_lock);

	return count;
}
