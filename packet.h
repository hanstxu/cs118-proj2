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
	Packet();
	
	Packet(uint32_t seq, uint32_t ack, uint16_t cid, uint16_t flags,
	unsigned int payload_size);
	
	Packet(unsigned char* buffer, unsigned int payload_size);
	
	Packet &operator=(const Packet& other);
	
	~Packet();
	
	void set_packet(unsigned char* buffer);
	
	void read_buffer() const;
	
	void read_header() const;
	
	void read_payload() const;
	
	unsigned char* get_buffer() const;
	unsigned char* get_payload() const;
	unsigned int get_seq() const;
	unsigned int get_ack() const;
	unsigned short get_cid() const;
	unsigned short get_flags() const;
	unsigned int get_size() const;
private:
	unsigned char m_header[HEADER_SIZE];
	uint32_t m_seq, m_ack;
	uint16_t m_cid, m_flags;
	unsigned int m_payload_size;
	unsigned char* m_payload;
	unsigned char* m_packet;
};


#endif