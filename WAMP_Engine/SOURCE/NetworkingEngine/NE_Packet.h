#pragma once

#include <iostream>
#include <string.h>


const int SIZE_HEADER = 2*(sizeof(unsigned)+sizeof(unsigned char));
const int SUPER_SECRET_VALUE = 255;

//@TODO: Despite there being 256 possible values
// the one with the highest (or lowest depending on the operator <)
// value will determine which gets sent first from the priority queue
const unsigned char DISCONNECT = 0;  //  0 is the lowest value or 0 lol
const unsigned char LOW = 1;
const unsigned char MED = 2;
const unsigned char HIGH = 3;
const unsigned char ADD_ME = 4;
const unsigned char KEEP_ALIVE = 5;
const unsigned char ACKED = 6;
const unsigned char RTT = 7; // -1 is highest value or 255

struct Header
{
  unsigned ak, sequenceNum;
  unsigned char priority, sizeData;
};

struct Packet
{
  private:
  // The ack is what you recieve.
  // The sequenceNum is what you send.
  unsigned ak, sequenceNum;
  // The priority holds secret information.
  // The sizeData is the size of the data we are sending in the packet.
  unsigned char priority, sizeData;
  // The full data = the header + data_ passed in.
  char data[SUPER_SECRET_VALUE];

  public:
  // Create a Packet.
  Packet(void);
  Packet(const char* data_, unsigned sizeData_, unsigned char priority_, unsigned sequenceNum_ = 0, unsigned ak_ = 0);
  Packet(const Packet& rhs);
  const Packet& operator=(const Packet& rhs);
  bool operator<(const Packet& rhs) const;
  bool operator<=(const Packet& rhs) const;
  bool operator>(const Packet& rhs) const;
  bool operator>=(const Packet& rhs) const;
  bool operator==(const Packet& rhs) const;

  // For sending the full data.
  char* sendData(void); // GMAN -- had to edit this, send requires NON const..
  // Getting the data_ back out of the full data.
  const char* getData(void) const;
  // Get the header.
  Header getHeader(void) const; // GMAN
  // get the size
  unsigned getSizeData(void) const;
};

// Get a Packet out of the fulldata.
Packet rawToPacket(const char* raw);
