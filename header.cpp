#include "header.h"

void convert_to_buffer(unsigned char* buffer, uint32_t seq, uint32_t ack,
	uint16_t cid, uint16_t flags) {
	uint32_t nbo_seq = seq %(MAX_SEQ_NUM + 1);
	nbo_seq = htonl(seq);
	uint32_t nbo_ack = htonl(ack);
	uint16_t nbo_cid = htons(cid);
	uint16_t nbo_flags = flags & 0x0007;
	nbo_flags = htons(flags);
	
	memcpy(buffer, &nbo_seq, sizeof(int));
	memcpy(&buffer[4], &nbo_ack, sizeof(int));
	memcpy(&buffer[8], &nbo_cid, sizeof(short));
	memcpy(&buffer[10], &nbo_flags, sizeof(short));
}

void get_header_info(unsigned char* buffer, uint32_t& seq, uint32_t& ack,
	uint16_t& cid, uint16_t& flags) {
	uint32_t nbo_seq;
	uint32_t nbo_ack;
	uint16_t nbo_cid;
	uint16_t nbo_flags;
	
	memcpy(&nbo_seq, buffer, sizeof(int));
	memcpy(&nbo_ack, &buffer[4], sizeof(int));
	memcpy(&nbo_cid, &buffer[8], sizeof(short));
	memcpy(&nbo_flags, &buffer[10], sizeof(short));
	
	seq = ntohl(nbo_seq);
	seq = seq %(MAX_SEQ_NUM + 1);
	ack = ntohl(nbo_ack);
	cid = ntohs(nbo_cid);
	flags = ntohs(nbo_flags);
	flags &= 0x0007;
}