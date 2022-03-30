#include "stopwatch.h"

ddd_stopwatch::ddd_stopwatch()
{
	//Query frequency; This is only needed once
	QueryPerformanceFrequency(&freq);

	//Reset to clear out value
	reset();
}

double ddd_stopwatch::elapsed_seconds()
{
	//Query now
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	//Calculate
	return (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
}

void ddd_stopwatch::reset()
{
	//Query now
	QueryPerformanceCounter(&start);
}
