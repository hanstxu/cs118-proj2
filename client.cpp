#include <string>
#include <thread>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string.h>		// for memset
#include <unistd.h>		// POSIX operating system API
#include <stdio.h>	// for printf

#include "packet.h"
using namespace std;

#define CLIENT_START 12345

void initiate_handshake(unsigned char* buffer, char* src_ip, char* src_port,
	char* dst_ip, char* dst_port) {
	memcpy(buffer, "src-ip=", 7);
	memcpy(&buffer[7], ", src-port=", 11);
	memcpy(&buffer[18], ", dst-ip=", 9);
	memcpy(&buffer[27], ", dst-port=", 11);
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

	cout << "Initiate Handshake..." << endl;
	
	//initiate_handshake(payload, argv[1], argv[2], argv[1], argv[2]);
	Packet one(CLIENT_START, 0, 0, S_FLAG, 0);
	one.set_packet(NULL);
    cout << "SYN Number: " << CLIENT_START << "\tACK Number:    0" << "\tFlags: SYN" << endl;        

	sendto(sockfd, one.get_buffer(), HEADER_SIZE, 0, servinfo->ai_addr, servinfo->ai_addrlen);
	
	int numbytes;
	unsigned char buffer[PACKET_SIZE];
	
	numbytes = recvfrom(sockfd, buffer, PACKET_SIZE, 0, servinfo->ai_addr, &servinfo->ai_addrlen);
	if (numbytes < 0) {
		cerr << "ERROR: recvfrom";
		exit(EXIT_FAILURE);
	}
	
	Packet two(buffer, numbytes-HEADER_SIZE);
	// two.read_header();
	
	FILE* filp = fopen(argv[3], "rb");
	if (!filp) {
		cerr << "ERROR: could not open file: " << argv[3] << endl;
		exit(EXIT_FAILURE);
	}
	
	unsigned char* read_buffer = new unsigned char[PAYLOAD_SIZE];
	
	int num_bytes = fread(read_buffer, sizeof(char), PAYLOAD_SIZE, filp);
	
	Packet three(two.get_ack(), two.get_syn() + 1, two.get_cid(), A_FLAG, num_bytes);
	three.set_packet(read_buffer);
	
	cout << "SYN Number: " << two.get_ack() << "\tACK Number: " << two.get_syn() + 1 << "\tFlags: ACK" << endl;        
	sendto(sockfd, three.get_buffer(), three.get_size(), 0, servinfo->ai_addr, servinfo->ai_addrlen);
	
	//TODO: Check if this case is correct for < 512
	while (num_bytes == 512) {
		numbytes = recvfrom(sockfd, buffer, PACKET_SIZE, 0, servinfo->ai_addr, &servinfo->ai_addrlen);
		if (numbytes < 0) {
			cerr << "ERROR: recvfrom";
			exit(EXIT_FAILURE);
		}
		
		Packet receive_packet(buffer, numbytes-HEADER_SIZE);
		
		num_bytes = fread(read_buffer, sizeof(char), PAYLOAD_SIZE, filp);
		
		Packet file_packet(receive_packet.get_ack(), receive_packet.get_syn(), receive_packet.get_cid(), A_FLAG, num_bytes);
		file_packet.set_packet(read_buffer);

	    cout << "SYN Number: " << receive_packet.get_ack() << "\tACK Number: " << receive_packet.get_syn() << "\tFlags: ACK" << endl;        
		sendto(sockfd, file_packet.get_buffer(), file_packet.get_size(), 0, servinfo->ai_addr, servinfo->ai_addrlen);
	}

	//Finished sending entire file, receive the last ACK from the server...
	numbytes = recvfrom(sockfd, buffer, PACKET_SIZE, 0, servinfo->ai_addr, &servinfo->ai_addrlen);
	if (numbytes < 0) {
		cerr << "ERROR: recvfrom";
		exit(EXIT_FAILURE);
	}
	Packet receive_last_ack(buffer, num_bytes-HEADER_SIZE);


	//Send FIN to server when done.
	Packet send_fin_packet(receive_last_ack.get_ack(), 0, receive_last_ack.get_cid(), F_FLAG, 0);
	send_fin_packet.set_packet(NULL);
	
	cout << "SYN Number: " << receive_last_ack.get_ack() << "\tACK Number:    0" << "\tFlags: FIN" << endl; 
	sendto(sockfd, send_fin_packet.get_buffer(), send_fin_packet.get_size(), 0, servinfo->ai_addr, servinfo->ai_addrlen);


	//Expect an ACK after sending FIN
	numbytes = recvfrom(sockfd, buffer, PACKET_SIZE, 0, servinfo->ai_addr, &servinfo->ai_addrlen);
	if (numbytes < 0) {
		cerr << "ERROR: recvfrom";
		exit(EXIT_FAILURE);
	}
	Packet receive_fin(buffer, num_bytes-HEADER_SIZE);
	unsigned int final_syn = receive_fin.get_syn() + 1;
	unsigned int final_ack = receive_fin.get_ack();

	//Expect an FIN right after, respond with ACK
	numbytes = recvfrom(sockfd, buffer, PACKET_SIZE, 0, servinfo->ai_addr, &servinfo->ai_addrlen);
	if (numbytes < 0) {
		cerr << "ERROR: recvfrom";
		exit(EXIT_FAILURE);
	}
	// receive_packet = new Packet(buffer, num_bytes-HEADER_SIZE);

	Packet final_packet(final_ack, final_syn, receive_last_ack.get_cid(), A_FLAG, 0);	
	final_packet.set_packet(NULL);
	cout << "SYN Number: " << final_ack << "\tACK Number: " << final_syn << "\tFlags: ACK" << endl; 
	sendto(sockfd, final_packet.get_buffer(), final_packet.get_size(), 0, servinfo->ai_addr, servinfo->ai_addrlen);
	
	delete read_buffer;
	close(sockfd);
	freeaddrinfo(servinfo);
	return 0;
}