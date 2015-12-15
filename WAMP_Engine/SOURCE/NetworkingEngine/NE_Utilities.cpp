#include <windows.h>
#include <fstream>
#include <iostream>


#include "NE_Utilities.h"



// ASSERT CODE WRITTEN BY Chris Kuspis
bool WAMPNE::Debug_Print(const char * expression, const char * msg, const char * file, unsigned int linenum)
	{

#ifdef _DEBUG
	
		const int Buffer = 1024;
		char ErrorMessage[Buffer];

		int offset = sprintf_s(ErrorMessage, "%s(%d) : ", file , linenum );	
		
		va_list arg;
		va_start(arg, msg);
		_vsnprintf(ErrorMessage + offset, sizeof(ErrorMessage) - offset , msg, arg);
		va_end(arg);

		OutputDebugStringA(ErrorMessage);
		OutputDebugStringA("\n");

		MessageBoxA(NULL, ErrorMessage, "ASSERT", 0);

		HANDLE conWin = GetStdHandle(STD_OUTPUT_HANDLE);

		/*
		if(conWin)
		{
			WriteDebugOutput(ErrorMessage);
		}
		*/

#endif

		return true;
	}