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

void initiate_handshake(char* buffer, char* src_ip, char* src_port,
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
	
	//initiate_handshake(payload, argv[1], argv[2], argv[1], argv[2]);
	//sendto(sockfd, packet, HEADER_SIZE + 38, 0, servinfo->ai_addr, servinfo->ai_addrlen);
	Packet one(12345, 0, 0, S_FLAG, 0);
	
	one.set_packet("");
	
	one.read_buffer();
	
	close(sockfd);
	freeaddrinfo(servinfo);
	return 0;
}