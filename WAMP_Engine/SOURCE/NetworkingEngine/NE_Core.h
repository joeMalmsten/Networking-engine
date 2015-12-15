#pragma once

#include <windows.h>
#include <time.h>
#include <list>
#include <string>
#include <assert.h>

class Server;
class Client;

namespace WAMPNE
{
	class NESingletonWatcher;

	class NetworkEngine
	{
		public:

			~NetworkEngine(void);
			
			static NetworkEngine* getEnginePtr(void);  // creates the first instance and returns
												// a valid pointer for later calls
			
			static std::list<std::string> getDebugOutStrings(void);
			
			// if anyone knows how to create a message system, by all means, go for it!
			void addMessage(std::string message);

			// a return value of FALSE indicates an error
			//bool initEngine(void);              // init and destroy the engine once		
			//void killEngine(void);			    // -- might not need this yet (might be helpful for 
			//									// destroying everything, or getting ready to swap from server to client

			void update(float dt);
		private:
		
			NetworkEngine(void);

			friend class NESingletonWatcher;
			friend class PacketPipe;
			friend class Server;
			friend class Client;

			bool   engineInitialized;

			// methods to add to the output

			static void writeDebugOut(std::string str);
			static void writeDebugOut(const char* str);

			static NetworkEngine* singletonPointer;
			static std::list<std::string> debugOutStrings;
			static HANDLE debugMutex; 

			Server *pServer; // XOR condition. One of these should
			Client *pClient; // always be NULL -- I'm going to check to make sure it doesn't happen!

	};
}