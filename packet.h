#ifndef PACKET
#define PACKET

#define PAYLOAD_SIZE 512
#define PACKET_SIZE 524

#include "header.h"
#include <iostream>
#include <string.h>
#include <stdio.h>	// for printf
using namespace std;

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

class Packet {
public:
	Packet(unsigned int syn, unsigned int ack, unsigned short cid,
	unsigned short flags, unsigned int payload_size);
	
	Packet(unsigned char* buffer, unsigned int payload_size);
	
	~Packet();
	
	void set_packet(unsigned char* buffer);
	
	void read_buffer() const;
	
	void read_header() const;
	
	void read_payload() const;
	
	unsigned char* get_buffer() const;
	unsigned char* get_payload() const;
	unsigned int get_syn() const;
	unsigned int get_ack() const;
	unsigned short get_cid() const;
	unsigned short get_flags() const;
	unsigned int get_size() const;
private:
	unsigned char m_header[HEADER_SIZE];
	unsigned int m_syn, m_ack;
	unsigned short m_cid, m_flags;
	unsigned int m_payload_size;
	unsigned char* m_payload;
	unsigned char* m_packet;
};


#endif