/********************************************************************************************************************************
Program:		ftserver.cpp
Author:			Abby Meunier
Email:			meuniera@oregonstate.edu
Date:			11/29/2015
Resources:
				http://www.bogotobogo.com/cplusplus/sockets_server_client.php
                http://www.tutorialspoint.com/unix_sockets/socket_server_example.html
				http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=245
********************************************************************************************************************************/

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <vector>

using namespace std;

int start_up(int server_port) {
	struct sockaddr_in server_addr;
	int sockfd;
	int opt_val = 1;

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
	
	//allow reuse of addresses
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) == -1) 
		cout << "Error setting SO_REUSEADDR" << endl;

	//bind to port
	if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		cout << "Error binding to host address." << endl;
		exit(EXIT_FAILURE);
	}

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
	addr_size = sizeof(client_addr);

	//establish new connection
	new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
	
	if(new_sockfd < 0)
		cout << "Error accepting connection." << endl;
	else {
		getnameinfo((struct sockaddr *)&client_addr, sizeof(client_addr), host, sizeof(host), service, sizeof(service), 0);
		cout << type << " connection from " << host << " established." << endl;
	}
	
	return new_sockfd;
}

void send_directory(int sockfd) {
	DIR *pdir;
	struct dirent *current;
	pdir = opendir(".");
	string new_line = "\n";
	string start = "start";

	if(!pdir)
		cout << "Error calling opendir()" << endl;
	
	if(send(sockfd, start.c_str(), strlen(start.c_str()), 0) < 0)
		cout << "Error sending start to client" << endl;	

	while((current = readdir(pdir))) {
		//send all directory contents
		cout << current->d_name << endl;
		if(send(sockfd, current->d_name, strlen(current->d_name), 0) < 0)
			cout << "Error sending " << current->d_name << " to client" << endl;	
		if(send(sockfd, new_line.c_str(), strlen(new_line.c_str()), 0) < 0)
			cout << "Error sending new_line to client" << endl;
		sleep(0.2);	
	}
	
	closedir(pdir);
}

void send_file(int data_sockfd, int control_sockfd, string file_name) {
	DIR *pdir;
	struct dirent *current;
	bool file_exists = false;
	pdir = opendir(".");
	FILE *file_fd;
	char buffer[2048];
	string msg;

	if(!pdir)
		cout << "Error calling opendir()" << endl;
	
	//check if requested file exists in directory
	while((current = readdir(pdir))) {
		if(strcmp(current->d_name, file_name.c_str()) == 0) {
			file_exists = true;
			break;
		}
	}
	closedir(pdir);

	if(file_exists) {
		//send to client
		file_fd = fopen(file_name.c_str(), "r");
		if(file_fd == NULL)
			cout << "Error opening file" << endl;
			
		msg = "SUCCESS";
		if(send(control_sockfd, msg.c_str(), strlen(msg.c_str()), 0) < 0)
			cout << "Error sending SUCCESS" << endl;

		while(fgets(buffer, sizeof(buffer), file_fd) != NULL) {
			if(send(data_sockfd, buffer, strlen(buffer), 0) < 0)
				cout << "Error sending file data" << endl;
			sleep(0.01);
		}
		fclose(file_fd);
	}
	else {
		cout << "File not found. Sending error message to client." << endl;
		msg = "FILE NOT FOUND";
		if(send(control_sockfd, msg.c_str(), strlen(msg.c_str()), 0) < 0)
			cout << "Error sending error message to client" << endl;		
	}
}

bool handle_request(int new_sockfd, int sockfd) {
	int bytes_read, i, data_sockfd, welcome_sockfd, data_port;
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
		return false;
	}
	
	//parse buffer for command arguments
	tok = strtok(buffer, "[',]\n ");
	for(i = 0; tok != NULL; i++) {
		args[i] = tok;
		tok = strtok(NULL, "[',]\n ");
	}

	//list directory or send requested file
	if(strcmp(args[2], "-l") == 0) {
		data_port = atoi(args[3]);
		welcome_sockfd = start_up(data_port);				
		
		cout << "List directory requested on port " << data_port << endl;		
		
		if(welcome_sockfd < 1)
			cout << "Error creating welcome_sockfd" << endl;

		//tell client that server is ready for data connection	
		msg = "SUCCESS";
		if(send(new_sockfd, msg, strlen(msg), 0) < 0)
			cout << "Error sending data connection message to client" << endl;
		
		data_sockfd = new_connection(welcome_sockfd, "Data");
		if(data_sockfd < 1)
			cout << "Error creating data connection" << endl;

		//send directory to client	
		cout << "Sending directory contents to port " << data_port << endl; 
		send_directory(data_sockfd);
	}	
	else if(strcmp(args[2], "-g") == 0) {
		data_port = atoi(args[4]);
		welcome_sockfd = start_up(data_port);				
		
		cout << "File '" << args[3] << "' requested on port " << args[4] << endl;
		
		if(welcome_sockfd < 1)
			cout << "Error creating welcome_sockfd" << endl;

		//tell client that server is ready for data connection	
		msg = "SUCCESS";
		if(send(new_sockfd, msg, strlen(msg), 0) < 0)
			cout << "Error sending data connection message to client" << endl;
		
		data_sockfd = new_connection(welcome_sockfd, "Data");
		if(data_sockfd < 1)
			cout << "Error creating data connection" << endl;
		
		//send requested file to client
		cout << "Sending '" << args[3] << "' to port " << args[4] << endl;
		send_file(data_sockfd, new_sockfd, args[3]);
	}
	else {
		//send error message to client
		msg = "INVALID COMMAND";
		if(send(new_sockfd, msg, strlen(msg), 0) < 0)
			cout << "Error sending error message to client" << endl;
	}

	//after request has been handled, close data socket
	if(close(data_sockfd) == -1 || close(welcome_sockfd) == -1)
		cout << "Error closing data socket" << endl;

	return true;
}


int main(int argc, char* argv[]) {
	int sockfd, control_sockfd;
	int server_port = atoi(argv[1]);

	//set up socket and start listening
	sockfd = start_up(server_port);

	//listen on server_port until program is terminated 
	while(1) {	
		cout << "Server open on " << server_port << endl;
		
		//establish control connection 
		control_sockfd = new_connection(sockfd, "Control");

		while(1) {
			//handle client request
			if(!handle_request(control_sockfd, sockfd)) 
				break;
		}
	}
	
	return 0;
}
