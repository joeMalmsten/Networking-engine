///////////////////////////////////////////////////////////////////////////////////////////////////
//   Filename: Server.cpp
//     Author: Joseph Malmsten
//    Purpose: UDP server for a client/server based UDP network
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
#include "Server.h"
//#include "NE_Core.h"
#include "NE_Utilities.h"

#define alpha 0.875F

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Server()
//  Purpose: constructor
///////////////////////////////////////////////////////////////////////////////////////////////////
Server::Server(): Server_Up_(false){

	//initialize the critical section and read from the input file
	Thread.ListGuardInit();
	FILE * pFile = fopen("Input.txt", "r");
	if(!pFile){
		////WAMPNE::NetworkEngine::writeDebugOut("Invalid File");
		//printf("Invalid File\n");
		exit(666);
	}

	//get the port from the input file, set current ID to 0
	fscanf(pFile, "Port: %i\n", &Port_);
	fclose(pFile);
	ID = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: ~Server()
//  Purpose: destructor
///////////////////////////////////////////////////////////////////////////////////////////////////
Server::~Server(){
	//destroy the critical section
	Thread.ListGuardDestroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Connect()
//  Purpose: connects the server and sets it up to recieve clients
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::Connect(){
	char* localIP;

	//start winsock, exit if the program fails
	int result = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(result){
		////WAMPNE::NetworkEngine::writeDebugOut("Winsock could not start up");
		//printf("Winsock could not start up\n");
		exit(666);
	}

	//get the local IP and use the port provided in Input.txt
	localhost = gethostbyname("");
	localIP = inet_ntoa(*(in_addr*)*localhost->h_addr_list);

	//set both the IP and the Port
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(Port_);
	socketAddress.sin_addr.s_addr  = inet_addr(localIP); //INADDR_ANY

	//set the socket, cleanup and exit if the socket is invalid
	listenerSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, 0); //Socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP )
	if(listenerSocket == INVALID_SOCKET){
		//assert(false);
		WSACleanup();
		////WAMPNE::NetworkEngine::writeDebugOut("WSASocket Failed");
		//printf("WSASocket Failed\n");
		exit(666);
	}

	//bind the sock, exit the program if the socket fails to bind
	result = bind(listenerSocket, (SOCKADDR*)&socketAddress, sizeof(socketAddress));
	if(result == SOCKET_ERROR){
		////WAMPNE::NetworkEngine::writeDebugOut("Could not bind socket");
		//printf("Could not bind socket\n");
		exit(666);
	}

	//set the socket up to be non blocking, and set the server to "up"
	u_long val = 1;
	ioctlsocket(listenerSocket, FIONBIO, &val);
	Server_Up_ = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: reset_counter
//  Purpose: resets the time out counter for the current user
//   Params: ID - the ID of the user being reset
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::reset_counter(int ID){
	std::list<user>::iterator walker = users.begin();
	while(walker != users.end()){
		if(ID == walker->ID_){
			walker->timeout_counter = 0;
			break;
		}
		++walker;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: get_user
//  Purpose: gets the user by an ID
//   Params: ID - the ID of the user being searched for
///////////////////////////////////////////////////////////////////////////////////////////////////
user * Server::get_user(int I_D){
	std::list<user>::iterator walker = users.begin();
	while(walker != users.end()){
		if(I_D == walker->ID_){
			return &(*walker);
		}
		++walker;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: RecievefromSockets
//  Purpose: listens for packets from the clients and handles them accordingly 
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::RecievefromSockets(void){

	//variables needed to read packets and parse users
	int bytesread = 0;
	int errCode = 0;
	int I_D = 0;
	int time_ = 0;
	char buffer[BUF_LEN];
	
	//for every packet that needs to be read per frame
	for(int numRead = 0; numRead < MAX_PACKETS_TO_READ; ++numRead)
	{
		//zero the memory of the buffer being written into and get the packet from the clients
		SecureZeroMemory((void *)buffer,BUF_LEN); //memclear
		SecureZeroMemory(&remoteEndpoint, sizeof(remoteEndpoint));
		int size = sizeof(remoteEndpoint);
		
		//attempt to read a packet
		bytesread = recvfrom(listenerSocket, buffer, BUF_LEN, 0, (SOCKADDR*)&remoteEndpoint, &size); 
		//if there was an error reading the packets
		if(bytesread == SOCKET_ERROR){
			//get the error and check to see if no packet was sent
			errCode = WSAGetLastError();
			//if the packet is bad close the program with the error code.
			if(errCode != WSAEWOULDBLOCK){
				////WAMPNE::NetworkEngine::writeDebugOut("Error on recvfrom()");
				//printf("Error on recvfrom()");
				//@TODO: Better clean up here...
				exit(errCode);
			}
		}
		//otherwise lets read the packet and handle it
		else if(bytesread > 0 && bytesread <= SUPER_SECRET_VALUE)
		{
			//if a packet was recieved serialize the packet and get the header
			packetPipe.PacketReceived();
			bool add_user = false;
			Packet packet = rawToPacket(buffer);
			Header head_ = packet.getHeader();

			//if the packet is a user trying to be added into the server
			if(head_.priority == ADD_ME){
				add_user = true;
			}	
			//check to see if the user in the list, also adds the user if add_user is true
			I_D =  check_users(remoteEndpoint, add_user);
			// if the user is alread connected to the server 
			if(I_D != -1){
				//reset the user's timeout counter, get the user, and its packet
				reset_counter(I_D);
				packet = rawToPacket(buffer);
				Header head_ = packet.getHeader();
				user * user_ = get_user(I_D);

				//if this is simply a RTT check calculate the RTT and skip the rest to save time
				if(head_.priority == RTT){
					calc_RTT(I_D, user_);
					continue;
				}
				//if we are recieving an ACK from a high priority pack go through the ACK list
				//and remove this needed ACK from the list
				if(head_.priority == ACKED){
					std::list<Ack_Pack>::iterator walker = user_->acks.begin();
					while(walker != user_->acks.end()){
						//we find the ACK vie a sequence number generated from the packet
						if(head_.sequenceNum == walker->ack_number){
							user_->acks.erase(walker);
							break;
						}
						++walker;
					}
					continue;
				}
				//if it's simply a keep alive packet update the user's time out counter
				if(head_.priority = KEEP_ALIVE){
					user_->timeout_counter = 0;
					continue;
				}
				
				//push the packet to be handled by the program recieving it
				inQue.push(packet);
			}
		}
		else
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: SendtoSockets
//  Purpose: sends the buffer to all users except the ID being passed in
//   Params: buffer - the data being sent
//        bytesread - the size of the buffer
//               ID - the person who initially sent the data
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::SendtoSockets(char buffer[], int bytesread, int ID){
	std::list<user>::iterator sending;
	int size;

	//this is an old function, do not use itd
	WAMPASSERT(true, "Don't use me anymore...");

	//send to every user except the one indicated
	sending = users.begin();
	while(sending != users.end()){
		if(ID != sending->ID_){
			size = sizeof(sending->addr);
			sendto(listenerSocket, buffer, bytesread, 0, (SOCKADDR*)&sending->addr, size);
		}
		++sending;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Shutdown
//  Purpose: gracefully shut down the server
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::Shutdown(){

	//if the server is up shut it down
	if(Server_Up_){
		Server_Up_ = false;
		int ret;



		//shut the server down, return an error and tell user if failed
		ret = shutdown(listenerSocket, SD_BOTH);
		if(ret == SOCKET_ERROR){
			DWORD error = GetLastError();
			WAMPNE::NetworkEngine::writeDebugOut("Shutdown Error");
			//printf("Shutdown Error\n");
			exit(666);
		}
		//close the socket after the server is shutdown, return an error and tell the user if fails
		ret = closesocket(listenerSocket);
		if(ret == SOCKET_ERROR){
			DWORD error_ = GetLastError();
			WAMPNE::NetworkEngine::writeDebugOut("Close socket error");
			//printf("Close socket error\n");
			exit(666);
		}
		

		WAMPNE::NetworkEngine::writeDebugOut("Server shutdown!");
		//cleanup winsock
		WSACleanup();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: clean_lists
//  Purpose: cleans the server of dead users
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::clean_lists(void){
	std::list<user>::iterator begin = quitting.begin();

	//quitting is a list of users that have left, timeout out otherwise failed
	while(begin != Server::quitting.end()){
		users.remove(*begin);
		++begin;
	}
	quitting.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: check_users
//  Purpose: adds new users, and updates the last update time on current users
//   Params: address - the address of the socket sending the current packet
//          add_user - tells us whether or not this is a new user
///////////////////////////////////////////////////////////////////////////////////////////////////
int Server::check_users(sockaddr_in address, bool add_user){
	//if we have a new user add him to the list  
	if(add_user){
		#ifdef DBG_OUT
				//WAMPNE::NetworkEngine::writeDebugOut("Client Connected");
        #endif
		user temp(ID, address, clock());
		users.push_back(temp);
		
		return ID++;
	}
	//update the timeout counter for the user and return his ID
	std::list<user>::iterator begin = users.begin();
	while(begin != users.end()){
		if(address.sin_addr.s_addr == begin->addr.sin_addr.s_addr){
			begin->time_ = clock();
			return begin->ID_;
		}
		begin++;
	}
	//if no user was found return -1
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: check_timeout
//  Purpose: checks to see if any user has timed out, and if so adds them to the quitting list
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::check_timeout(void){
	//for ever user
	std::list<user>::iterator begin = users.begin();
	while(begin != users.end()){
		//if it has been 2 seconds since thier last packet
		if(clock() - begin->time_ > 2000){
			//increment the timeout counter
			++begin->timeout_counter;
			begin->time_ = clock();
			//send a keep alive packet to see if they are there
			Packet packet_(NULL, 0, KEEP_ALIVE, 0, 0);
			#ifdef DBG_OUT
				//WAMPNE::NetworkEngine::writeDebugOut("Keepin' Alive");
            #endif
			SendData(begin->ID_, packet_);
			//if they have exceeded all of the time out chances kick them off of the server
			if(begin->timeout_counter > 3){
			#ifdef DBG_OUT
				//WAMPNE::NetworkEngine::writeDebugOut("User Kicked");
            #endif
				quitting.push_back(*begin);
			}
		}
		//go to the next user
		begin++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: init_RTT
//  Purpose: sends a RTT packet to every user to get the round trip time
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::init_RTT(void){
	std::list<user>::iterator begin = users.begin();
	while(begin != users.end()){
		Packet packet_(NULL, 0, RTT, 0, 0);
		begin->RTT_Query = clock();
		SendData(begin->ID_, packet_);
		begin++;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: calc_RTT
//  Purpose: when the user sends his RTT reply we calculate how long it took to get his latency
//   Params: ID - the ID of the user being updated
//        user_ - the information of the user
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::calc_RTT(int ID, user * user_){
	user_->Curr_RTT = clock() - user_->RTT_Query;

	user_->Avg_RTT = (unsigned)(alpha * user_->Prev_RTT + (1.0F - alpha) * user_->Curr_RTT);
	user_->Prev_RTT = user_->Curr_RTT;
	#ifdef DBG_OUT
				char buffer[BUF_LEN];\
				SecureZeroMemory(buffer, BUF_LEN);
				sprintf(buffer, "Prev RTT - %u, Curr RTT - %u, Avg RTT - %u", user_->Prev_RTT, user_->Curr_RTT, user_->Avg_RTT);
				//////WAMPNE::NetworkEngine::writeDebugOut(buffer);
    #endif
	//if they have a average round trip time of 10 seconds kick them off the server
	if(user_->Avg_RTT > 10000)
		quitting.push_back(*user_);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: thread
//  Purpose: thread created to set the recieve function of the server
//   Params: args - a void pointer to a server
///////////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI thread(void * args){
	//reinterpret the server and begin listening for clients
	Server * server = reinterpret_cast<Server *>(args);
	server->RecievefromSockets();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: SendData
//  Purpose: send a packet to a specific user
//   Params: userID - the ID of the user being sent a packet
//           packet - the data being sent
//   skipSimulation - tells us whether to simulate packets or not
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::SendData(unsigned userID, Packet &packet, bool skipSimulation)
{
	std::list<user>::iterator IT = users.begin();
	for( ;IT != users.end(); ++IT)
	{
		if(IT->ID_ != userID)
			continue;

		SendData(IT, packet, skipSimulation);
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: SendData
//  Purpose: send a packet to a specific user
//   Params:  IT - an iterator to a user
//        packet - the data being sent
//skipSimulation - tells us whether to simulate packets or not
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::SendData(std::list<user>::iterator &IT, Packet &packet, bool skipSimulation)
{
	//if we have a high priority packet set up a ACK that will need to be recieved
	Header head_ = packet.getHeader();
	if(head_.priority == HIGH){
		Ack_Pack Ack_(packet, sequence_number + packet.getSizeData() + 1, clock());
		IT->acks.push_back(Ack_);
	}
	++sequence_number;
	packetPipe.PacketSent();

#ifdef PACKET_SIMULATION
	unsigned numToSend;

	if(!skipSimulation)
	{
		packetPipe.packPipe(IT->ID_, packet, numToSend);
	}
	else
	{
		numToSend = 1;
	}

	for(unsigned i = 0; i < numToSend; ++i)
		sendto(listenerSocket, packet.sendData(), packet.getSizeData(), 0, (SOCKADDR*)&IT->addr, sizeof(IT->addr));
#else
		sendto(listenerSocket, packet.sendData(), packet.getSizeData(), 0, (SOCKADDR*)&IT->addr, sizeof(IT->addr));
#endif


}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: SendToAllNow
//  Purpose: send a packet to all users
//   Params: packet - the data being sent
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::SendToAllNow(Packet &packet)
{
	std::list<user>::iterator IT = users.begin();
	for( ; IT != users.end(); ++IT)
	{
		SendData(IT->ID_, packet);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: AddMessage
//  Purpose: adds a message 
//   Params: string - the message being added
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::AddMessage(std::string string)
{
	messages.push_back(string);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: update_acks
//  Purpose: updates the ack counters and removes users who have exceeded the limit
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::update_acks(void){
	std::list<user>::iterator walker = users.begin();
	while(walker != users.end()){
		std::list<Ack_Pack>::iterator temp = walker->acks.begin();
		while(temp != walker->acks.begin()){
			if(clock() - temp->time_ > 2000){
				SendData(walker->ID_, temp->packet_);
				++temp->ack_count;
				temp->time_ = clock();
				if(temp->ack_count > 5){
					quitting.push_back(*walker);
				}
			}
			++temp;
		}
		++walker;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function: Update
//  Purpose: updates the server and the message system
///////////////////////////////////////////////////////////////////////////////////////////////////
void Server::Update(void)
{
	//parse through the messages and handle them
	while(!messages.empty())
	{
		std::string &message = messages.front();

		if(message == "hello")
		{
			//WAMPNE::NetworkEngine::writeDebugOut("Server says hello!");
		}
		else if(message == "help")
		{
			//WAMPNE::NetworkEngine::writeDebugOut("Things I know");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server hello");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server lossinfo");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server duplicateinfo");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server delayinfo");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server numpacketssent");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server numpacketsreceived");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server allinfo");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server set lossprob 0..1000");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server set duplicateprob 0..1000");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server set delayprob 0..1000");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server set maxduplicate 0..1000");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server set printstats 0..1");
			//WAMPNE::NetworkEngine::writeDebugOut("ne server set debugsettings 0..10");
			//WAMPNE::NetworkEngine::writeDebugOut("users");

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
		else if(message == "users"){
			std::list<user>::iterator walker = users.begin();
			char buffer[BUF_LEN];
			while(walker != users.end()){
				SecureZeroMemory(buffer, BUF_LEN);
				sprintf(buffer, "User ID: %u, User RTT: %u", walker->ID_, walker->Avg_RTT); 
				//WAMPNE::NetworkEngine::writeDebugOut(buffer);
				++walker;
			}
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
			//WAMPNE::NetworkEngine::writeDebugOut("Invalid server command");
		}
		messages.pop_front();
	}

	//if the server is up recieve from the sockets and update
	if(Server_Up_)
	{
		// pre logic -- i.e. kick people maybe

		// receive packets
		RecievefromSockets();
		//check RTT once a frame
		init_RTT();

		//send determine RTT packets
		char tempC[] = "RTT";
		Packet tempP(tempC,strlen(tempC)+1,HIGH,0,0);
		SendToAllNow(tempP);


		// post logic i.e. maybe some of the old stuff: check_timeout(), clean_lists()
		update_acks();
		check_timeout();
		clean_lists();

		// send out duplicate packets for simulation
#ifdef PACKET_SIMULATION
		for(;;)
		{
			Packet packet;
			unsigned ID;
			if(packetPipe.removePacket(&packet, &ID))
			{
				SendData(ID, packet, true); // only time I should use a third parameter
			}
			else
			{
				break;
			}
		}
#endif
	}
}