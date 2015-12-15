#pragma once
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
class Threader{
public:
	static CRITICAL_SECTION socketListGuard;
	static void ListGuardInit();
	static void ListGuardDestroy();
	DWORD WINAPI Thread_It(void * args);
	DWORD threadHandle;
};
