#pragma once

#include <list>
#include <string>

// ASSERT CODE WRITTEN BY Chris Kuspis
namespace WAMPNE
{
	bool Debug_Print(const char * expression, const char * msg, const char * file, unsigned int linenum);
}






#ifndef _DEBUG
#define WAMPASSERT(...) 
#else
#define WAMPASSERT(expr, ...)\
	do{ if( (expr) && WAMPNE::Debug_Print(#expr, __VA_ARGS__, __FILE__, __LINE__))\
	__asm { int 3 }; } while(0)
#endif

