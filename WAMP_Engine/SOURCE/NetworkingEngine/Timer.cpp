#include "Timer.h"

void WAMPNE::Timer::block(int ms)
{
	endTime = clock()  + (time_t)((ms / 1000.0f) * CLOCKS_PER_SEC);

	while(clock() < endTime) {}

	return;
}

void WAMPNE::Timer::startTimer(int ms)
{
	endTime = clock() + (time_t)((ms / 1000.0f) * CLOCKS_PER_SEC);
}

bool WAMPNE::Timer::timerOver(void)
{
	return clock() >= endTime;
}
