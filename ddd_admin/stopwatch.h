#pragma once
#include <windows.h>

class ddd_stopwatch {

public:
	ddd_stopwatch();

	double elapsed_seconds();
	void reset();

private:
	LARGE_INTEGER freq;
	LARGE_INTEGER start;

};