#include <string>
#include <thread>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string.h>		// for memset
#include <unistd.h>		// POSIX operating system API
#include <stdio.h>	// for printf

#include <sys/select.h>	// using select for timeout
#include <sys/time.h>	// for the timeval structure

#include "packet.h"
#include "output.h"
using namespace std;

#define CLIENT_START 12345

// TODO: print drops

Packet handshake(int sockfd, struct addrinfo* servinfo, uint32_t& seq_num,
 uint32_t& ack_num, uint16_t& cid, uint32_t cwnd, uint32_t ss_thresh) {
	Packet one(seq_num, 0, 0, S_FLAG, 0);
	one.set_packet(NULL);

	print_packet_send(seq_num, 0, 0, cwnd, ss_thresh, S_FLAG);
	sendto(sockfd, one.get_buffer(), HEADER_SIZE, 0, servinfo->ai_addr,
	 servinfo->ai_addrlen);
	
	int numbytes;
	unsigned char buffer[PACKET_SIZE];
	
	Packet receive_packet;
	
	// 10 second timeout
	struct timeval tv;
	fd_set readfds;
	
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	
	select(sockfd + 1, &readfds, NULL, NULL, &tv);
	
	// if packet is responded to
	if (FD_ISSET(sockfd, &readfds)) {
		// check that SYN and ACK flags are set
		while (!CHECK_BIT(receive_packet.get_flags(), 1) ||
		 !CHECK_BIT(receive_packet.get_flags(), 2)) {
			numbytes = recvfrom(sockfd, buffer, PACKET_SIZE, 0, servinfo->ai_addr,
			 &servinfo->ai_addrlen);
			if (numbytes < 0) {
				cerr << "ERROR: recvfrom" << endl;
				exit(EXIT_FAILURE);
			}
			
			receive_packet = Packet(buffer, numbytes-HEADER_SIZE);
		}
	}
	else {
		cerr << "ERROR: 10 second timeout on handshake" << endl;
		close(sockfd);
		freeaddrinfo(servinfo);
		exit(EXIT_FAILURE);
	}
	
	// update seq_num, ack_num, and the client id here
	seq_num += 1;
	ack_num = receive_packet.get_seq() + 1;
	cid = receive_packet.get_cid();
	
	print_packet_recv(receive_packet.get_seq(), receive_packet.get_ack(),
	 receive_packet.get_cid(), cwnd, ss_thresh, receive_packet.get_flags());
	
	return receive_packet;
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cerr << "ERROR: You must give three arguments when initializing the server.\n";
		exit(EXIT_FAILURE);
	}
	
	int port_num = 0;
	
	try {
		port_num = stoi(argv[2]);
	}
	catch(const invalid_argument& ia) {
		cerr << "ERROR: invalid argument - " << ia.what() << '\n';
		exit(EXIT_FAILURE);
	}
	
	if (port_num < 1023 || port_num > 65535) {
		cerr << "ERROR: You must choose a port number between 1024 and 65535.\n";
		exit(EXIT_FAILURE);
	}
	
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;		// will point to the results
	
	memset(&hints, 0, sizeof hints);	// make sure the struct is empty
	hints.ai_family = AF_INET;			// support only IPv4 (36th post in piazza)
	hints.ai_socktype = SOCK_DGRAM;	// UDP stream sockets
	
	// get ready to connect
	status = getaddrinfo(argv[1], argv[2], &hints, &servinfo);
	if (status != 0) {
		cerr << "ERROR: hostname or ip address is invalid\n";
		exit(EXIT_FAILURE);
	}
	
	// make a socket
	int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (sockfd < 0) {
		cerr << "ERROR: unable to make a socket" << endl;
		freeaddrinfo(servinfo);
		exit(EXIT_FAILURE);
	}

	// Start of Handshake
	uint32_t seq_num = CLIENT_START;
	uint32_t ack_num = 0;
	uint16_t cid = 0;
	uint32_t cwnd = 512;
	uint32_t ss_thresh = 10000;
	
	Packet recv_packet = handshake(sockfd, servinfo, seq_num, ack_num, cid,
	 cwnd, ss_thresh);
	
	FILE* filp = fopen(argv[3], "rb");
	if (!filp) {
		cerr << "ERROR: could not open file: " << argv[3] << endl;
		exit(EXIT_FAILURE);
	}
	
	// Have zero_ack = ack_num for the last part of the handshake
	// Change it to zero after (same for zero_flag)
	uint16_t zero_ack = ack_num;
	uint16_t zero_flag = 0x0007 & A_FLAG;
	
	int recv_bytes;
	unsigned char buffer[PACKET_SIZE];
	unsigned char read_buffer[PAYLOAD_SIZE];
	int file_bytes = fread(read_buffer, sizeof(char), PAYLOAD_SIZE, filp);
	
	while (file_bytes > 0) {
		Packet file_packet(seq_num, zero_ack, cid, zero_flag, file_bytes);
		file_packet.set_packet(read_buffer);

		print_packet_send(seq_num, zero_ack,
		 recv_packet.get_cid(), 512, 10000, zero_flag);
		sendto(sockfd, file_packet.get_buffer(), file_packet.get_size(), 0, servinfo->ai_addr, servinfo->ai_addrlen);
		
		// 10 second timeout
		struct timeval tv;
		fd_set readfds;
		
		tv.tv_sec = 10;
		tv.tv_usec = 0;
		
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		
		select(sockfd + 1, &readfds, NULL, NULL, &tv);
	
		// if packet is responded to
		if (FD_ISSET(sockfd, &readfds)) {		
		// check to make sure the acknowledgement packet from server has the
		// correct connection id
			do {
				recv_bytes = recvfrom(sockfd, buffer, PACKET_SIZE, 0, servinfo->ai_addr, &servinfo->ai_addrlen);
				if (recv_bytes < 0) {
					cerr << "ERROR: recvfrom";
					exit(EXIT_FAILURE);
				}
				
				recv_packet = Packet(buffer, recv_bytes - HEADER_SIZE);
			}while(recv_packet.get_cid() != cid);
		}
		else {
			cerr << "ERROR: 10 second timeout while sending file" << endl;
			close(sockfd);
			freeaddrinfo(servinfo);
			exit(EXIT_FAILURE);
		}
		
		print_packet_recv(recv_packet.get_seq(), recv_packet.get_ack(),
		 cid, 0, 0, recv_packet.get_flags());
		
		// update seq_num and set zero_ack/zero_flag to zero if not equal to 0
		seq_num += file_bytes;
		if (zero_ack != 0)
			zero_ack = 0;
		if (zero_flag != 0)
			zero_flag = 0;
		
		file_bytes = fread(read_buffer, sizeof(char), PAYLOAD_SIZE, filp);
	}
	
	//Send FIN to server when done.
	Packet fin_packet(seq_num, 0, cid, F_FLAG, 0);
	fin_packet.set_packet(NULL);
	
	print_packet_send(seq_num, 0, cid, 512, 10000, F_FLAG);
	sendto(sockfd, fin_packet.get_buffer(), fin_packet.get_size(), 0,
	 servinfo->ai_addr, servinfo->ai_addrlen);

	// 10 second timeout
	struct timeval tv;
	fd_set readfds;
	
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	
	select(sockfd + 1, &readfds, NULL, NULL, &tv);
	
	// if packet is responded to
	if (FD_ISSET(sockfd, &readfds)) {		
		//Expect an FIN-ACK after sending FIN
		do {
			recv_bytes = recvfrom(sockfd, buffer, PACKET_SIZE, 0, servinfo->ai_addr,
			 &servinfo->ai_addrlen);
			if (recv_bytes < 0) {
				cerr << "ERROR: recvfrom";
				exit(EXIT_FAILURE);
			}
			
			recv_packet = Packet(buffer, recv_bytes - HEADER_SIZE);
		}while(recv_packet.get_cid() != cid ||
		 !(CHECK_BIT(recv_packet.get_flags(), 2) ||
		   CHECK_BIT(recv_packet.get_flags(), 0)));
		}
	else {
		cerr << "ERROR: 10 second timeout while waiting for ack to fin packet"
		 << endl;
		close(sockfd);
		freeaddrinfo(servinfo);
		exit(EXIT_FAILURE);
	}
	
	print_packet_recv(recv_packet.get_seq(), recv_packet.get_ack(),
	 recv_packet.get_cid(), 512, 10000, recv_packet.get_flags());
	 
	// Update sequence number to note that FIN has already been sent
	seq_num += 1;
	ack_num += 1;
	
	if (CHECK_BIT(recv_packet.get_flags(), 0)) {
		Packet final_packet(seq_num, ack_num, cid, A_FLAG, 0);
		final_packet.set_packet(NULL);
		
		print_packet_send(seq_num, ack_num, cid, 512, 10000, A_FLAG);	
		sendto(sockfd, final_packet.get_buffer(), final_packet.get_size(), 0,
		 servinfo->ai_addr, servinfo->ai_addrlen);
	}
	
	// 2 second timeout
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	
	select(sockfd + 1, &readfds, NULL, NULL, &tv);
	
	// TODO: check if multiple FINs are received and responded to
	if (FD_ISSET(sockfd, &readfds)) {
		do {
			recv_bytes = recvfrom(sockfd, buffer, PACKET_SIZE, 0,
			 servinfo->ai_addr, &servinfo->ai_addrlen);
			if (recv_bytes < 0) {
				cerr << "ERROR: recvfrom";
				exit(EXIT_FAILURE);
			}
			
			recv_packet = Packet(buffer, recv_bytes - HEADER_SIZE);
			if (CHECK_BIT(recv_packet.get_flags(), 0)) {
				Packet final_packet(seq_num, ack_num, cid, A_FLAG, 0);
				final_packet.set_packet(NULL);
				
				print_packet_send(seq_num, ack_num, cid, 512, 10000, A_FLAG);	
				sendto(sockfd, final_packet.get_buffer(), final_packet.get_size(), 0,
				 servinfo->ai_addr, servinfo->ai_addrlen);
			}
		}while(tv.tv_sec > 0 && tv.tv_usec > 0);
	}
	
	close(sockfd);
	freeaddrinfo(servinfo);
	return 0;
}