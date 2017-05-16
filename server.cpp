#include <string.h>
#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>         // std::thread
#include <vector>

#include "header.h"


#define MAXBUFLEN 100
#define MYPORT "4950"    // the port users will be connecting to
#define SERVER_INITIAL_SEQUENCE_NUMBER 4321
#define BUFFER_SIZE 512

using namespace std;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void write_to_file (int num_connections, string filepath, char* buf, int numbytes){
    ofstream o_file;
    o_file.open(filepath, ios::app | ios::out | ios::binary);
    o_file.write(buf, numbytes);
    cout << "filepath: " << filepath << endl << "buf: " << buf << endl;
    memset(buf, '\0', BUFFER_SIZE);
    o_file.close();
}

void handle_packet(int* num_connections, string path, int sockfd, int* sequence_number) {
    socklen_t addr_len;
    struct sockaddr_storage their_addr;
    int numbytes;
    // char s[INET6_ADDRSTRLEN];
    char buf[BUFFER_SIZE] = {0};
    char header[HEADER_SIZE] = {0};
    unsigned int syn, ack;
    unsigned short cid, flags;


    //RECEIVE PACKET
    addr_len = sizeof(their_addr);
    if ((numbytes = recvfrom(sockfd, buf, BUFFER_SIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        cerr << "ERROR: recvfrom";
        exit(5);
    }


    //Extract header from packet.
    convert_to_buffer(header, 12345, 0, 12, 2);         //Currently hardcoded header...
    get_header_info(header, syn, ack, cid, flags);


    //CASE: SYN FLAG ONLY, PART 1 OF HANDSHAKE
    if(flags == S_FLAG && (*num_connections == 0)) {
        memset(&header, 0, sizeof(header));
        (*num_connections)++;

        unsigned int handshake_ack = syn+1;
        //default values          syn                             ack            cid               flags    
        convert_to_buffer(header, SERVER_INITIAL_SEQUENCE_NUMBER, handshake_ack, *num_connections, A_FLAG | S_FLAG );
        
        //send part 2 of handshake
        sendto(sockfd, header, BUFFER_SIZE-1 , 0, (struct sockaddr *)&their_addr, addr_len);

        return;
    }

    //CASE: FIN FLAG ONLY, PART 1 OF TERMINATE
    if(flags == F_FLAG) {
        //SEND ACK AFTER RECEIVING FIN
        memset(&header, 0, sizeof(header));
        unsigned int fin_sequence_number = 4322;
        unsigned int fin_ack_number = syn+1;
        convert_to_buffer(header, fin_sequence_number, fin_ack_number, cid, A_FLAG);
        sendto(sockfd, header, BUFFER_SIZE-1, 0, (struct sockaddr *)&their_addr, addr_len);

        //SEND FIN AND EXPECT ACK IN RETURN
        memset(&header, 0, sizeof(header));
        fin_sequence_number = 4322;
        fin_ack_number = 0;
        convert_to_buffer(header, fin_sequence_number, fin_ack_number, cid, F_FLAG);
        sendto(sockfd, header, BUFFER_SIZE-1, 0, (struct sockaddr *)&their_addr, addr_len);
        
        //EXPECTING ACK IN RETURN...
        if ((numbytes = recvfrom(sockfd, buf, BUFFER_SIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            cerr << "ERROR: recvfrom";
            exit(5);
        }

    }


    //write to file
    if(numbytes > 0) {
        string folder = path;
        string filepath = "./" + to_string(*num_connections) + ".file";
        write_to_file(*num_connections, filepath, buf, numbytes);
    }
}



int main(int argc, char *argv[])
{
    //Declare all variables
    string port;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int num_connections = 0;
    int sequence_number = 4321;

    vector<thread> threads;

    //Handle basic command line argument inputs
    //server <PORT> <FILE-DIR>
    if(argc != 3) {
        cerr << "ERROR: You must give 2 arguments when initializing the server: ./server <PORT> <FILE-DIR>" << endl;
        exit(1);
    }

    //argv[1] = <PORT>, argv[2] = <FILE-DIR>
    port = argv[1];
    string::size_type sz;
    int portnum = stoi(port, &sz);
    if(portnum < 1024 || portnum > 65535) {
        cerr << "ERROR: Incorrect portnum." << endl;
        exit(2);
    }

    //set up hints struct
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;          //IPv4
    hints.ai_socktype = SOCK_DGRAM;     //UDP
    hints.ai_flags = AI_PASSIVE;        //use my IP

    //Get addr info
    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        cerr << "ERROR: getaddrinfo: " << gai_strerror(rv) << endl;
        exit(3);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        //make socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            cerr << "ERROR: server: socket" << endl;
            continue;
        }

        //bind socket to port we passed in to get addr info
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            cerr << "ERROR: server: bind" << endl;
            continue;
        }
        break;
    }

    if (p == NULL) {
        cerr << "ERROR: server.cpp failed to bind to socket" << endl;
        exit(4);
    }
    freeaddrinfo(servinfo);

    cerr << "server: waiting to recvfrom...\n" ;

    //At this point, handle the packet recieved
    while(1) {
        //hang at recvfrom() so the while loop is okay here..?


        handle_packet(&num_connections, argv[2], sockfd, &sequence_number);

        //threads... it hangs on the rcvfrom so can create a thread after the first rcvfrom?

        // threads.push_back(thread(handle_packet, &num_connections, argv[2], sockfd, &sequence_number));
    }

    // for(size_t i = 0; i < threads.size(); i++) {
    //     threads[i].join();
    // }

    close(sockfd);
}


    //Print useful debug info
    // printf("listener: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
    // printf("listener: packet is %d bytes long\n", numbytes);
    // buf[numbytes] = '\0';
    // printf("listener: packet contains \"%s\"\n", buf);