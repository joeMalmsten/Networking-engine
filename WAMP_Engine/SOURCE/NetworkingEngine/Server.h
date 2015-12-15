///////////////////////////////////////////////////////////////////////////////////////////////////
//   Filename: Server.h
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
#pragma once
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>
#include <list>
#include <time.h>
#include <queue>

#include "winsock2.h"
#include "Thread_Network.h"

#include "PacketPipe.h"
#include "NE_Packet.h"
#include "NE_Core.h"
#pragma comment(lib, "ws2_32.lib")
#define BUF_LEN 255
#define MAX_PACKETS_TO_READ 100
#define DBG_OUT

//contains a high priority packets, a counter, and a timer 
class Ack_Pack
{
public:
	Ack_Pack(Packet pack, unsigned ack_num, unsigned time): packet_(pack), ack_number(ack_num), ack_count(0), time_(time){};
	Packet packet_;
	unsigned ack_number;
	unsigned ack_count;
	unsigned time_;
};

//contains all of the nexessary information for every user
class user
{
public:
	user(unsigned ID, sockaddr_in address_, int time):ID_(ID), addr(address_), time_(time), timeout_counter(0), Prev_RTT(0), Curr_RTT(0), Avg_RTT(0){};
	bool operator==(user rhs){ 
		if(ID_ == rhs.ID_)
			return true; 
		return false;
	};
	unsigned ID_;
	sockaddr_in addr;
	int time_;
	unsigned timeout_counter;
	unsigned RTT_Query;
	unsigned Prev_RTT, Curr_RTT, Avg_RTT;
	unsigned packet_count;
	std::list<Ack_Pack> acks;
	//std::priority_queue<Packet> outPackets; 
	//@TODO:
	/* INCLUDE PACKET OUT QUEUE HERE! */
};

class Server
{
public:
	Server();
	~Server();
	void Connect(void);
	void RecievefromSockets(void);
	void SendtoSockets(char buffer[], int bytesread, int ID);
	void Update(void); 
	void SendToAllNow(Packet &packet);  // skips the queue portion, and sends right away
	//void SendToAllQueue(Packet &packet); // puts the packet in everyone's queue
	//void SendToUserQueue(unsigned userID, Packet &packet);
	//void SendToUserQueue(std::list<user>::iterator &IT, Packet &packet);
	void SendData(unsigned userID, Packet &packet, bool skipSimulation = false); // DO NOT PASS IN A THIRD ARGUMENT
	void SendData(std::list<user>::iterator &IT, Packet &packet, bool skipSimulation = false); // DO NOT PASS IN A THIRD ARGUMENT
	
	void Shutdown(void);// do any updating here -- receive, send, whatever.
	                   // we will call this every frame, so don't make this block


	//@TODO:
	/* NEED A SEND PACKET NOW FUNCTION */
	// void sendPacketNow(DATA HERE); // -- this is especially useful in the shutdown function when you need to tell
	//								  // clients you're about to shutdown. If you queue the shutdown packets, then close
    //                                // winsock how would you send them?
	    
	//@TODO:
	/* NEED A QUEUE PACKET FUNCTION */
	// void queuePacket(DATA HERE);   // -- allows us to append a pirority for the packets (they will be sorted)
	//                                // and sent out at the end of the update loop
	void AddMessage(std::string string);
	std::list<user> users;
	std::list<user> quitting;

private:
	int Port_;
	unsigned ID;
	unsigned sequence_number;
	Threader Thread;
	WSADATA wsaData;
	SOCKET listenerSocket;;
	sockaddr_in socketAddress;
	hostent* localhost;	
	void clean_lists(void);
	void reset_counter(int ID);
	void init_RTT(void);
	void calc_RTT(int ID, user * user_);
	void update_acks(void);
	user * get_user(int I_D);
	int check_users(sockaddr_in address, bool add_user);
	bool is_user(sockaddr_in address);
	void check_timeout(void);
	bool Server_Up_;
	
	sockaddr_in remoteEndpoint;

	WAMPNE::PacketPipe packetPipe;

	std::priority_queue<Packet> inQue;
	std::list<std::string> messages;
};
DWORD WINAPI thread(void * args);
