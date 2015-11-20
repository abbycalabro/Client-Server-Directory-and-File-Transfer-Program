/****************************************************************************************
Program:		chatserv.cpp
Assignment:     CS 372, Project 1
Author:			Abby Meunier
Email:			meuniera@oregonstate.edu
Date:			11/1/2015
Description:	Program takes port # as a command line argument, creates
				welcoming socket and begins listening for connection requests 
				on the specified port. When a connection to client is established,
				server sends messages and prints received messages to the 
				terminal. If client or server enters the message "/quit", server
				closes connection and listens for new connections until terminated
				by a supervisor.
Resources:
				http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
				http://www.tutorialspoint.com/unix_sockets/socket_server_example.html
******************************************************************************************/

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <netdb.h>
#include <stdio.h>

using namespace std;

int start_up(int server_port) {
	struct sockaddr_in server_addr;
	int sockfd;

	//create socket file descriptor
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		cout << "Error creating socket." << endl;
	}

	//initialize socket
	bzero((char *) &server_addr, sizeof(server_addr));	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(server_port);
	
	//bind to port
	if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
		cout << "Error binding to host address." << endl;

	//start listening for connections
	if(listen(sockfd, 5) < 0)
		cout << "Error listening for connections." << endl;

	return sockfd;
}

bool rec_msg(int new_sockfd) {
	int bytes_read;
	char buffer[504];

	//receive message
	bytes_read = recv(new_sockfd, buffer, sizeof(buffer), 0);
	
	//print message or close connection
	if(bytes_read > 0) {
		buffer[bytes_read] = '\0';
		cout << buffer << endl;
		return true;
	}
	else if(bytes_read < 0)
		cout << "Error receiving message from client." << endl;
	else if(bytes_read == 0) {
		//close connection
		cout << "Client has quit. Now closing connection." << endl;
		
		if(close(new_sockfd) !=  0)
			cout << "Error closing connection" << endl;
		else
			cout << "Connection closed." << endl;
		return false;
	}
}

bool send_msg(int new_sockfd) {
	string msg;
	const char *c_msg;

	//get message
	cout << "A > ";
	getline(cin, msg, '\n');
	while(msg.length() < 1 || msg.length() > 500) {
		cout << "Messages must be between 1 and 500 characters. Try again." << endl;
		cout << "A > ";
		getline(cin, msg, '\n');
	}
	
	//close connection or send message
	if(msg == "/quit") {
		if(close(new_sockfd) !=  0)
			cout << "Error closing connection." << endl;
		cout << "Connection closed." << endl;
		return false;
	}
	else {
		//send message
		c_msg = msg.c_str();
		if(send(new_sockfd, c_msg, strlen(c_msg), 0) < 0)
			cout << "Error sending message to client" << endl;
		return true;
	}
}

int new_connection(int sockfd) {
	int new_sockfd;
	struct sockaddr_in client_addr;
	socklen_t addr_size;

	//accept new connection
	addr_size = sizeof(client_addr);
	new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
	if(new_sockfd < 0)
		cout << "Error accepting connection." << endl;
	else
		cout << "Connection to client established. Awaiting first message..." << endl;
		
	return new_sockfd;
}

int main(int argc, char* argv[]) {
	int sockfd, new_sockfd;
	int server_port = atoi(argv[1]);
	
	//set up sockets and start listening
	sockfd = start_up(server_port);

	while(1) {
		cout << "Server now listening for connections..." << endl;
		
		//accept new connection 
		new_sockfd = new_connection(sockfd);
		
		//receive and send messages until client or server enters "/quit", then go back to listening		
		while(1) {	
			if(!rec_msg(new_sockfd))
				break;
			if(!send_msg(new_sockfd))
				break;			
		}
	}
	return 0;
}
