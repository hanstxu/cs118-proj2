#include "header.h"
#include <cstring>
#include <stdio.h>

void convert_to_buffer(unsigned char* buffer, unsigned int syn, unsigned int ack,
	unsigned short cid, unsigned short flags) {


	// int i = 0;
	// for (; i < 4; i++)
	// 	buffer[i] = ((syn >> (24 - 8*i)) & 0xFF);
	// for (; i < 8; i++)
	// 	buffer[i] = (ack >> (24 - 8*(i - 4))) & 0xFF;
	// for (; i < 10; i++)
	// 	buffer[i] = (cid >> (8 - 8*(i - 8))) & 0xFF;
	// for (; i < 12; i++)
	// 	buffer[i] = (flags >> (8 - 8*(i - 10))) & 0xFF;
	
	//offset by 0 (initial), 4(int), 8(int), 10(short) 
	memcpy(buffer, &syn, sizeof(int));
	memcpy(&buffer[4], &ack, sizeof(int));
	memcpy(&buffer[8], &cid, sizeof(short));
	memcpy(&buffer[10], &flags, sizeof(short));

}

void get_header_info(unsigned char* buffer, unsigned int& syn,
	unsigned int& ack, unsigned short& cid, unsigned short& flags) {
	syn = 0, ack = 0, cid = 0, flags = 0;
	// int i = 0;
	// for (;i < 4; i++)
	// 	syn += buffer[i] << (24 - 8*i);
	// for (;i < 8; i++)
	// 	ack += buffer[i] << (24 - 8*(i-4));
	// for (;i < 10; i++)
	// 	cid += buffer[i] << (8 - 8*(i-8));
	// for (;i < 12; i++)
	// 	flags += buffer[i] << (8 - 8*(i-10));

	//offset by 0 (initial), 4(int), 8(int), 10(short)
	memcpy(&syn, buffer, sizeof(int));
	memcpy(&ack, &buffer[4], sizeof(int));
	memcpy(&cid, &buffer[8], sizeof(short));
	memcpy(&flags, &buffer[10], sizeof(short));	
}