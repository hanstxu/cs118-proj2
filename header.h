#ifndef HEADER
#define HEADER

#define HEADER_SIZE 12
#define A_FLAG 0x4
#define S_FLAG 0x2
#define F_FLAG 0x1

void convert_to_buffer(unsigned char* buffer, unsigned int syn, unsigned int ack,
	unsigned short cid, unsigned short flags);
	
void get_header_info(unsigned char* buffer, unsigned int& syn,
	unsigned int& ack, unsigned short& cid, unsigned short& flags);

#endif