#pragma once

#include <list>
#include "NE_Packet.h"
#include "Timer.h"

#define PACKET_SIMULATION
#define DUPLICATE_PACKET 1
#define DELAY_PACKET 2
//#define 

class user;

namespace WAMPNE
{
	typedef Packet PACKET;

	class PacketPipe
	{
		public:

			PacketPipe(void);
			/* NOTE -- these functions assume there's an operator= or copy constructor, packets are small, shouldn't be too bad */
			void PacketReceived(void);
			void PacketSent(void);
			void update(void);

			void packPipe(unsigned ID, PACKET &packet, unsigned &numToSend);
			bool removePacket(PACKET *packet, unsigned *ID); // this is used for delayed packets!
			// 0 = 0 chance of it happening
			// 100 = 100 chance of it happening
			void setPacketLossProbability(unsigned prob); // 0-100 
			void setPacketDuplicateProbability(unsigned prob); // 0-100
			void setMaxDuplicates(unsigned max); // initially 
			void setPacketDelayProbability(unsigned prob); // 0 - 100, packets are delayed from 0ms - ?000ms
			void setPrintStats(unsigned onoff);

			void getPacketsSent(void);
			void getPacketsReceived(void);
			void getPacketLossInfo(void);
			void getPacketDuplicateInfo(void);
			void getPacketDelayInfo(void);
			void getAllInfo(void);


		private:								 
		
			unsigned packetLossProb;
			unsigned packetDubplicateProb;
			unsigned maxDuplicates;
			unsigned packetDelayProb;

			unsigned packetsSent;
			unsigned packetsDropped;
			unsigned packetsDelayed;
			unsigned packetsDuplicated;

			unsigned packetsReceived;

			bool printStats;



			Timer delayPacketTimer;
			std::list<PACKET> delayedPackets;
			std::list<unsigned> delayedID;
	};
}

