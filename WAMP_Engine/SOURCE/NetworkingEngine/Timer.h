#pragma once

#include <time.h>

namespace WAMPNE
{
	class Timer
	{
		public:
			// method one
			void block(int ms); // blocks code for desired time in ms

			// method two
			void startTimer(int ms); // start a timer (also resets)
			bool timerOver(void);    // lets you know if the timer has reach the time above
			
		private:
			clock_t endTime;
	};
}