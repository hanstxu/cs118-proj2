#ifndef OUTPUT
#define OUTPUT

// make a overall constants header
#include "packet.h"

void print_packet_recv(uint32_t seq, uint32_t ack, uint16_t cid,
 uint32_t cwnd, uint32_t ss_thresh, uint16_t flags);
 
void print_packet_send(uint32_t seq, uint32_t ack, uint16_t cid,
 uint32_t cwnd, uint32_t ss_thresh, uint16_t flags);

void print_packet_drop(uint32_t seq, uint32_t ack, uint16_t cid,
 uint16_t flags);

#endif