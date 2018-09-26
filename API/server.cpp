#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <vector>

using namespace std;

// ** ERROR FUNCTION ** //
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// ** A FUNCTION THAT SPLITS BUFFER INTO TOKENS ON " " ** //
void manageBuffer(char* buffer, string &command, string &name) {
    
    buffer[strcspn(buffer, "\n")] = 0;
    char* split;
    char* split2;
    split = strtok (buffer," ,.-");
    split2 = strtok(NULL, " ");
    command = string(split);
    if (split2 == NULL) {
        return;
    }
    name = string(split2);
}

// ** FYRIR PRÓFANIR ** //
void fillUserArray(vector<string> &users) {
    users.push_back("Hilmar");
    users.push_back("Svana");
    users.push_back("Mollý");
}

// ** A FUNCTION TO HANDLE COMMANDS ** //
void handleConnection(char* buffer, vector<string> &users, int bufferSize, int &server) {
    int n;
    string command;
    string name;
    manageBuffer(buffer, command, name);
    if (command == "CONNECT") {
        users.push_back(name);
        cout << "User " << name << " is now connected to the server!" << endl;
        bzero(buffer, 256);
        strcat(buffer, "You are now connected to the server\n");
        n = (int)write(server, buffer, strlen(buffer));
        if (n < 0) {
            cout << "ERROR writing to socket" << endl;
        }
    }
    else if (command == "WHO") {
        bzero(buffer, 256);
        for (int i = 0; i < users.size(); i++) {
            strcat(buffer, "User ");
            strcat(buffer, to_string(i + 1).c_str());
            strcat(buffer, ".");
            strcat(buffer, users[i].c_str());
            strcat(buffer, "\n");
        }
        n = (int)write(server, buffer, strlen(buffer));
        if (n < 0) {
            cout << "ERROR writing to socket" << endl;
        }
    }
    else {
        bzero(buffer, 256);
        strcat(buffer, "Invalid command\n");
        n = (int)write(server, buffer, strlen(buffer));
        if (n < 0) {
            cout << "ERROR writing to socket" << endl;
        }
    }
    
}

int main(int argc, char *argv[]) {
    
    // ** INITITALIZING VARIABLES ** //
    int listeningSocket, server, portno, n;
    int bufferSize = 256;
    char buffer[bufferSize];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    vector<string> users;
    fillUserArray(users);
    
    // ** SETTING LISTENING SOCKET ** //
    listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listeningSocket < 0)
        error("ERROR opening socket");
    
    // ** CLEARING ADDRESSES ** //
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    // ** SETTING UP SERVER ** //
    portno = 2000;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    // ** BINDING LISTENING SOCKET TO SERVER ** //
    if (::bind(listeningSocket, (struct sockaddr *) &serv_addr,
               sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    // ** LISTEN ON SOCKET FOR TRAFFIC ** //
    listen(listeningSocket, 5);
    clilen = sizeof(cli_addr);
    
    // ** ACCEPTING INCOMING CONNECTION ** //
    server = accept(listeningSocket,
                    (struct sockaddr *) &cli_addr,
                    &clilen);
    if (server < 0)
        error("ERROR on accept");
    
    // ** RUN SERVER UNTIL EXIT COMMAND ** //
    while (1) {
        bzero(buffer, 256);
        n = (int)read(server, buffer, 255);
        if (n < 0) error("ERROR reading from socket");
        cout << "Client: " << buffer << endl;
        handleConnection(buffer, users, bufferSize, server);
    }
    
    //close(newsockfd);
    //close(sockfd);*/
    return 0;
}
