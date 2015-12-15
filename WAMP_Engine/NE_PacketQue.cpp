#include "NE_PacketQue.h"

  Packet send()
  {
    return *packetQue.front();
  }
  Packet sendAndPop()
  {
    Packet packet = *packetQue.front();
    pop();
    return packet;
  }
  void pop()
  {
    packetQue.pop_front();
  }
  void push(Packet& packet)
  {
    packetQue.push_front();
    packetQue.sort();
  }
  bool empty();
  unsigned size();
  void clear();
