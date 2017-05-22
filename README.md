# CS118 Project 2

Brandon Liu, 004439799
Steven Xu, 604450388

Brandon worked mainly on the server.cpp.
Steven worked mainly on the client.cpp.

## High-level design of server and client

We made additional source files for better code reorganization.

// Self-made library for header formatting, network byte order, conversion
// between buffer and variables

header.h
header.cpp

// A Packet class that organizes information about a packet

packet.cpp
packet.h

//  Self-made library for print statements (i.e. RECV, SEND, DROP) 

output.h
output.cpp

Using the UNIX SOCKETS API, we tried to make source code that would
create server and client executables that would implement TCP
at the application level on top of a UDP transport layer.

## Additional libraries

\#include <iostream>
\#include <sstream>
\#include <fstream>
\#include <sys/types.h>
\#include <sys/socket.h>
\#include <netdb.h>
\#include <netinet/in.h>
\#include <string.h>
\#include <unistd.h>
\#include <stdio.h>
\#include <stdlib.h>
\#include <errno.h>
\#include <sys/select.h>
\#include <sys/time.h>
\#include <algorithm>
\#include <vector>
\#include <cstring>
\#include <arpa/inet.h>

## Problems I ran into

Having troubles retransmitting lost packets
Dealing with lots of code and organization problems
Putting timeouts everywhere in the client.cpp

## References

To understand BSD sockets, we used Beej's Guide to Network Programming.
For any other libraries we used, we referenced documentation on
cplusplus.com, man7.org, and linux.die.net.

We also referenced the textbook and piazza for udp and tcp clarifications.