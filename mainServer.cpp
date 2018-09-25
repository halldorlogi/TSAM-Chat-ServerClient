#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <utility>
#include <vector>
#include <chrono>

#define KNOCK_PORT_1 54002
#define KNOCK_PORT_2 54001
#define MAIN_PORT 54000

using namespace std;

class ClientInfo{
public:
    int timeOfKnock1, timeOfKnock2, socketVal;
    string peerName;
    string userName;
    bool knock1, knock2;

    ClientInfo(string peerName){
        knock1 = false;
        knock2 = false;
        this->peerName = peerName;
        timeOfKnock1 = NULL;
        timeOfKnock2 = NULL;
        socketVal = 0;
    }
    ~ClientInfo(){

    }
};

vector<ClientInfo*> knockingClients;
vector<ClientInfo*> allowedClients;

void validateSocket(int sockfd) {

    if (sockfd < 0) {
        cerr << "ERROR on binding socket" << endl;
        exit(0);
    }
}

void openPort(struct sockaddr_in addr, int sockfd, int portno) {

    std::cout << "Opening port: ";
    std::cout << portno;
    std::cout << std::endl;
    // Allow reusage of address
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0){
        cerr << ("setsockopt(SO_REUSEADDR) failed") << endl;
        exit(0);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(portno);

    // Attempt to bind the socket to the host (portno) the server is currently running on
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        cerr << "Error on binding" << endl;
        exit(0);
    }
}

///manages what vector to use and what to to with it based on peer name, portno and time.
void vectorManagement(string peerName, int portno, int t){
    if(portno == KNOCK_PORT_1){
        if(knockingClients.size() == 0){
            knockingClients.push_back(new ClientInfo(peerName));
            knockingClients[0]->knock1 = true;
            knockingClients[0]->timeOfKnock1 = t;
        }
    }
    else if(portno == KNOCK_PORT_2){
        for(int i = 0 ; i < knockingClients.size() ; i++){
            cout << "this works " << i << endl;
            if(knockingClients[i]->peerName == peerName){
                knockingClients[i]->knock2 = true;
                knockingClients[i]->timeOfKnock2 = t;
            }
        }
    }
}

///checks who is knocking on a port, calls vectorManager to store information about who knocks on what port and when
void checkWhoIsKnocking(struct sockaddr_in addr, int sockfd, int portno, int len){
    //accept the connection
    int knockerSock = accept(sockfd, (struct sockaddr *)&addr, (socklen_t *)&len);
    validateSocket(knockerSock);
    //see who it is
    getpeername(knockerSock, (struct sockaddr * )&addr, (socklen_t *)&len);
    string peerName = inet_ntoa(addr.sin_addr);
    int t = chrono::duration_cast< chrono::milliseconds >(chrono::system_clock::now().time_since_epoch()).count();

    //close the connection
    close(knockerSock);
    vectorManagement(peerName, portno, t);
}

///similar to checkWhoIsKnocking but specifically for the listening port. Does not close the socket
int checkIfThisPeerIsAllowed(struct sockaddr_in addr, int sockfd, int len){
    //see who it is
    getpeername(sockfd, (struct sockaddr * )&addr, (socklen_t *)&len);
    string peerName = inet_ntoa(addr.sin_addr);
    int t = chrono::duration_cast< chrono::milliseconds >(chrono::system_clock::now().time_since_epoch()).count();
    //if they are in peersAndTime2 and have a value of less than 120000ms (2 minutes) they can connect
    for(int i = 0 ; i < knockingClients.size() ; i++){
        if(knockingClients[i]->peerName == peerName){
            if(knockingClients[i]->knock2 && t - knockingClients[i]->timeOfKnock1 < 120000){
                allowedClients.push_back(knockingClients[i]);
                knockingClients.erase(knockingClients.begin() + i);
                return 1;
            }
        }
    }
    return 0;
}

int main(){
///####################### VARIABLE INITIALIZATION #######################

    int knockSock1, knockSock2, listeningSock, newSock, socketArray[10], messageCheck, socketVal, maxSocketVal, addrlen;
    int arrayLen = 10;
    struct sockaddr_in addr;


    //the message buffer
    char message[1000];

    //a messagage sent to enyone who connects
    char* loginMessage = "Welcome to this fucking server";

    // a set of file descriptors
    fd_set masterFD;

    //initialize the socket array just in case
   /*for(int i = 0 ; i < arrayLen ; i++){
        socketArray[i] = 0;
    }*/

///##################### SETTING UP MAIN SOCKETS ########################

    //create the sockets
    knockSock1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    knockSock2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    listeningSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    validateSocket(listeningSock);

    //socket settings
    bzero((char*)&addr, sizeof(addr));

    openPort(addr, knockSock1, 54002);
    openPort(addr, knockSock2, 54001);
    openPort(addr, listeningSock, 54000);

    //listen for activity
    listen(knockSock1, 5);
    listen(knockSock2, 5);
    listen(listeningSock, 5);
    addrlen = sizeof(addr);

///##################### RUNNING BODY OF SERVER ######################
    while(true){

        //reset the fd_set
        FD_ZERO(&masterFD);
        FD_SET(knockSock1, &masterFD);
        FD_SET(knockSock2, &masterFD);
        FD_SET(listeningSock, &masterFD);
        maxSocketVal = listeningSock;

        //add active sockets to set
        for(int i = 0 ; i < allowedClients.size() ; i++){
            socketVal = allowedClients[i]->socketVal;
            if(socketVal > 0){
                FD_SET(socketVal, &masterFD);
            }
            if(socketVal > maxSocketVal){
                maxSocketVal = socketVal;
            }
        }

        //wait for something to happen on some socket. Will wait forever if nothing happens.
        select(maxSocketVal + 1, &masterFD, NULL, NULL, NULL);

        //if something happens on the knocking socket, we check who it is.
        if(FD_ISSET(knockSock1, &masterFD)){
            checkWhoIsKnocking(addr, knockSock1, 54002, addrlen);
        }

        else if(FD_ISSET(knockSock2, &masterFD)){
            checkWhoIsKnocking(addr, knockSock2, 54001, addrlen);
        }

        //if something happens on the listening socket, someone is trying to connet to the server.
        if(FD_ISSET(listeningSock, &masterFD)){
            newSock = accept(listeningSock, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
            validateSocket(newSock);
            if(checkIfThisPeerIsAllowed(addr, newSock, addrlen) == 1){
                send(newSock, loginMessage, strlen(loginMessage), 0);
            }
            else{
                close(newSock);
            }
        }

        //if something happens on any other socket, it's propably a message
        for(int i = 0 ; i < allowedClients.size() ;i++){
            socketVal = allowedClients[i]->socketVal;

            if(FD_ISSET(socketVal, &masterFD)){
                messageCheck = read(socketVal, message, 1000);
                if(messageCheck != 0){
                    message[messageCheck] = '\0';

                    //send the message to everyone except the sender and the listening socket
                    for(int j = 0 ; j < allowedClients.size() ; j++){
                        if(allowedClients[j]->socketVal != listeningSock && i != j){
                            send(allowedClients[j]->socketVal, message, strlen(message), 0);
                        }
                    }
                }
                else{
                    close(socketVal);
                    allowedClients.erase(allowedClients.begin() + i);
                }
            }
        }
    }
    return 0;
}
