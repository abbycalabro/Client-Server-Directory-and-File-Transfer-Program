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

int new_connection(int sockfd, string type) {
	int new_sockfd;
	struct sockaddr_in client_addr;
	socklen_t addr_size;
	char host[1024], service[20];

	//establish new connection
	addr_size = sizeof(client_addr);
	
	new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
	
	if(new_sockfd < 0)
		cout << "Error accepting connection." << endl;
	else {
		getnameinfo((struct sockaddr *)&client_addr, sizeof(client_addr), host, sizeof(host), service, sizeof(service), 0);
		cout << type << " connection from " << host << " established." << endl;
	}
	return new_sockfd;
}

bool handle_request(int new_sockfd, int sockfd) {
	int bytes_read, i, data_sockfd, data_port;
	char buffer[504];
	char *tok, *args[504];
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
		//cout << "Current tok: " << args[i] << endl;
		tok = strtok(NULL, "[',]\n ");
	}

	//handle request based on command
	if(strcmp(args[2], "-l") == 0) {
		data_port = atoi(args[3]);
		
		int welcome_sockfd = start_up(data_port);				
		if(welcome_sockfd < 1)
			cout << "Error creating welcome_sockfd" << endl;

		//tell client that server is ready for data connection	
		msg = "SUCCESS";
		if(send(new_sockfd, msg, strlen(msg), 0) < 0)
			cout << "Error sending data connection message to client" << endl;
		
		data_sockfd = new_connection(welcome_sockfd, "Data");
		if(data_sockfd < 1)
			cout << "Error creating data connection" << endl;

		cout << "List directory requested on port " << data_port << endl;		

		//send directory to client	
		if(send(data_sockfd, msg, strlen(msg), 0) < 0)
			cout << "Error sending directory to client" << endl;
	}	
	else if(strcmp(args[2], "-g") == 0) {
		//TODO set up TCP data connection
		
		cout << "File '" << args[3] << "' requested on port " << args[4] << endl;
	
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
	int sockfd, control_sockfd;
	int server_port = atoi(argv[1]);

	//set up sockets and start listening
	sockfd = start_up(server_port);

	cout << "Server open on " << server_port << endl;
	
	//accept new control connection 
	control_sockfd = new_connection(sockfd, "Control");

	if(!handle_request(control_sockfd, sockfd))
		cout << "Error handling client request" << endl;

	//close data connection
//	if(close(control_sockfd) == -1) 
//		cout << "Error closing data socket." << endl;
	
	return 0;
}
