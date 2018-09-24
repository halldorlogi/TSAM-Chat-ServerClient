#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void openPort(struct sockaddr_in serv_addr, int sockfd, int portno) {
    
    
    std::cout << "Opening port: ";
    std::cout << portno;
    std::cout << std::endl;
    // Allow reusage of address
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    // Attempt to bind the socket to the host (portno) the server is currently running on
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
}

void validateSocket(int sockfd) {
    
    if (sockfd < 0) {
        error("ERROR opening socket");
    }
}

void readBuffer(int n, struct sockaddr_in serv_addr, int newsockfd, char buffer[]) {
    
    n = read(newsockfd, buffer, 255);
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);
}

void writeBuffer(int n, int sockfd) {
    
    n = write(sockfd, "I got your message", 18);
    if (n < 0) error("ERROR writing to socket");
}

void validateConnection(int newsockfd) {
    
    if (newsockfd < 0)
        error("ERROR on accept");
}

int main(int argc, char *argv[])
{
    int sockfd50003, sockfd50001, sockfd50002, newsockfd1, newsockfd2, newsockfd3, portno1, portno2, portno3;
    socklen_t clilen; // Size of the address of the client
    char buffer[256]; // Bufffer of size 256 bytes
    struct sockaddr_in serv_addr, cli_addr; // Server/Client address
    
    std::cout << "Server up and running";
    std::cout << std::endl;
    
    sockfd50003 = socket(AF_INET, SOCK_STREAM, 0);
    sockfd50001 = socket(AF_INET, SOCK_STREAM, 0);
    sockfd50002 = socket(AF_INET, SOCK_STREAM, 0);
    
    validateSocket(sockfd50003);
    validateSocket(sockfd50001);
    validateSocket(sockfd50002);
    
    portno1 = 50003;
    portno2 = 50001;
    portno3 = 50002;
    
    // bzero copies n bytes (sizeof), each with a value of zero, into string s(serv_addr).
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    // Open port 50003
    openPort(serv_addr, sockfd50003, portno1);
    
    while (true) {
        
        // Listen on port 50003 for traffic
        listen(sockfd50003, 5);
        clilen = sizeof(cli_addr);
        
/*
        Þurfum væntanlega að breyta þessu socketi í non-blocking og þá breyta þessum
        skilyrðum eitthvað.
 
        Úr verkefnislýsingu:  It only responds to the client on port
        50002, and only if it has just seen connection attempts on the other two ports.
        Note, to do this you will need to use non-blocking sockets, and the select() call.
        In addition to the online manual for socket and select, the following example may
        be useful:
            http://www.gnu.org/software/libc/manual/html_node/Server-Example.html
 
        ** Sennilega líka fyrir næsta port
*/
        
        // Accept incoming connection from client
        newsockfd1 = accept(sockfd50003,
                            (struct sockaddr *) &cli_addr,
                            &clilen);
        
        validateConnection(newsockfd1);
        bzero(buffer,256);
        int n = 0;
        readBuffer(n, serv_addr, newsockfd1, buffer);
        writeBuffer(n, newsockfd1);
        
        // If connection made, close Port 1 and open Port 2
        if (newsockfd1 >= 0) {
            printf("Shutting down connection");
            std::cout << std::endl;
            close(newsockfd1);
            openPort(serv_addr, sockfd50001, portno2);
        }
        
        // Listen on socket2 for traffic
        listen(sockfd50001, 5);
        clilen = sizeof(cli_addr);
        
        // *** Þurfum væntanlega að breyta þessu socketi í non-blocking ***
        
        newsockfd2 = accept(sockfd50001,
                            (struct sockaddr *) &cli_addr,
                            &clilen);
        
        validateConnection(newsockfd2);
        bzero(buffer,256);
        readBuffer(n, serv_addr, newsockfd2, buffer);
        writeBuffer(n, newsockfd2);
        
        // If connection made, close Port 2 and open Port 3
        if (newsockfd2 >= 0) {
            printf("Shutting down connection");
            std::cout << std::endl;
            close(newsockfd2);
            openPort(serv_addr, sockfd50002, portno3);
            
        // Listen on socket3 for traffic
        listen(sockfd50002, 5);
        clilen = sizeof(cli_addr);
        newsockfd3 = accept(sockfd50002,
                            (struct sockaddr *) &cli_addr,
                            &clilen);
    
        validateConnection(newsockfd3);
        bzero(buffer,256);
        
        readBuffer(n, serv_addr, newsockfd3, buffer);
        writeBuffer(n, newsockfd3);
        
        std::cout << "Keeping connection alive";
        std::cout << std::endl;
        
        //close(newsockfd3);
        }
    }
    /*close(sockfd1);
     close(sockfd2);
     close(sockfd3);*/
    return 0;
}

// Kóði sem skilar address frá client
/*char clnt2Name[INET_ADDRSTRLEN];
 if (inet_ntop(AF_INET, &cli_addr.sin_addr.s_addr, clnt2Name,sizeof(clnt2Name)) != NULL){
 printf("Handling client %s/%d\n", clnt2Name, ntohs(cli_addr.sin_port));
 }
 char client2Name[strlen(clnt2Name) + 1];
 strcpy(client2Name, clnt2Name);*/





