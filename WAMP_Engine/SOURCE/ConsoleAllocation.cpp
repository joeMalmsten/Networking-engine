#include <windows.h>
#include <stdio.h>
#include "ConsoleAllocation.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

bool CORE::ConsoleAllocation(char* pTitle)
{
	if(false == AllocConsole())
		return false;

	FILE* file;

	freopen_s(&file, "CONOUT$", "wt", stdout);
	freopen_s(&file, "CONOUT$", "wt", stderr);
	freopen_s(&file, "CONIN$", "rt", stdin);
	
	SetConsoleTitle(pTitle);

	return true;
}