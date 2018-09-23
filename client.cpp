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


/*// An error helper
 void error(const char *msg)
 {
 perror(msg);
 exit(0);
 }*/

int main(int argc, char *argv[]) {
    int sockfd, portno, connection;
    int n;
    string address;
    struct sockaddr_in serv_addr;           // Socket address structure
    struct hostent *server;
    string line;
    
    char buffer[256];
    /*if (argc < 3) {
     fprintf(stderr,"usage %s hostname port\n", argv[0]);
     exit(0);
     }*/
    
    //server = gethostbyname(argv[1]);        // Get host from IP
    //portno = atoi(argv[2]);     // Read Port No from command line
    
    address = "localhost";
    const char *addr = address.c_str();
    
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
            
            int hincl = 1; /* 1 = on, 0 = off */
            setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &hincl, sizeof(hincl));
            
            bzero((char *) &serv_addr, sizeof(serv_addr));
            
            serv_addr.sin_family = AF_INET; // This is always set to AF_INET
            
            // Host address is stored in network byte order
            bcopy((char *) server->h_addr,
                  (char *) &serv_addr.sin_addr.s_addr,
                  server->h_length);
            
            serv_addr.sin_port = htons(portno);
            
            // Attempt a connection to the socket.
            connection = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
            if (connection == -1) {
                cout << "Port: " << portno << " CLOSED" << endl;
            }
            
            if (connection == 0) {
                cout << "Port: " << portno << " OPEN" << endl;
                // Read and write to socket
                printf("Please enter the message: ");
                bzero(buffer,256);
                fgets(buffer,255, stdin);
                n = write(sockfd, buffer, strlen(buffer));
                
                if (n < 0) {
                    
                    cout << "ERROR writing to socket" << endl;
                }
                
                bzero(buffer,256);
                n = read(sockfd, buffer, 255);
                
                if (n <= 0) {
                    cout << "ERROR reading from socket" << endl;
                }
                
                cout << buffer << endl;
                close(sockfd);
                
            }
        }
        myfile.close();
        
    }
    else cout << "unable to open file";
    
}
