#include "output.h"

void print_packet_recv(uint32_t seq, uint32_t ack, uint16_t cid,
 uint32_t cwnd, uint32_t ss_thresh, uint16_t flags) {
	bool print_recv = true;
	if(!print_recv)
		return;
    cout << "RECV " << seq << " " << ack << " " << cid << " " << cwnd 
	 << " " << ss_thresh;
    if(CHECK_BIT(flags, 2))
        cout << " ACK";

    if(CHECK_BIT(flags, 1))
        cout << " SYN";
    
    if(CHECK_BIT(flags, 0))
        cout << " FIN";
    cout << endl;
}

void print_packet_send(uint32_t seq, uint32_t ack, uint16_t cid,
 uint32_t cwnd, uint32_t ss_thresh, uint16_t flags) {
	bool print_recv = true;
	if(!print_recv)
		return;

    cout << "SEND " << seq << " " << ack << " " << cid << " " <<
	 cwnd << " " << ss_thresh;
    if(CHECK_BIT(flags, 2))
        cout << " ACK";

    if(CHECK_BIT(flags, 1))
        cout << " SYN";
    
    if(CHECK_BIT(flags, 0))
        cout << " FIN";
    
    if(CHECK_BIT(flags, 3))
        cout << " DUP";
    cout << endl;
}

void print_packet_drop(uint32_t seq, uint32_t ack, uint16_t cid,
 uint16_t flags) {
	bool print_recv = true;
	if(!print_recv)
		return;

    cout << "DROP " << seq << " " << ack << " " << cid;
    if(CHECK_BIT(flags, 2))
        cout << " ACK";

    if(CHECK_BIT(flags, 1))
        cout << " SYN";
    
    if(CHECK_BIT(flags, 0))
        cout << " FIN";
    cout << endl;
}