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
#include "output.h"
using namespace std;

#define SERVER_INITIAL_SEQUENCE_NUMBER 4321
#define BUFFER_SIZE 524

struct connection_vars {
    uint32_t seq;           //gonna be 4322 for the most part  
    uint32_t ack;           //this ack is the ack sent by server to client. expect this number as the client's next SYN
    bool isValid;
    uint32_t max_ack;
};


void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void handle_packet(unsigned int* num_connections, string path, int sockfd, int* sequence_number, vector<connection_vars> &connection) {
    socklen_t addr_len;
    struct sockaddr_storage their_addr;
    int numbytes;
    unsigned char buf[BUFFER_SIZE] = {0};

    //RECEIVE PACKET
    addr_len = sizeof(their_addr);
    if ((numbytes = recvfrom(sockfd, buf, BUFFER_SIZE , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        cerr << "ERROR: recvfrom";
        exit(5);   
    }

    //Extract info from packet and set syn, ack, cid, flags, and payload.
    Packet p_receive(buf, numbytes-HEADER_SIZE);

    //CASE: SYN FLAG ONLY, PART 1 OF HANDSHAKE, REPLY WITH SYN-ACK
    bool SYN_FLAG_ONLY = CHECK_BIT(p_receive.get_flags(), 1) && (!CHECK_BIT(p_receive.get_flags(), 2)) && (!CHECK_BIT(p_receive.get_flags(), 0));
    if(SYN_FLAG_ONLY && (p_receive.get_cid() == 0)) {
        //print here
        print_packet_recv(p_receive.get_seq(), p_receive.get_ack(), p_receive.get_cid(), 512, 10000, p_receive.get_flags());

        (*num_connections)++;

        // ofstream file;
        // string filepath = path + "/" + to_string(*num_connections) + ".file";
        // file.open(filepath, ios::out | ios::binary | ios::app);
        // file.write((char*)p_receive.get_payload(), p_receive.get_size() - HEADER_SIZE);
        // file.close();

		unsigned int send_ack = p_receive.get_seq() + 1;
        unsigned int initial_seq_num = 4321;
        //declare struct with connection seq number and ack
        connection_vars new_connection = { initial_seq_num, send_ack, true, send_ack };
        connection.push_back(new_connection);
        Packet packet_to_send(initial_seq_num, send_ack, *num_connections, A_FLAG | S_FLAG, 0);
        packet_to_send.set_packet(NULL);
        print_packet_send(initial_seq_num, send_ack, *num_connections, 512, 10000, A_FLAG | S_FLAG);

        sendto(sockfd, packet_to_send.get_buffer(), p_receive.get_size() , 0, (struct sockaddr *)&their_addr, addr_len);
        return;
    }

    //ERROR: Drop because syn flag and connection id is NOT 0.
    if(SYN_FLAG_ONLY && (p_receive.get_cid() == 0)) {
        cerr << "Syn flag error" << endl;
        print_packet_drop(p_receive.get_seq(), p_receive.get_ack(), p_receive.get_cid(), p_receive.get_flags()); 
        return;        
    }

    //ERROR: Drop if access invalid connection
    if(!connection[p_receive.get_cid() - 1].isValid) {
        cerr << "connection error" << endl;
        print_packet_drop(p_receive.get_seq(), p_receive.get_ack(), p_receive.get_cid(), p_receive.get_flags()); 
        return;
    }



    //CASE: ACK FLAG ONLY, RECEIVE FILE AND REPLY WITH ACK. This is technically part 2 of handshake
    bool ACK_FLAG_ONLY = CHECK_BIT(p_receive.get_flags(), 2) && (!CHECK_BIT(p_receive.get_flags(), 1)) && (!CHECK_BIT(p_receive.get_flags(), 0));
    bool NO_FLAGS = !CHECK_BIT(p_receive.get_flags(), 2) && (!CHECK_BIT(p_receive.get_flags(), 1)) && (!CHECK_BIT(p_receive.get_flags(), 0));
    if(ACK_FLAG_ONLY) {
        print_packet_recv(p_receive.get_seq(), p_receive.get_ack(), p_receive.get_cid(), 512, 10000, p_receive.get_flags());    
        //TODO: Check if need this if statement because if payload is 0, still open empty file..?
        if(numbytes > 0) {
			ofstream file;
            string filepath = path + "/" + to_string(p_receive.get_cid()) + ".file";
            // string filepath = "./" + to_string(p_receive.get_cid()) + ".file";

			file.open(filepath, ios::out | ios::binary | ios::app);
			file.write((char*)p_receive.get_payload(), p_receive.get_size() - HEADER_SIZE);
			file.close();
        }

        unsigned int send_seq = p_receive.get_ack();                                            //syn = recieved ack
        unsigned int send_ack = p_receive.get_seq() + p_receive.get_size() - HEADER_SIZE;       //ack = syn + payload size
        int this_cid = p_receive.get_cid() - 1;
        connection[this_cid].seq = send_seq;
        connection[this_cid].ack = send_ack;                                    //Server expects this number from client's SYN. 
        connection[this_cid].max_ack = send_ack;
        Packet packet_to_send(connection[this_cid].seq, send_ack, p_receive.get_cid(), A_FLAG, 0);
        packet_to_send.set_packet(NULL);

        print_packet_send(send_seq, send_ack, p_receive.get_cid(), 512, 10000, A_FLAG);
        sendto(sockfd, packet_to_send.get_buffer(), p_receive.get_size() , 0, (struct sockaddr *)&their_addr, addr_len);
        return;
    }
    
    //RECEIVE SOME PAYLOAD HERE (no flags, so after the handshake...)
    else if(NO_FLAGS) {
        //TODO: Check if need this if statement because if payload is 0, still open empty file..?
        
        int this_cid = p_receive.get_cid() - 1;
        unsigned int send_seq = connection[this_cid].seq;                                       //seq = recieved ack
        unsigned int send_ack = p_receive.get_seq() + p_receive.get_size() - HEADER_SIZE;       //ack = seq + payload size

        unsigned int print_flag = A_FLAG;
        //send ack is less than max ack that has been sent... Packet is from the past...
        cerr << "send_ack: " << send_ack << " , max_ack: " << connection[this_cid].max_ack << endl;
        if(send_ack <= connection[this_cid].max_ack) {
            //print dup
            print_flag |= 0x8;
        } 
        //send ack is not a dup
        else {
            //update maxack
            connection[this_cid].max_ack = send_ack;
        }
        //modulo for printing purposes
        send_ack = send_ack % (102400 + 1);
    
        //OUT OF ORDER CHECK
        if(connection[this_cid].ack != p_receive.get_seq()) {
            print_packet_drop(p_receive.get_seq(), p_receive.get_ack(), p_receive.get_cid(), p_receive.get_flags());
            Packet packet_to_send(connection[this_cid].seq, connection[this_cid].ack, p_receive.get_cid(), A_FLAG, 0);
            packet_to_send.set_packet(NULL);
            print_packet_send(connection[this_cid].seq, connection[this_cid].ack, p_receive.get_cid(), 512, 10000, print_flag);            
            
            sendto(sockfd, packet_to_send.get_buffer(), p_receive.get_size() , 0, (struct sockaddr *)&their_addr, addr_len);

            return;
        }



        print_packet_recv(p_receive.get_seq(), p_receive.get_ack(), p_receive.get_cid(), 512, 10000, p_receive.get_flags());


        //write after check if valid
        if(numbytes > 0) {
            // cerr << "Writing with SEQ: " << p_receive.get_seq() << " and ACK: " << p_receive.get_ack() << endl;
			ofstream file;
            string filepath = path + "/" + to_string(p_receive.get_cid()) + ".file";
			file.open(filepath, ios::out | ios::binary | ios::app);
			file.write((char*)p_receive.get_payload(), p_receive.get_size() - HEADER_SIZE);
			file.close();
        }

        connection[this_cid].seq = send_seq;
        connection[this_cid].ack = send_ack;                                    //Server expects this number from client's SYN. 

        Packet packet_to_send(send_seq, send_ack, p_receive.get_cid(), A_FLAG, 0);
        packet_to_send.set_packet(NULL);
        print_packet_send(send_seq, send_ack, p_receive.get_cid(), 512, 10000, print_flag);
        sendto(sockfd, packet_to_send.get_buffer(), p_receive.get_size() , 0, (struct sockaddr *)&their_addr, addr_len);
        return;
    }


    //CASE: FIN FLAG ONLY, START TERMINATE
    bool FIN_FLAG_ONLY = CHECK_BIT(p_receive.get_flags(), 0) && !CHECK_BIT(p_receive.get_flags(), 1) && !CHECK_BIT(p_receive.get_flags(), 2);
    if(FIN_FLAG_ONLY) {
        //Send FIN-ACK after receiving FIN
        unsigned int send_seq = connection[p_receive.get_cid() - 1].seq;                              
        unsigned int send_ack = p_receive.get_seq() + 1;     
        Packet ack_packet_to_send(send_seq, send_ack, p_receive.get_cid(), A_FLAG | F_FLAG, 0);
        ack_packet_to_send.set_packet(NULL);
        // cerr << "SEQ Number: " << send_seq << "\tACK Number: " << send_ack << "\tFlags: ACK" << endl;
        print_packet_send(send_seq, send_ack, p_receive.get_cid(), 512, 10000, A_FLAG | F_FLAG);
        sendto(sockfd, ack_packet_to_send.get_buffer(), p_receive.get_size() , 0, (struct sockaddr *)&their_addr, addr_len);

        //Expect ACK in return
        if ((numbytes = recvfrom(sockfd, buf, BUFFER_SIZE , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            cerr << "ERROR: recvfrom";
            exit(5);   
        }

        Packet rec(buf, numbytes - HEADER_SIZE);
        print_packet_recv(rec.get_seq(), rec.get_ack(), rec.get_cid(), 512, 10000, rec.get_flags());
        
        //at this point, connection should be closed...
        if(rec.get_flags() == A_FLAG) {
            connection[rec.get_cid() - 1].isValid = false;
            return;
        }
        //lost packet, receive fin again
        else if(rec.get_flags() == F_FLAG) {
            return;
            // print_packet_send(send_seq, send_ack, p_receive.get_cid(), 512, 10000, A_FLAG | F_FLAG);
        }

        //if ack is lost... try again.

    }
}

void test_header_with_packets() {
    cerr << "Begin test..." << endl;

    Packet test(12345, 4321, 10, A_FLAG | S_FLAG, 1);
    char testbuffer[400];
    strncpy(testbuffer, "hello world", sizeof(testbuffer));
    test.set_packet((unsigned char*)testbuffer);

    Packet test2(test.get_buffer(), 400);
    test2.read_header();

    cerr << "End test..." << endl;
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
    vector<connection_vars> connection;

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
    //At this point, handle the packet recieved
    while(1) {
        handle_packet(&num_connections, argv[2], sockfd, &sequence_number, connection);
    }
    
    close(sockfd);
}