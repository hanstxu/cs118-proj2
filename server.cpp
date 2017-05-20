#include <string.h>
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
#include <vector>
#include "packet.h"
using namespace std;

#define SERVER_INITIAL_SEQUENCE_NUMBER 4321
#define BUFFER_SIZE 524

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* void write_to_file (int num_connections, string filepath, unsigned char* buf, int numbytes){
    ofstream o_file;
    o_file.open(filepath, ios::app | ios::out | ios::binary);
    o_file.write((char*)buf, numbytes);
    cout << "filepath: " << filepath << endl << "buf: " << buf << endl;
    memset(buf, '\0', BUFFER_SIZE);
    o_file.close();
} */

void print_packet_received(uint32_t syn, uint32_t ack, uint16_t cid, uint32_t cwnd, uint32_t ss_thresh, uint16_t flags) {
    cout << "RECV " << syn << " " << ack << " " << cid << " " << cwnd << " " << ss_thresh;
    if(CHECK_BIT(flags, 2))
        cout << " ACK";

    if(CHECK_BIT(flags, 1))
        cout << " SYN";
    
    if(CHECK_BIT(flags, 0))
        cout << " FIN";
    cout << endl;
}

void handle_packet(unsigned int* num_connections, string path, int sockfd, int* sequence_number, vector<unsigned int> &connection_seq_number) {
    socklen_t addr_len;
    struct sockaddr_storage their_addr;
    int numbytes;
    // char s[INET6_ADDRSTRLEN];
    unsigned char buf[BUFFER_SIZE] = {0};
    // unsigned char header[HEADER_SIZE] = {0};
    // unsigned int syn, ack;
    // unsigned short cid, flags;


    //RECEIVE PACKET
    addr_len = sizeof(their_addr);
    if ((numbytes = recvfrom(sockfd, buf, BUFFER_SIZE , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        cerr << "ERROR: recvfrom";
        exit(5);   
    }

    //Extract info from packet and set syn, ack, cid, flags, and payload.
    Packet p_receive(buf, numbytes-HEADER_SIZE);
    print_packet_received(p_receive.get_syn(), p_receive.get_ack(), p_receive.get_cid(), 0, 0, p_receive.get_flags());


    // //CASE: SYN FLAG ONLY, PART 1 OF HANDSHAKE, REPLY WITH SYN-ACK
    bool SYN_FLAG_ONLY = CHECK_BIT(p_receive.get_flags(), 1);
    if(SYN_FLAG_ONLY && (p_receive.get_cid() == 0)) {
        (*num_connections)++;
		unsigned int send_ack = p_receive.get_syn() + 1;
        unsigned int initial_seq_num = 4321;
        connection_seq_number.push_back(initial_seq_num);
        Packet packet_to_send(connection_seq_number[*num_connections - 1], send_ack, *num_connections, A_FLAG | S_FLAG, 0);
        packet_to_send.set_packet(NULL);
        // cout << "SYN Number: " << connection_seq_number[*num_connections - 1] << "\tACK Number: " << send_ack << "\tFlags: SYN-ACK" << endl;
        sendto(sockfd, packet_to_send.get_buffer(), p_receive.get_size() , 0, (struct sockaddr *)&their_addr, addr_len);
        return;
    }

    //CASE: ACK FLAG ONLY, RECEIVE FILE AND REPLY WITH ACK.
    bool ACK_FLAG_ONLY = CHECK_BIT(p_receive.get_flags(), 2) && (!CHECK_BIT(p_receive.get_flags(), 1)) && (!CHECK_BIT(p_receive.get_flags(), 0));
    if(ACK_FLAG_ONLY) {
        //TODO: Check if need this if statement because if payload is 0, still open empty file..?
        if(numbytes > 0) {
			ofstream file;
            string filepath = to_string(*num_connections) + ".file";
			file.open(filepath, ios::out | ios::binary | ios::app);
			file.write((char*)p_receive.get_payload(), p_receive.get_size() - HEADER_SIZE);
			file.close();
        }

        unsigned int send_syn = p_receive.get_ack();                                            //syn = recieved ack
        unsigned int send_ack = p_receive.get_syn() + p_receive.get_size() - HEADER_SIZE;       //ack = syn + payload size
        connection_seq_number[p_receive.get_cid() - 1] = send_syn;                              //payload size = 0 so add 0
        Packet packet_to_send(send_syn, send_ack, p_receive.get_cid(), A_FLAG, 0);
        packet_to_send.set_packet(NULL);
        // cout << "SYN Number: " << send_syn << "\tACK Number: " << send_ack << "\tFlags: ACK" << endl;
        sendto(sockfd, packet_to_send.get_buffer(), p_receive.get_size() , 0, (struct sockaddr *)&their_addr, addr_len);
    }

    //CASE: FIN FLAG ONLY, START TERMINATE
    bool FIN_FLAG_ONLY = CHECK_BIT(p_receive.get_flags(), 0) && !CHECK_BIT(p_receive.get_flags(), 1) && !CHECK_BIT(p_receive.get_flags(), 2);
    if(FIN_FLAG_ONLY) {
        //Send ACK after receiving FIN
        unsigned int send_syn = connection_seq_number[p_receive.get_cid() - 1];                              
        unsigned int send_ack = p_receive.get_syn() + 1;     
        Packet ack_packet_to_send(send_syn, send_ack, p_receive.get_cid(), A_FLAG, 0);
        ack_packet_to_send.set_packet(NULL);
        // cout << "SYN Number: " << send_syn << "\tACK Number: " << send_ack << "\tFlags: ACK" << endl;
        sendto(sockfd, ack_packet_to_send.get_buffer(), p_receive.get_size() , 0, (struct sockaddr *)&their_addr, addr_len);

        //Send FIN right after sending ACK
        //keep send_syn the same
        send_ack = 0;   //send_ack = 0 because fin
        Packet fin_packet_to_send(send_syn, send_ack, p_receive.get_cid(), F_FLAG, 0);
        fin_packet_to_send.set_packet(NULL);
        // cout << "SYN Number: " << send_syn << "\tACK Number: " << send_ack << "\tFlags: FIN" << endl;        
        sendto(sockfd, fin_packet_to_send.get_buffer(), p_receive.get_size() , 0, (struct sockaddr *)&their_addr, addr_len);

        //Expect ACK in return
        if ((numbytes = recvfrom(sockfd, buf, BUFFER_SIZE , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            cerr << "ERROR: recvfrom";
            exit(5);   
        }

        Packet rec(buf, numbytes - HEADER_SIZE);
        print_packet_received(rec.get_syn(), rec.get_ack(), rec.get_cid(), 0, 0, rec.get_flags());


        //if ack is lost... try again.

    }
}

void test_header_with_packets() {
    cout << "Begin test..." << endl;

    Packet test(12345, 4321, 10, A_FLAG | S_FLAG, 1);
    char testbuffer[400];
    strncpy(testbuffer, "hello world", sizeof(testbuffer));
    test.set_packet((unsigned char*)testbuffer);

    Packet test2(test.get_buffer(), 400);
    test2.read_header();

    cout << "End test..." << endl;
}

int main(int argc, char *argv[])
{
    //Declare all variables
    string port;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    unsigned int num_connections = 0;
    int sequence_number = 4321;
    vector<unsigned int> connection_seq_number;

    // test_header_with_packets();

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
    cout << "     SYN   ACK  CID" << endl;
    //At this point, handle the packet recieved
    while(1) {
        handle_packet(&num_connections, argv[2], sockfd, &sequence_number, connection_seq_number);
    }
    
    close(sockfd);
}