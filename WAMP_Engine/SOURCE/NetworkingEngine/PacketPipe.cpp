#include <stdlib.h>
#include <time.h>

#include "PacketPipe.h"
#include "NE_Utilities.h"
//#include "NE_Core.h"


#define NO_DECISION (-1)

WAMPNE::PacketPipe::PacketPipe(void) : packetLossProb(0), packetDubplicateProb(0), maxDuplicates(1), packetDelayProb(0),
packetsSent(0), packetsReceived(0), packetsDropped(0), packetsDelayed(0), packetsDuplicated(0), printStats(false)
{
#ifdef PACKET_SIMULATION
	delayPacketTimer.startTimer(1000);
#endif
}


void WAMPNE::PacketPipe::PacketReceived(void)
{
	++packetsReceived;
}

void WAMPNE::PacketPipe::PacketSent(void)
{
	++packetsSent;
}


void WAMPNE::PacketPipe::packPipe(unsigned ID, PACKET &packet, unsigned &numToSend)
{
#ifdef PACKET_SIMULATION

	//++packetsSent;

	float totalProb = (float) packetLossProb + packetDubplicateProb + packetDelayProb;
	
	float sendProb = (totalProb < 100) ? (100 - totalProb) : 0;
	
	totalProb += sendProb;

	float pLossP      = 100.0f * (packetLossProb / totalProb);
	float pDelayP     = 100.0f * (packetDelayProb / totalProb);
	float pDuplicateP = 100.0f * (packetDubplicateProb / totalProb);
	sendProb          = 100.0f * (sendProb / totalProb);

	float lossBegin  = 0.0f;
	float lossEnd    = pLossP;

	float delayBegin	 = lossEnd;
	float delayEnd		 = delayBegin + pDelayP;

	float duplicateBegin = delayEnd;
	float duplicateEnd   = duplicateBegin + pDuplicateP;

	float sendBegin		 = duplicateEnd;
	float sendEnd		 = sendBegin + sendProb;

	unsigned option = NO_DECISION; // just default it to send
	
	while(option == NO_DECISION)
	{
		float decision = rand() % 100 + 1.0f;
		
		if(decision > lossBegin && decision <= lossEnd)
		{
			option = 0;
		}
		else if(decision > delayBegin && decision <= delayEnd)
		{
			option = 2;
		}
		else if(decision > duplicateBegin && decision <= duplicateEnd)
		{
			option = 1;
		}
		else if(decision > sendBegin && decision <= sendEnd)
		{
			option = 3;
		}	
	}

	if(option == 0)
	{


		++packetsDropped;
		numToSend = 0;

		if(printStats)
			getPacketLossInfo();

		return; // don't even need to do anything else;
	}

	if(option == 1)
	{
		// check to see if there are already packets in the queue,
		// otherwise we'll just add duplicates and add them to the duplicate list
		// (by inserting them randomlly, we can simulate delayed duplicates
		++packetsDuplicated;

		if(printStats)
			getPacketDuplicateInfo();

		numToSend = static_cast<unsigned>(rand() % maxDuplicates + 1); 
		return;
	}

	if(option == 2)
	{
		++packetsDelayed;

		if(printStats)
			getPacketDelayInfo();

		delayedPackets.push_back(packet);
		delayedID.push_back(ID);
		numToSend = 0;
		return;
	}
#endif
	numToSend = 1;
	return;
}

void WAMPNE::PacketPipe::setPrintStats(unsigned onoff)
{
	if(onoff)
	{
		printStats = true;
	}
	else
	{
		printStats = false;
	}
}

bool WAMPNE::PacketPipe::removePacket(PACKET *packet, unsigned *ID)
{
#ifdef PACKET_SIMULATION
	//WAMPASSERT(packet == NULL || ID == NULL, "Packet sent in OR ID sent in was null...");


	if(!delayedPackets.empty() && delayPacketTimer.timerOver())
	{
		if(printStats)
		{
			printf("Delayed packet sent");
			//NetworkEngine::writeDebugOut("Delayed packet sent");
		}
		*packet = delayedPackets.front();
		delayedPackets.pop_front();
		delayPacketTimer.startTimer(rand() % 1000 ); // 0ms - ?000ms
		return true;
	}
#endif
	
	return false;
}

void WAMPNE::PacketPipe::setMaxDuplicates(unsigned max)
{
	maxDuplicates = max;
}

