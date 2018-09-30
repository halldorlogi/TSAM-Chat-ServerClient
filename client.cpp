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

#define KNOCK_PORT_1 33745
#define KNOCK_PORT_2 33746
#define KNOCK_PORT_3 33747

using namespace std;

bool knockOnPort(sockaddr_in serv_addr, hostent* server, int portno, int sockfd){

    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
    }

    // ** SETTING UP SERVER ** //
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    // ** HOST ADDRESS IS STORED IN NETWORK BYTE ORDER ** //
    bcopy((char *) server->h_addr,
        (char *) &serv_addr.sin_addr.s_addr,
        server->h_length);
	
	//** SPECIFY THE PORT **//
    serv_addr.sin_port = htons(portno);


    // Attempt a connection to the socket.
    int connection = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (connection == -1) {
        cout << "Port: " << portno << " is closed. The server might be down." << endl;
        return false;
    }
    return true;
}

// Check if the first words of the input are valid commands
bool validateCommand(string input){
	string str = input;
    string firstWord = strtok(&input[0], " ");

	// if it's any of these words, it's valid
    if(firstWord == "CONNECT" || firstWord == "ID" || firstWord == "MSG" || firstWord == "WHO" || firstWord == "LEAVE"){
        return true;
    }
	
	//if the word is CHANGE, the next word has to be ID to be a valid command
    if(firstWord == "CHANGE"){
        str = str.substr(str.find_first_of(" ")+1);
        firstWord = strtok(&str[0], " ");
        if(firstWord == "ID"){
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {

    // ** INITIALIZING VARIABLES **//
    int knock1, knock2, sockfd;
    struct sockaddr_in serv_addr;           // Socket address structure
    struct hostent *server;
    struct timeval time;
    string line, input;
    char buffer[1000];
    fd_set masterFD, readFD;

	if (argc < 2) {
       cerr << "you need to specify a host (./client <HOSTNAME>)"  << endl;
       exit(0);
    }
	
    server = gethostbyname(argv[1]);

    if (server == NULL) {
        cerr << "ERROR, no such host\n" << endl;
        exit(0);
    }

	//open the sockets
    knock1 = socket(AF_INET, SOCK_STREAM, 0); // Open Socket
    knock2 = socket(AF_INET, SOCK_STREAM, 0); // Open Socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Open Socket
	
	//knock on the first port
    if(knockOnPort(serv_addr, server, KNOCK_PORT_1, knock1)){
        
		//if we get a response, close the socket and knock on port 2
		close(knock1);
        if(knockOnPort(serv_addr, server, KNOCK_PORT_2, knock2)){
			
			//if we get a responce, close the socket and try to connect to the final port
			close(knock2);
            if(knockOnPort(serv_addr, server, KNOCK_PORT_3, sockfd)){
				
                cout << "Here is a list of available commands" << endl << endl;
                cout << "ID                   ::      Set ID of server" << endl;
                cout << "CONNECT <USERNAME>   ::      Connect to the server" << endl;
                cout << "LEAVE                ::      Disconnect from server" << endl;
                cout << "WHO                  ::      List users on server" << endl;
                cout << "MSG <USERNAME>       ::      Send private message" << endl;
                cout << "MSG ALL              ::      Send message to everyone" << endl;
                cout << "CHANGE ID            ::      Change ID of server" << endl;
                cout << endl;
				
				
				///this will loop forever until the user inputs LEAVE
                do{
					
					//reinitialize the fd sets
                    FD_ZERO((&masterFD));
                    FD_ZERO((&readFD));
                    FD_SET(sockfd, &masterFD); //this sends and receives data
                    FD_SET(STDIN_FILENO, &readFD); //this watches the user input

                    time.tv_sec = 1;
					
					//check if something is happening on the server, waits for 1 second
                    select(sockfd + 1, &masterFD, NULL, NULL, &time);
					
					//if something is happening, read it and print it out
                    if(FD_ISSET(sockfd, &masterFD)){
                        bzero(buffer, 1000);
                            int bytesRecv = recv(sockfd, buffer, 1000, 0);
                            if(bytesRecv > 0){
                            cout << string(buffer, 0, bytesRecv) << endl;
                        }
                    }
					
					//waits for input from the user, waits for 1 second
                    select(STDIN_FILENO + 1, &readFD, NULL, NULL, &time);
					
					//if the user input something, get it and validate it.
                    if(FD_ISSET(STDIN_FILENO, &readFD)){
                        getline(cin, input);
                        if(input.size() > 0 && validateCommand(input)){
							
							//if the command is validated, send the buffer to the server
                            send(sockfd, input.c_str(), input.size() + 1, 0);
                        }
                        else{
							
							//if the comand isn't validated, alert the user.
                            cout << "That's not a valid command\n" << endl;
                        }
                    }
                }
                while(input != "LEAVE");
            }
        }
    }
	return 0;
}
