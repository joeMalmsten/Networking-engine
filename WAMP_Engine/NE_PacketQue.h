#pragma once
#include "NE_Packet.h"
#include <list> 

struct PacketQue
{
  Packet send();
  Packet sendAndPop();
  void pop();
  void push(Packet& packet);
  bool empty();
  unsigned size();
  void clear();

  std::list<Packet> packetQue;
};
