///////////////////////////////////////////////////////////////////////////////////////////////////
//   Filename: Client.cpp
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
#include "Client.h"
#include "NE_Core.h"
#include "NE_Packet.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Client()
//  Purpose: constructor
///////////////////////////////////////////////////////////////////////////////////////////////////
Client::Client():MaxPackets(10), timer_(0){

	//clear the winsock dats
	SecureZeroMemory(&wsaData, sizeof(wsaData));

	//set the socket and make it non-blocking
	socketAddress.sin_family = AF_INET;
	u_long val = 1;
	ioctlsocket(clientSocket, FIONBIO, &val);

	//open the input file for port/IP information
	FILE * pFile = fopen("Input.txt", "r");
	if(!pFile){
		WAMPNE::NetworkEngine::writeDebugOut("Bad File");
		//printf("Bad File\n");
		exit(666);
	}

	//collect the port at the IP
	char IP[20] = {NULL};
	fscanf(pFile, "Port: %i\n", &Port_);
	fscanf(pFile, "IP: %s\n", IP);
	IP_Address_.append(IP, strlen(IP));
	SecureZeroMemory(&IP, strlen(IP));
	fclose(pFile);
	Thread.ListGuardInit();
	connectedToServer = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: ~Client()
//  Purpose: destructor
///////////////////////////////////////////////////////////////////////////////////////////////////
Client::~Client(){
	//destroy the critical section
	Thread.ListGuardDestroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Initialize_Winsock
//  Purpose: initializes winsock for use with the client
///////////////////////////////////////////////////////////////////////////////////////////////////
void Client::Initialize_Winsock(void){
	//start winsock
	int ret = WSAStartup(MAKEWORD(2,2), &wsaData);
	//if winsock did not start up properly write what happened and exit
	if(ret != 0){
		WAMPNE::NetworkEngine::writeDebugOut("Winsock Failed to start");
		exit(666);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Connect_to_Server
//  Purpose: attempts to connect the client to a corresponding server
///////////////////////////////////////////////////////////////////////////////////////////////////
void Client::Connect_to_Server(void){


	//if the client is already connected to a server tell the user and return from the function
	if(connectedToServer)
	{
		WAMPNE::NetworkEngine::writeDebugOut("Client is already connected to a server!");
		return;
	}

	//get the local host information
	localhost = gethostbyname("");

	//get the local host IP
	const char * IP_ = IP_Address_.c_str();
	char * localIP = inet_ntoa(*(in_addr*)*localhost->h_addr_list);

	//set up the socket to be connected to the server, set the port and the IP adress
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr  = inet_addr(localIP);
	socketAddress.sin_port = htons(Port_);

	//setting the remote port and adress of the server(gotten from Input.txt)
	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = inet_addr(IP_);
	remote.sin_port = htons(Port_);


	//set the socket, return failed socket and exit if the socket fails
	clientSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, 0);
	if(clientSocket == INVALID_SOCKET){
		WAMPNE::NetworkEngine::writeDebugOut("WSASocket Failed");
		//printf("WSASocket Failed\n");
		exit(666);
	}

	//bind the socket to the server, return failure and exit is it does not bind
	int ret = bind(clientSocket, (SOCKADDR*)&socketAddress, sizeof(socketAddress));
	if(ret == SOCKET_ERROR){
		ret = WSAGetLastError();
		WAMPNE::NetworkEngine::writeDebugOut("Failed to bind socket");
		//printf("Failed to bind socket\n");
		exit(666);
	}

	//set the client socket to non blocking
	u_long val = 1;
	ioctlsocket(clientSocket, FIONBIO, &val);

	//set the client to connected
	connectedToServer = true;
	
	//send the server a ADD_ME packet so the server will add the client and his IP to the correct 
	//user list
	Packet packet_(NULL, 0, ADD_ME, 0, 0);
	Send(packet_.sendData(), packet_.getSizeData());

	//tell the user we have connected
	WAMPNE::NetworkEngine::writeDebugOut("Client connected to server!");
	CreateThread(NULL, 0, thread_client, this, 0, &Thread.threadHandle);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Shutdown
//  Purpose: shuts down the client if it is connected to a server
///////////////////////////////////////////////////////////////////////////////////////////////////
void Client::Shutdown(){

	//if the client is connected to a server
	if(connectedToServer){

		//set the connection to false;
		connectedToServer = false;
		
		//skip the rest of the current time chunk to allow for clean up
		Sleep(0);

		//shut down the client socket, if that fails tell the user and exit
		int ret = shutdown(clientSocket, SD_BOTH);
		if(ret == SOCKET_ERROR){
			DWORD error = GetLastError();
			WAMPNE::NetworkEngine::writeDebugOut("Shutdown Error");
			//printf("Shutdown Error\n");
			exit(666);
		}
	}
	//tell the user the client has shut down
	WAMPNE::NetworkEngine::writeDebugOut("Client shutdown!");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Send
//  Purpose: sends a packet to the server
//   Params: buffer - the buffer of data being sent to the server
//             size - the size of the buffer being sent
//   skipSimulation - tell the client if we are simulating packets, defaulted to false
///////////////////////////////////////////////////////////////////////////////////////////////////
void Client::Send(char* buffer, unsigned size, bool skipSimulation){
	packetPipe.PacketSent();

//if we are simulating packets for the network do debugging needs then send the packet
#ifdef PACKET_SIMULATION
	unsigned numToSend;

	if(!skipSimulation)
	{
		Packet packet = rawToPacket(buffer);

		packetPipe.packPipe(0, packet, numToSend);
	}
	else
	{
		numToSend = 1;
	}

	for(unsigned i = 0; i < numToSend; ++i)
		sendto(clientSocket, buffer, size, 0, (sockaddr*)&remote, sizeof(remote));
//otherwise just send the packet
#else
		sendto(clientSocket, buffer, size, 0, (sockaddr*)&remote, sizeof(remote));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: SendNow
//  Purpose: wrapper to send that takes a packet instead of multiple parameters
//   Params: packet - the packet of information being sent to the server
///////////////////////////////////////////////////////////////////////////////////////////////////
void Client::SendNow(Packet& packet){
	//send the packet using Send
	Send(packet.sendData(), packet.getSizeData());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: AddToQueue
//  Purpose: adds a packet to the queue
//   Params: packet - the packet of information being sent to the queue
///////////////////////////////////////////////////////////////////////////////////////////////////
void Client::AddToQueue(Packet& packet){
	packetQue.push(packet);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Recieve
//  Purpose: recieves packet from the server, runs in a seperate thread
///////////////////////////////////////////////////////////////////////////////////////////////////
void Client::Recieve(void){

	//the data needed for recieving packets
	int ret, size;
	char buffer[BUF_LEN];

	//timer to keep RTT's and keep alives
	timer_ = clock();

	//@TODO: Add Packets to Packet Pipe here!!!!!!!!!!
	// NOTE - you're going to need a mutex here for critical
	// sections because the receive function lives on it's own thread,
	// and will be writing to the pipe, and the main thread will try and read from it

	//continue looping
	while(true){
		//if we disconnected from the server return from the function to stop trying to recieve packets
		if(!connectedToServer){
			return;
		}

		//zero out the memory of the recieving buffer and listen for a packet
		SecureZeroMemory(buffer, BUF_LEN);
		size = sizeof(socketAddress);
		ret = recvfrom(clientSocket, buffer, BUF_LEN, 0, (SOCKADDR*)&socketAddress, &size);
		//if the packet is bad and causes an error tell the user and exit the program
		if(ret == SOCKET_ERROR)
		{
			ret = WSAGetLastError();
			if(ret != WSAEWOULDBLOCK){
				WAMPNE::NetworkEngine::writeDebugOut("Bad File");
				exit(666);
			}
		}
		//if the packet is valid calculate the RTT's and process the packet for its needs and priority
		else if(ret > 0)
		{
			packetPipe.PacketReceived();
			timer_ = clock();
			Packet packet = rawToPacket(buffer);
			Header header = packet.getHeader();
			Header newHeader = {0,0,0,0};

			//process the packet depending on its priority
			if(header.priority == HIGH)
			{
				//if it's a high priority package set the sequence number for ACKs and send it
				newHeader.sequenceNum = header.sequenceNum + packet.getSizeData() + 1;
				newHeader.priority = ACKED;
				Packet newPacket(NULL, 0, newHeader.priority, newHeader.sequenceNum, newHeader.ak);
				SendNow(newPacket);
			}
			else if(header.priority == KEEP_ALIVE){
				//if we recieve a keep alive from server simply reply back with another keep alive
				newHeader.priority = KEEP_ALIVE;
				Packet newPacket(NULL, 0, newHeader.priority, newHeader.sequenceNum, newHeader.ak);
				SendNow(newPacket);
			}
			else if(header.priority == RTT){
				//if we have a RTT immediatly send any packet for the server to calc the client RTT
				newHeader.priority = RTT;
				Packet newPacket(NULL, 0, newHeader.priority, newHeader.sequenceNum, newHeader.ak);
				SendNow(newPacket);
			}

		}
		//if the client has not recieved a packet in a set amount of time disconnect
		if(clock() - timer_ > 10000)
			Shutdown();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: AddMessage
//  Purpose: pushes a message back for the test harness
//   Params: string - the message being pushed back
///////////////////////////////////////////////////////////////////////////////////////////////////
void Client::AddMessage(std::string string)
{
	messages.push_back(string);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Update
//  Purpose: updates the client, sets the test harness inputs
///////////////////////////////////////////////////////////////////////////////////////////////////
void Client::Update(void)
{
	// deal with messages here
	while(!messages.empty()) 
	{
		std::string &message = messages.front();

		if(message == "hello")
		{
			WAMPNE::NetworkEngine::writeDebugOut("Client says hello!");
		}
		else if(message == "help")
		{
			WAMPNE::NetworkEngine::writeDebugOut("Things I know");
			WAMPNE::NetworkEngine::writeDebugOut("ne client hello");
			WAMPNE::NetworkEngine::writeDebugOut("ne client lossinfo");
			WAMPNE::NetworkEngine::writeDebugOut("ne client duplicateinfo");
			WAMPNE::NetworkEngine::writeDebugOut("ne client delayinfo");
			WAMPNE::NetworkEngine::writeDebugOut("ne client numpacketssent");
			WAMPNE::NetworkEngine::writeDebugOut("ne client numpacketsreceived");
			WAMPNE::NetworkEngine::writeDebugOut("ne client allinfo");
			WAMPNE::NetworkEngine::writeDebugOut("ne client set lossprob 0..1000");
			WAMPNE::NetworkEngine::writeDebugOut("ne client set duplicateprob 0..1000");
			WAMPNE::NetworkEngine::writeDebugOut("ne client set delayprob 0..1000");
			WAMPNE::NetworkEngine::writeDebugOut("ne client set maxduplicate 0..1000");
			WAMPNE::NetworkEngine::writeDebugOut("ne client set printstats 0..1");
			WAMPNE::NetworkEngine::writeDebugOut("ne client set debugsettings 0..10");

		}
		else if(message == "lossinfo")
		{
			packetPipe.getPacketLossInfo();
		}
		else if(message == "duplicateinfo")
		{
			packetPipe.getPacketDuplicateInfo();
		}
		else if(message == "delayinfo")
		{
			packetPipe.getPacketDelayInfo();
		}
		else if(message == "numpacketssent")
		{
			packetPipe.getPacketsSent();
		}
		else if(message == "numpacketsreceived")
		{
			packetPipe.getPacketsReceived();
		}
		else if(message == "allinfo")
		{
			packetPipe.getAllInfo();
		}
		/// TIME TO CHECK FOR STRINGS WITH VALUES
		else if(message.npos != message.find("set lossprob ") && message.length() >= strlen("set lossprob ") + 1)
		{
			char str0[20];
			char str1[20];
			char number[20];

			sscanf(message.c_str(), "%s %s %s", str0, str1, number);

			packetPipe.setPacketLossProbability(atoi(number));
		}
		else if(message.npos != message.find("set duplicateprob ") && message.length() >= strlen("set duplicateprob ") + 1)
		{
			char str0[20];
			char str1[20];
			char number[20];

			sscanf(message.c_str(), "%s %s %s", str0, str1, number);

			packetPipe.setPacketDuplicateProbability(atoi(number));
		}
		else if(message.npos != message.find("set delayprob ") && message.length() >= strlen("set delayprob ") + 1)
		{
			char str0[20];
			char str1[20];
			char number[20];

			sscanf(message.c_str(), "%s %s %s", str0, str1, number);

			packetPipe.setPacketDelayProbability(atoi(number));
		}
		else if(message.npos != message.find("set maxduplicate ") && message.length() >= strlen("set maxduplicate ") + 1)
		{
			char str0[20];
			char str1[20];
			char number[20];

			sscanf(message.c_str(), "%s %s %s", str0, str1, number);

			packetPipe.setMaxDuplicates(atoi(number));
		}
		else if(message.npos != message.find("set printstats ") && message.length() >= strlen("set printstats ") + 1)
		{
			char str0[20];
			char str1[20];
			char number[20];

			sscanf(message.c_str(), "%s %s %s", str0, str1, number);

			packetPipe.setPrintStats(atoi(number));
		}
		else if(message.npos != message.find("set debugsettings ") && message.length() >= strlen("set debugsettings ") + 1)
		{
			char str0[20];
			char str1[20];
			char number[20];

			sscanf(message.c_str(), "%s %s %s", str0, str1, number);

			int num = atoi(number);
			
			if(num >= 0 && num <= 10)
			{
				packetPipe.setPacketLossProbability(num * 100);
				packetPipe.setPacketDuplicateProbability(num * 100);
				packetPipe.setMaxDuplicates(1 + num);
				packetPipe.setPacketDelayProbability(num * 100);
				packetPipe.setPrintStats(num * 100);
				packetPipe.getAllInfo();
			}

		}
		else
		{
			WAMPNE::NetworkEngine::writeDebugOut("Invalid client command");
		}

		messages.pop_front();
	}

	if(connectedToServer)
	{

		//if we are simulating packets on the test harness
		#ifdef PACKET_SIMULATION
		for(;;)
		{
			Packet packet;
			unsigned ID;
			if(packetPipe.removePacket(&packet, &ID))
			{
				Send(packet.sendData(), packet.getSizeData(), true);
			}
			else
			{
				break;
			}
		}
		#endif
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: thread_client
//  Purpose: the thread function for the clients recieve abilities
//   Params: args - a void pointer to a client.
///////////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI thread_client(void * args){
	//get the client from the void pointer and start recieving messages
	Client * client = reinterpret_cast<Client *>(args);
	client->Recieve();
	return 0;
}
