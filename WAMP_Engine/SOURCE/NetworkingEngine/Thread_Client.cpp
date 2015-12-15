#include "Thread_Client.h"
CRITICAL_SECTION Threader::socketListGuard;
void Threader::ListGuardInit(){
    InitializeCriticalSection(&Threader::socketListGuard);
}

void Threader::ListGuardDestroy(){
    DeleteCriticalSection(&Threader::socketListGuard);
}
DWORD WINAPI Thread_It(void * args){
return 0;
}