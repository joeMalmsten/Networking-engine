#include <stdio.h>
#include <windows.h>
#include <iostream>
#include "Utilities.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

namespace UTILS
{
	HANDLE InputMutex;
	bool InitializeMutex(void);
}


// to initialize multiple objects just do return InitializeMutex() && obj1() && obj2() && ... && objn()
// NOTE: RETURNS FALSE WHEN THERE'S AN ERROR!
bool UTILS::InitializeUtils(void)
{
	return InitializeMutex(); 
}

// do all clean up here
void UTILS::CleanUpUtils(void)
{
	CloseHandle(InputMutex);
}


// initializes mutex for input
bool UTILS::InitializeMutex(void)
{
	InputMutex = CreateMutex(NULL, FALSE, NULL);
	
	if(NULL == InputMutex)
		return false;

	return true;
}

// input  thread function
DWORD WINAPI UTILS::GetConsoleInputAsString(LPVOID lpParam)
{
	// the input is a pointer to a string
	std::string *inString = reinterpret_cast<std::string*>(lpParam);
	char inChar;  // read in char's here
	DWORD waitResult; // thread wait result

	std::string tempString;

	for(;;) // infinitely loop
	{
		tempString.clear();
		inChar = getchar(); // get a char
		
		


		// this loop will take in the previously read in char
		// and continue reading them / adding them to a string
		// as long as the char isn't the newline
		while(inChar != '\n')    // if not a newline add to buffer
		{
			tempString.push_back(inChar);
			inChar = getchar();
		}
			

		waitResult = WaitForSingleObject(InputMutex, INFINITE); // ask for the mutex

		if(WAIT_OBJECT_0 == waitResult)
		{
			// release the mutex
			*inString = tempString;
			ReleaseMutex(InputMutex);
		}
		else if(WAIT_FAILED == waitResult)
		{
			// an error happened -- don't wait for the mutex... lol.
			std::cout << "ERROR WITH 'InputMutex'\n" << std::endl;
			return 0;
		}
	}
	return 0;
}