void WAMPNE::PacketPipe::setPacketDelayProbability(unsigned prob)
{
	if(prob > 100)
		prob = 100;

	packetDelayProb = prob;
}

void WAMPNE::PacketPipe::setPacketDuplicateProbability(unsigned prob)
{
	if(prob > 100)
		prob = 100;

	packetDubplicateProb = prob;
}

void WAMPNE::PacketPipe::setPacketLossProbability(unsigned prob)
{
	if(prob > 100)
		prob = 100;

	packetLossProb = prob;
}

void WAMPNE::PacketPipe::getPacketLossInfo(void)
{
	char buffer[255];
	float totalProb = (float) packetLossProb + packetDubplicateProb + packetDelayProb;
	float sendProb = (totalProb < 100) ? (100 - totalProb) : 0;
	
	totalProb += sendProb;

	float pLossP      = 100.0f * (packetLossProb / totalProb);
	float pDelayP     = 100.0f * (packetDelayProb / totalProb);
	float pDuplicateP = 100.0f * (packetDubplicateProb / totalProb);
	sendProb          = 100.0f * (sendProb / totalProb);	

	sprintf(buffer, "Packet Loss Info => Current probability of losing a packet: %f  Actual Stats: %i / %i -> %f%", pLossP / 100.0f,  packetsDropped, packetsSent, (packetsSent) ? packetsDropped/ (float)packetsSent : 0.0f); 
	printf("%s\n", buffer);
	//NetworkEngine::writeDebugOut(buffer);
}

void WAMPNE::PacketPipe::getPacketDuplicateInfo(void)
{
	char buffer[255];
	float totalProb = (float) packetLossProb + packetDubplicateProb + packetDelayProb;
	float sendProb = (totalProb < 100) ? (100 - totalProb) : 0;
	
	totalProb += sendProb;

	float pLossP      = 100.0f * (packetLossProb / totalProb);
	float pDelayP     = 100.0f * (packetDelayProb / totalProb);
	float pDuplicateP = 100.0f * (packetDubplicateProb / totalProb);
	sendProb          = 100.0f * (sendProb / totalProb);	

	sprintf(buffer, "Packet Duplicate Info => Current probability of duplicating a packet: %f  Actual Stats: %i / %i -> %f%", pDuplicateP / 100.0f,  packetsDuplicated, packetsSent, (packetsSent)?  packetsDuplicated/ (float)packetsSent : 0.0f); 
	//NetworkEngine::writeDebugOut(buffer);
	printf("%s\n", buffer);
}

void WAMPNE::PacketPipe::getPacketDelayInfo(void)
{
	char buffer[255];
	float totalProb = (float) packetLossProb + packetDubplicateProb + packetDelayProb;
	float sendProb = (totalProb < 100) ? (100 - totalProb) : 0;
	
	totalProb += sendProb;

	float pLossP      = 100.0f * (packetLossProb / totalProb);
	float pDelayP     = 100.0f * (packetDelayProb / totalProb);
	float pDuplicateP = 100.0f * (packetDubplicateProb / totalProb);
	sendProb          = 100.0f * (sendProb / totalProb);	
	sprintf(buffer, "Packet Delay Info => Current probability of delaying a packet: %f  Actual Stats: %i / %i -> %f%", pDelayP / 100.0f,  packetsDelayed, packetsSent, (packetsSent)? packetsDelayed/ (float)packetsSent : 0.0f); 
	//NetworkEngine::writeDebugOut(buffer);
	printf("%s\n", buffer);
}

void WAMPNE::PacketPipe::getPacketsSent(void)
{
	char buffer[255];
	sprintf(buffer, "Number of packets sent: %i", packetsSent);
	//NetworkEngine::writeDebugOut(buffer);
	printf("%s\n", buffer);
}

void WAMPNE::PacketPipe::getPacketsReceived(void)
{
	char buffer[255];
	sprintf(buffer, "Number of packets received: %i", packetsReceived);
	//NetworkEngine::writeDebugOut(buffer);
	printf("%s\n", buffer);
}

void WAMPNE::PacketPipe::getAllInfo(void)
{
	getPacketsSent();
	getPacketsReceived();
	getPacketLossInfo();
	getPacketDuplicateInfo();
	getPacketDelayInfo();
}