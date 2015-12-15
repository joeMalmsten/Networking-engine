///////////////////////////////////////////////////////////////////////////////////////////////////
//   Filename: Client.h
//     Author: Joseph Malmsten
//    Purpose: UDP client for a client/server based UDP network
//    
//     README: To run the network compile the solution and run the executable given. Type "ne help" 
//             to recieve a list of commands to use with the engine. In order to use "ne client 
//             help" or "ne server help" the corresponding server or client must be created.
//
//             In order to run a server type "ne server create", in order to create a client type 
//             "ne client create", to then connect the client to the server type "ne client connect" 
//             AFTER the client has been created. If there is no server up the client will still 
//             "connect to a server" only to be kicked off from a keep alive counter. 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Thread_Network.h"
#include "winsock2.h"
#include "Ws2tcpip.h"
#include "PacketPipe.h"
#include <time.h>
#include <iostream>
#include <string>
#include <list>
#include <queue>
#include "NE_Packet.h"
#pragma comment(lib, "ws2_32.lib")
#define BUF_LEN 255

//the client class, contains all of the definitions needed to run the client
class Client{
public:
	Client();
	~Client();
	void Initialize_Winsock(void);
	void Connect_to_Server(void);
	void Recieve(void);
	void Send(char* buffer, unsigned size, bool skipSim = false);
	void SendNow(Packet& packet);
	void AddToQueue(Packet& packet);
	void Shutdown(void);
	void Update(void);	// do any updating here -- receive, send, whatever.
	                   // we will call this every frame, so don't make this block

	void AddMessage(std::string string);
private:
	int Port_;
    std::string IP_Address_;
	WSADATA wsaData;
	SOCKET clientSocket;
    sockaddr_in socketAddress, remote;
    hostent* localhost;
	Threader Thread;
	bool connectedToServer;
	unsigned timer_;
	
	/* INCLUDE PACKET OUT QUEUE HERE */
   std::priority_queue<Packet> packetQue;
   unsigned MaxPackets;

	WAMPNE::PacketPipe packetPipe;
	std::list<std::string> messages;
};

//the thread function
DWORD WINAPI thread_client(void * args);