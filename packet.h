#ifndef PACKET
#define PACKET

#define PAYLOAD_SIZE 512
#define PACKET_SIZE 524

#include "header.h"
#include <iostream>
#include <string.h>
#include <stdio.h>	// for printf
using namespace std;
	
class Packet {
public:
	Packet(unsigned int syn, unsigned int ack, unsigned short cid,
	unsigned short flags, unsigned int payload_size);
	
	Packet(char* buffer, unsigned int payload_size);
	
	~Packet();
	
	void set_packet(char* buffer);
	
	void read_buffer() const;
private:
	char m_header[HEADER_SIZE];
	unsigned int m_syn, m_ack;
	unsigned short m_cid, m_flags;
	unsigned int m_payload_size;
	char* m_payload;
	char* m_packet;
};


#endif