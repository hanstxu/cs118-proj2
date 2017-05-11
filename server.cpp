#include <string>
#include <thread>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    string port;

    //server <PORT> <FILE-DIR>
    if(argc != 3) {
        cerr << "ERROR: Invalid arguments. ./server <PORT> <FILE-DIR>" << endl;
        exit(1);
    }
    //argv[1] = <PORT>
    //argv[2] = <FILE-DIR>


    port = argv[1];
    string::size_type sz;
    int portnum = stoi(port, &sz);
    if(portnum < 1024 || portnum > 65535) {
        cerr << "ERROR: Incorrect portnum." << endl;
        exit(2);
    }

}
