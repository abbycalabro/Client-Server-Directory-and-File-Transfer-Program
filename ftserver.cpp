/****************************************************************************************
Program:		ftserver.cpp
Assignment:     CS 372, Project 2
Author:			Abby Meunier
Email:			meuniera@oregonstate.edu
Date:			11/29/2015
Description:	Program takes port # as a command line argument, creates
				welcoming socket and begins listening for connection requests 
				on the specified port. When a connection to client is established,
				server sends messages and prints received messages to the 
				terminal. If client or server enters the message "/quit", server
				closes connection and listens for new connections until terminated
				by a supervisor.
Resources:
				http://www.bogotobogo.com/cplusplus/sockets_server_client.php
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
#include <arpa/inet.h>

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

int new_connection(int sockfd) {
	int new_sockfd;
	struct sockaddr_in client_addr;
	socklen_t addr_size;
	char host[1024];
	char service[20];

	//establish new control connection
	addr_size = sizeof(client_addr);
	new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
	if(new_sockfd < 0)
		cout << "Error accepting connection." << endl;
	else {
		getnameinfo((struct sockaddr *)&client_addr, sizeof(client_addr), host, sizeof(host), service, sizeof(service), 0);
		cout << "Connection from " << host << " established." << endl;
	}

	return new_sockfd;
}

bool handle_request(int new_sockfd) {
	int bytes_read, i;
	char buffer[504];
	char *tok;
	char *args[504];
	const char *msg;	

	//receive command
	bytes_read = recv(new_sockfd, buffer, sizeof(buffer), 0);

	//handle errors receiving command
	if(bytes_read > 0) {
		buffer[bytes_read] = '\0';
	}
	else if(bytes_read < 0) {
		cout << "Error receiving command from client." << endl;
		exit(EXIT_FAILURE);
	}
	else if(bytes_read == 0) {
		//close connection and exit
		if(close(new_sockfd) !=  0)
			cout << "Error closing connection" << endl;
		else
			cout << "Connection closed." << endl;
		exit(EXIT_FAILURE);
	}
	
	//parse buffer for command arguments
	tok = strtok(buffer, "[',]\n ");
	for(i = 0; tok != NULL; i++) {
		args[i] = tok;
		cout << "Current tok: " << args[i] << endl;
		tok = strtok(NULL, "[',]\n ");
	}

	//handle request based on command
	if(strcmp(args[2], "-l") == 0) {
		cout << "Let me list that for for you!" << endl;
	}	
	else if(strcmp(args[2], "-g") == 0) {
		cout << "Let me get that for for you!" << endl;
	}
	else {
		//send error message to client
		msg = "INVALID COMMAND";
		if(send(new_sockfd, msg, strlen(msg), 0) < 0)
			cout << "Error sending error message to client" << endl;
	}
	return true;
}

int main(int argc, char* argv[]) {
	int sockfd, new_sockfd;
	int server_port = atoi(argv[1]);

	//set up sockets and start listening
	sockfd = start_up(server_port);

	cout << "Server open on " << server_port << endl;
	
	//accept new control connection 
	new_sockfd = new_connection(sockfd);

	if(!handle_request(new_sockfd))
		cout << "Error handling client request" << endl;

	//close data connection
	if(close(new_sockfd) == -1) 
		cout << "Error closing data socket." << endl;
	
	return 0;
}
