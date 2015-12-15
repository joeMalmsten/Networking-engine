#include <string>

#pragma once

namespace UTILS
{
	bool InitializeUtils(void); // MUST CALL WHEN STARTING UP
	void CleanUpUtils(void);    // MUST CALL WHEN LEAVING PROGRAM
	DWORD WINAPI GetConsoleInputAsString(LPVOID lpParam); // THREAD FUNCTION FOR RECEIVING INPUT
	extern HANDLE InputMutex;  // GLOBAL HANDLE FOR CHECKING INPUT
}