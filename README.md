# tsam
Tölvusamskipti - Haustönn 2018

Chat server and client

OS we used to compile:
Linux Ubuntu 18.04

fortune and g++ compiler needs to be installed. 
This can be done by running these commands:

1. sudo apt install fortune
2. sudo apt install g++


Instructions to test/run:

1. Open a terminal window in the folder with the server file
2. Compile the server:
	g++ server.cpp -o server
3. Run the server:
	./server

4. Open another terminal window in the folder with the client file
5. Compile the client:
	g++ client.cpp -o client
6. Run the client with hostname of the server (if the server is on your localhost then you run):
	./client localhost 

7. Now you should have a connection and you can use the command that popped up on your client window 
