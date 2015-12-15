// EchoActiveObj.h: ActiveObject for receiving messages from clients and echoing them back to everyone
// CS260 Assignment 2
// Feb 27th 2010
#pragma once
#include <vector>
#include "Mutex.h"
#include "ActiveObject.h"

class EchoingThread : public ActiveObject
{
public:
	typedef std::vector<std::pair<std::string, SOCKET> > NamedSockets;

	EchoingThread (NamedSockets& clients_, Mutex& mutex_);
	virtual ~EchoingThread ();

private:
	virtual void InitThread ();
	virtual void Run ();
	virtual void FlushThread ();

	NamedSockets&    clients;   // the vector of sockets and names
	Mutex&           mutex;     // for the clients

};
