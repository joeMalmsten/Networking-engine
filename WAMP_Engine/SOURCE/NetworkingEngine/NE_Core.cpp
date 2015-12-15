#include <list>
#include <string>

#include "Server.h"
#include "Client.h"
#include "NE_Core.h"
#include "NE_Helper.h"
#include "NE_Utilities.h"




namespace WAMPNE
{

	// cleans up the singleton 
	class NESingletonWatcher
	{
	public:
		NESingletonWatcher(void);
		~NESingletonWatcher(void);
	} SingletonWatcher; // the global instance -- created and destroyed automatically 
}

// NetworkEngine singleton pointer
WAMPNE::NetworkEngine* WAMPNE::NetworkEngine::singletonPointer = 0;
std::list<std::string> WAMPNE::NetworkEngine::debugOutStrings; // uses this to get the debug out strings;
HANDLE WAMPNE::NetworkEngine::debugMutex;

// ENSURES THE SINGLETON IS CLEANED UP /////////////////
WAMPNE::NESingletonWatcher::NESingletonWatcher(void) {}
WAMPNE::NESingletonWatcher::~NESingletonWatcher(void) 
{
	if(NetworkEngine::singletonPointer) 
		delete NetworkEngine::singletonPointer; 
	NetworkEngine::singletonPointer = NULL;
}

////////////////////////////////////////////////////////




////// ACTUAL NETWORK ENGINE CODE ///////
WAMPNE::NetworkEngine::NetworkEngine(void) : engineInitialized(false), pServer(NULL), pClient(NULL)
{
	debugMutex = CreateMutex(NULL, FALSE, NULL);

	WAMPASSERT(NULL == debugMutex, "Couldn't create a critical mutex");

	if(NULL == debugMutex)
		writeDebugOut("Debug output mutex failed to create");

	// initialize here
	engineInitialized = true;
}

WAMPNE::NetworkEngine::~NetworkEngine(void) 
{
	if(pServer)
		delete pServer;
	if(pClient)
		delete pClient;
}
WAMPNE::NetworkEngine* WAMPNE::NetworkEngine::getEnginePtr(void)
{
	if(0 == singletonPointer)
	{
		singletonPointer = new NetworkEngine();
		WAMPASSERT(NULL == singletonPointer, "Couldn't allocate the engine");
		writeDebugOut(std::string("WAMP Network Engine Created!!!"));


	}

	return singletonPointer;
}

std::list<std::string> WAMPNE::NetworkEngine::getDebugOutStrings(void)
{
	std::list<std::string> out;

	WaitForSingleObject(debugMutex, INFINITE);
	out = debugOutStrings;
	debugOutStrings.clear();
	ReleaseMutex(debugMutex);
	return out;
}

void WAMPNE::NetworkEngine::addMessage(std::string message)
{
	// here we handle the creation and destruction of the server or client
	// any other message that is directed to server/client is forwarded to that sub system
	if(message.length() >= 3)
	{
		if( (message[0] == 'n' || message[0] == 'N') && (message[1] == 'e' || message[1] == 'E')  && message[2] == ' ')
		{
			std::string realMessage(&message.c_str()[3]); // strips off the network command 'ne '

			if(realMessage == "whats my name bitch") // here's an example
			{
				writeDebugOut("you're a bitch!");
			}
			else if(realMessage == "server create")
			{
		
				//WAMPASSERT(pClient != NULL || pServer != NULL, "Attempting to create server, when server or client is already running");
				if(pClient)
				{
					writeDebugOut("Client is already created, can not start server!");
				}
				else if(pServer)
				{
					writeDebugOut("Server is already running!");
				}
				else
				{
					pServer = new Server();
					pServer->Connect();
					writeDebugOut("Server created and is running!");
				}
			}
			else if(realMessage == "client create")
			{
				Client client;
				client.Initialize_Winsock();
		

				//WAMPASSERT(pClient != NULL || pServer != NULL, "Attempting to create client, when server or client is already running");
				if(pServer)
				{
					writeDebugOut("Server is already created, can not start client!");
				}
				else if(pClient)
				{
					writeDebugOut("Client is already running!");
				}
				else
				{
					pClient = new Client();
					pClient->Initialize_Winsock();
					writeDebugOut("Client created!");
				}
			}
			else if(realMessage == "client connect")
			{
				if(pClient)
					pClient->Connect_to_Server();
				else
					writeDebugOut("Client hasn't been created!");

			}
			else if(realMessage == "server disconnect")
			{
				if(pServer)
				{
					pServer->Shutdown(); // shut down needs to send disconnection
										 // message in this function because the 
										 // server is going to be deleted!
					delete pServer;
					pServer = NULL;
				}
				else
				{
					writeDebugOut("Server was never created!");
				}
			}
			else if(realMessage == "client disconnect")
			{
				if(pClient)
				{
					pClient->Shutdown(); // shut down needs to send disconnection
										 // message in this function because the 
										 // server is going to be deleted!

					delete pClient;
					pClient = NULL;
				}
				else
				{
					writeDebugOut("Client was never created!");
				}
			}
			else if(realMessage == "help")
			{
				writeDebugOut("Things I know...");
				writeDebugOut("'ne help'");
				writeDebugOut("'ne client help'");
				writeDebugOut("'ne client create'");
				writeDebugOut("'ne client connect'");
				writeDebugOut("'ne client disconnect'");
				writeDebugOut("'ne server help'");
				writeDebugOut("'ne server create'");
				writeDebugOut("'ne server disconnect'");
			


			}
			// ALL OTHER MESSAGES WILL BE FORWARDED TO THE SERVER OR CLIENT CLASS ITSELF
			// FROM THIS POINT ON
			else if(realMessage.length() > 7) // forward messages to the current networked thing
			{
				std::string forwardMessage;
				for(int i = 0; i < 6; ++i) forwardMessage.push_back(realMessage[i]);
			
				if(pServer && forwardMessage == "server")
				{
					forwardMessage = &realMessage.c_str()[7];
					pServer->AddMessage(forwardMessage);

				}
				else if(pClient && forwardMessage == "client")
				{
					forwardMessage = &realMessage.c_str()[7];
					pClient->AddMessage(forwardMessage);
				}
				else
					writeDebugOut("NE Echo:" + realMessage); // echo
			}
			// EXCEPT FOR THE ECHO
			else
			{
				writeDebugOut("NE Echo:" + realMessage); // echo
			}
		}
	}
}

void WAMPNE::NetworkEngine::update(float) // not using the parameter right now
{
	if(pServer)
	{
		pServer->Update();
	}
	else if(pClient)
	{
		pClient->Update();
	}

	// do anythign else for the enginer here
}

//bool WAMPNE::NetworkEngine::initEngine( void )
//{
//	return true;
//}
//void WAMPNE::NetworkEngine::killEngine( void )
//{
//
//}


void WAMPNE::NetworkEngine::writeDebugOut(const char *str)
{
	writeDebugOut(std::string(str));
}

void WAMPNE::NetworkEngine::writeDebugOut(std::string str)
{
	if(!singletonPointer)
		return;

	WaitForSingleObject(debugMutex, INFINITE);
	debugOutStrings.push_back(str);
	ReleaseMutex(debugMutex);

	return;
}