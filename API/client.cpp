#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <fstream>

using namespace std;

// ** ERROR FUNCTION ** //
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// ** A WRITE FUNCTION ** //
void sendCommand(char* buffer, int n, int sockfd) {
    
    cout << "Enter a command: ";
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);
    n = (int)write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        cout << "ERROR writing to socket" << endl;
    }
}

// ** A READ FUNCTION **//
void readCommand(char* buffer, int n, int sockfd) {
    bzero(buffer,256);
    n = (int)read(sockfd, buffer, 255);
    if (n <= 0) {
        cout << "ERROR reading from socket" << endl;
    }
    cout << buffer;
}

int main(int argc, char *argv[]) {
    
    // ** INITIALIZING VARIABLES **//
    int sockfd, portno, connection;
    int n = 0;
    string address;
    struct sockaddr_in serv_addr;           // Socket address structure
    struct hostent *server;
    string line;
    char buffer[256];
    
    // ** SETTING ADDRESS TO LOCALHOST ** //
    address = "localhost";
    const char *addr = address.c_str();
    
    // ** OPENING A FILE TO READ PORT #'S FROM ** //
    ifstream myfile("ports.txt");
    if(myfile.is_open()) {
        while(getline(myfile,line)) {
            
            cout << "Scanning port: " << line << '\n';
            portno = stoi(line);
            server = gethostbyname(addr);
            
            if (server == NULL) {
                fprintf(stderr, "ERROR, no such host\n");
                exit(0);
            }
            
            sockfd = socket(AF_INET, SOCK_STREAM, 0); // Open Socket
            
            if (sockfd < 0) {
                cout << "Error opening socket" << endl;
            }
            
            // ** APPLE COMPATIBILITY CODE ** //
            int hincl = 1; /* 1 = on, 0 = off */
            setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl));
            
            // ** SETTING UP SERVER ** //
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            
            // ** HOST ADDRESS IS STORED IN NETWORK BYTE ORDER ** //
            bcopy((char *) server->h_addr,
                  (char *) &serv_addr.sin_addr.s_addr,
                  server->h_length);
            
            serv_addr.sin_port = htons(portno);
            
            // Attempt a connection to the socket.
            connection = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
            if (connection == -1) {
                cout << "Port: " << portno << " CLOSED" << endl;
            }
            
            // ** UI FOR TESTING PURPOSES ** //
            if (connection == 0) {
                cout << "Port: " << portno << " OPEN" << endl << endl;
                cout << "Here is a list of available commands" << endl << endl;
                cout << "ID                   ::      Set ID of server" << endl;
                cout << "CONNECT <USERNAME>   ::      Connect to the server" << endl;
                cout << "LEAVE                ::      Disconnect from server" << endl;
                cout << "WHO                  ::      List users on server" << endl;
                cout << "MSG <USERNAME>       ::      Send private message" << endl;
                cout << "MSG ALL              ::      Send message to everyone" << endl;
                cout << "CHANGE ID            ::      Change ID of server" << endl;
                cout << endl;
                
                while (1) {
                    sendCommand(buffer, n, sockfd);
                    readCommand(buffer, n, sockfd);
                }
            }
        }
        //close(sockfd);
        myfile.close();
        
    }
    else cout << "unable to open file";
}



