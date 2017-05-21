#ifndef HEADER
#define HEADER

#define HEADER_SIZE 12
#define A_FLAG 0x4
#define S_FLAG 0x2
#define F_FLAG 0x1
#define MAX_SEQ_NUM 102400
#define MAX_CWND 51200
#define SLOW_START_INC 512

#include <cstring>
#include <arpa/inet.h>

void convert_to_buffer(unsigned char* buffer, uint32_t seq, uint32_t ack,
	uint16_t cid, uint16_t flags);
	
void get_header_info(unsigned char* buffer, uint32_t& seq, uint32_t& ack,
	uint16_t& cid, uint16_t& flags);

#endif