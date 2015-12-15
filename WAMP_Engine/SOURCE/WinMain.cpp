#include "NetworkingEngine\Server.h"
#include "NetworkingEngine\Client.h"
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>


#include "NetworkingEngine\NE_Utilities.h"
#include "NetworkingEngine\NE_Core.h"
#include "WindowJunk.h"
#include "ConsoleAllocation.h"
#include "Utilities.h"


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define CREATE_GAME_WINDOW

bool QUIT = false;


// thread information for INPUT CONSOLE
HANDLE InputThreadHandle;
DWORD  InputThreadID;

#ifdef CREATE_GAME_WINDOW
MSG msg;
WindowJunk window;
#endif
	

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	// we want to make sure the console allocates
	if( false == CORE::ConsoleAllocation("WAMP Inpuut Console"))
		return 0;

	
	// make sure our mutex and anything else we might need
	// is initialized correctly
	if( false == UTILS::InitializeUtils() )
		return 0;

  // CREATE THE WINDOW HERE!!!!!

 
	std::string input;// string that might contain input

	// create the thread that waits for input
	InputThreadHandle = CreateThread(NULL, 0, UTILS::GetConsoleInputAsString, (void*)&input, 0, &InputThreadID);
	
	// here we want to see if the thread was created, if not close the program
	if(NULL == InputThreadHandle)
		return 0;
	

	
	WAMPNE::NetworkEngine* NetworkEngine = WAMPNE::NetworkEngine::getEnginePtr();
	
	while(!QUIT) 
	{
		// WIN MAIN SHEISSE
		#ifdef CREATE_GAME_WINDOW
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// translate keystroke messages into the right format
			TranslateMessage(&msg);
			// send the message to the WindowProc function
			DispatchMessage(&msg);
			// If the message is WM_QUIT, exit the while loop
			if(msg.message == WM_QUIT)
			{
				QUIT = true;
				break;
			}
		}
	    
		#endif

		// try to obtain the object (wait 0ms then skip out)
		if(!QUIT && WAIT_OBJECT_0 == WaitForSingleObject(UTILS::InputMutex, 0))
		{
		
			// check to see if we have any input in our string
			if(input.length())
			{
				// if quit or exit, we want to live the loop
				if(input == "quit" || input == "exit")
					QUIT = true;

				#ifndef CREATE_GAME_WINDOW


				if(input == "help")
				{
					std::cout << "'quit'/'exit' closes the program\n'ne' followed by arguments sends message to network engine\nif no game window, type 'output' to receive network engine output\n";
				}

				if(input == "output")
				{

					std::list<std::string>& outStrings = NetworkEngine->getDebugOutStrings();
					while(outStrings.size())
					{
						std::cout << outStrings.front() << std::endl;
						outStrings.pop_front();
					}
				}

			 #else
			
				if(input == "help")
				{
					std::cout << "'quit'/'exit' closes the program\n to talk to the network engine start your message with the following tag 'ne' i.e. 'ne help'\n";
				}

        if(input == "ctest")
        {
          std::string temp = "ne client create";
          NetworkEngine->addMessage(temp);
          std::string temp2 = "ne client connect";
          NetworkEngine->addMessage(temp2);
        }

        if(input == "stest")
        {
          std::string temp = "ne server create";
          NetworkEngine->addMessage(temp);
        }

			 #endif
				
				NetworkEngine->addMessage(input);

				input.clear();
			}
			// release the mutex, we've checked the string for input
			// and we're now done with it
			ReleaseMutex(UTILS::InputMutex);
		}
		

		// UPDATE ENGINE 
		NetworkEngine->update(0.0f);

 
		// RETRIEVE ENGINE -- get console output, whatever

		#ifdef CREATE_GAME_WINDOW
		// PRINT TO IN GAME DEBUG CONSOLE HERE!
	    std::list<std::string> outStrings = NetworkEngine->getDebugOutStrings();
	    while(!outStrings.empty())
	    {
			window.addText(outStrings.front());
			outStrings.pop_front();
	    }

		window.addText(""); // this wont add any text, but will cause the window to update
							// every frame!
		#endif
  }
	
	// exit code to kill the thread and to close the handle
	// user typed in an exit/quit command
	DWORD exitCode;
	GetExitCodeThread(InputThreadHandle, &exitCode);
	TerminateThread(InputThreadHandle, exitCode);
	CloseHandle(InputThreadHandle);

	// clean up the mutex and anything else that UTILS needs
	UTILS::CleanUpUtils();
	
	return 0;
}