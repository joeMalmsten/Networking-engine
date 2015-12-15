#include "NE_Packet.h"

Packet::Packet(void)
:priority(0), sizeData(0), sequenceNum(0), ak(0)
{}
Packet::Packet(const char* data_, unsigned sizeData_, unsigned char priority_, unsigned sequenceNum_, unsigned ak_)
:priority(priority_), sizeData(0), sequenceNum(sequenceNum_), ak(ak_)
{
  if(sizeData_ > (SUPER_SECRET_VALUE-SIZE_HEADER))
  {
    std::printf("Data to large.\n");
    return;
  }
  sizeData = (unsigned char)sizeData_;
  memcpy(data, &ak, sizeof(unsigned));
  memcpy(data+sizeof(unsigned), &sequenceNum, sizeof(unsigned));
  memcpy(data+2*sizeof(unsigned), &priority, sizeof(unsigned char));
  memcpy(data+2*sizeof(unsigned)+sizeof(unsigned char), &sizeData, sizeof(unsigned char));
  memcpy(data+SIZE_HEADER, data_, sizeData);
}
Packet::Packet(const Packet& rhs)
{
  unsigned ak=rhs.ak, sequenceNum=rhs.sequenceNum;
  unsigned char priority=rhs.priority, sizeData=rhs.sizeData;
  memcpy(data, &rhs.ak, sizeof(unsigned));
  memcpy(data+sizeof(unsigned), &rhs.sequenceNum, sizeof(unsigned));
  memcpy(data+2*sizeof(unsigned), &rhs.priority, sizeof(unsigned char));
  memcpy(data+2*sizeof(unsigned)+sizeof(unsigned char), &rhs.sizeData, sizeof(unsigned char));
  memcpy(data+SIZE_HEADER, rhs.data+SIZE_HEADER, sizeData);
}
const Packet& Packet::operator=(const Packet& rhs)
{
  unsigned ak=rhs.ak, sequenceNum=rhs.sequenceNum;
  unsigned char priority=rhs.priority, sizeData=rhs.sizeData;
  memcpy(data, &ak, sizeof(unsigned));
  memcpy(data+sizeof(unsigned), &sequenceNum, sizeof(unsigned));
  memcpy(data+2*sizeof(unsigned), &priority, sizeof(unsigned char));
  memcpy(data+2*sizeof(unsigned)+sizeof(unsigned char), &rhs.sizeData, sizeof(unsigned char));
  memcpy(data+SIZE_HEADER, rhs.data+SIZE_HEADER, sizeData);
  return *this;
}
bool Packet::operator<(const Packet& rhs) const
{
  return priority < rhs.priority;
}
bool Packet::operator<=(const Packet& rhs) const
{
  return priority <= rhs.priority;
}
bool Packet::operator>(const Packet& rhs) const
{
  return priority > rhs.priority;
}
bool Packet::operator>=(const Packet& rhs) const
{
  return priority >= rhs.priority;
}
bool Packet::operator==(const Packet& rhs) const
{
  return priority == rhs.priority;
}
char* Packet::sendData(void)
{
  return data;
}
const char* Packet::getData(void) const
{
  return &data[SIZE_HEADER];
}
Header Packet::getHeader(void) const
{
  Header head = {ak,sequenceNum,(unsigned)priority,(unsigned)sizeData};
  return head;
}

unsigned Packet::getSizeData(void) const
{
	return sizeData+SIZE_HEADER;
}

Packet rawToPacket(const char* raw)
{
  unsigned ak[1], sequenceNum[1];
  unsigned char priority[1], sizeData[1];

  memcpy(ak, raw, sizeof(unsigned));
  memcpy(sequenceNum, raw+sizeof(unsigned), sizeof(unsigned));
  memcpy(priority, raw+2*sizeof(unsigned), sizeof(unsigned char));
  memcpy(sizeData, raw+2*sizeof(unsigned)+sizeof(unsigned char), sizeof(unsigned char));
  char data[SUPER_SECRET_VALUE];
  memcpy(data, raw+SIZE_HEADER, *sizeData);
  return Packet(data, *sizeData, *priority, *sequenceNum, *ak);
}
