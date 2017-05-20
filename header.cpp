#include "header.h"

void convert_to_buffer(unsigned char* buffer, uint32_t syn, uint32_t ack,
	uint16_t cid, uint16_t flags) {
	memcpy(buffer, &syn, sizeof(int));
	memcpy(&buffer[4], &ack, sizeof(int));
	memcpy(&buffer[8], &cid, sizeof(short));
	memcpy(&buffer[10], &flags, sizeof(short));

}

void get_header_info(unsigned char* buffer, uint32_t& syn, uint32_t& ack,
	uint16_t& cid, uint16_t& flags) {
	memcpy(&syn, buffer, sizeof(int));
	memcpy(&ack, &buffer[4], sizeof(int));
	memcpy(&cid, &buffer[8], sizeof(short));
	memcpy(&flags, &buffer[10], sizeof(short));	
}