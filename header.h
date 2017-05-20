#ifndef HEADER
#define HEADER

#define HEADER_SIZE 12
#define A_FLAG 0x4
#define S_FLAG 0x2
#define F_FLAG 0x1

#include <cstring>
#include <arpa/inet.h>

void convert_to_buffer(unsigned char* buffer, uint32_t syn, uint32_t ack,
	uint16_t cid, uint16_t flags);
	
void get_header_info(unsigned char* buffer, uint32_t& syn, uint32_t& ack,
	uint16_t& cid, uint16_t& flags);

#endif