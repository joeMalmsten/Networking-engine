// AcceptActiveObj.h: ActiveObject for accepting new connections and putting them in the vector
// CS260 Assignment 2
// Feb 25th 2010

#pragma once
#include <vector>
#include "ActiveObject.h"
#include "Mutex.h"

class AcceptingThread : public ActiveObject
{
public:
	typedef std::vector<std::pair<std::string, SOCKET> > NamedSockets;

	AcceptingThread (NamedSockets& clients_, Mutex& mutex_, SOCKET& listener_);
	virtual ~AcceptingThread ();

private:
	virtual void InitThread ();
	virtual void Run ();
	virtual void FlushThread ();

	NamedSockets&    clients;   // the vector of sockets and names
	Mutex&           mutex;     // for the clients
	SOCKET&          listener;  // we take a listening, non-blocking socket

	void AddNameSocketPair (std::string name, SOCKET& socket);

};
