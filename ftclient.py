'''
Program:		ftclient.py
Assignment:     CS 372, Project 2
Author:			Abby Meunier
Email:			meuniera@oregonstate.edu
Date:			11/29/2015
Description:	Program takes two command line arguments: server hostname and port #.
				Prompts user for a username handle and establishes a TCP connection with server.
				Once connected, client and server exchange messages until either one enters 
				the message "/quit", upon which the connection is closed and the program terminates.
Resources used:
				http://www.bogotobogo.com/python/python_network_programming_server_client_file_transfer.php
'''

from socket import *
import sys
import time

def init_contact(serverName, serverPort):
	#set up socket using validated arguments
	clientSocket = socket(AF_INET, SOCK_STREAM)

	#connect to server
	if clientSocket.connect((serverName, serverPort)) == -1:
		print "Error connecting to", serverName, "port #", serverPort
		sys.exit()

	return clientSocket

def validate():
	if len(sys.argv) < 5 or len(sys.argv) > 6:
		print "Invalid number of arguments. See README.txt for more information."
		sys.exit()
	if sys.argv[3] != '-l' and sys.argv[3] != '-g':
		print "Invalid <command> argument. See README.txt for more information."
		sys.exit()
	elif sys.argv[3] == '-l' and len(sys.argv) != 5:
		print "Invalid number of arguments for command '-l'. See README.txt for more information."
		sys.exit()
	elif sys.argv[3] == '-g' and len(sys.argv) != 6:
		print "Invalid number of arguments for command '-g'. See README.txt for more information."
		sys.exit()

def make_request(command, clientSocket):
	#client quits or sends message
	if command == "/quit":
		print "Closing connection."
		clientSocket.close()
		print "Connection closed. Terminating program."
		exit()
	else:
		clientSocket.send(command)

def receive_msg(clientSocket, serverName, serverPort):
	#receive and print message, or end program
	response = clientSocket.recv(500)
	if len(response) == 0:
		print "Server has quit. Closing connection."
		clientSocket.close()
		print "Connection closed. Now terminating."
		sys.exit()
	elif response == "SUCCESS":
		return response
	else:
		print "%s:%d says: %s" % (serverName, serverPort, response)
		return response

def main():
	#validate command line arguments
	#validate() 
	
	serverName = sys.argv[1]
	serverPort = int(sys.argv[2])
	command = sys.argv[3]

	#request control connection to server
	clientSocket = init_contact(serverName, serverPort)
	
	#send command and receive response to control connection
	make_request(str(sys.argv[1:]), clientSocket)
	control_response = receive_msg(clientSocket, serverName, serverPort)

	if control_response == "SUCCESS":
		#create data socket
		if command == '-l':
			dataPort = int(sys.argv[4])
			dataSocket = init_contact(serverName, dataPort)
			#data_response = receive_msg(dataSocket, serverName, dataPort)

			print "Receiving directory structure from %s:%s" % (serverName, dataPort)

			contents = []

			data = dataSocket.recv(1024)
			while data:
				contents.append(dataSocket.recv(1024))
				data = dataSocket.recv(1024)

			for item in contents:
				print item


			'''
			while(dataSocket.recv(1024)):
				directory_contents = dataSocket.recv(1024)
				print "%s\n" % (directory_contents)
			'''

		elif command == '-g':
			dataSocket = init_contact(serverName, int(sys.argv[5]))
	else:
		print "Error sending command. Terminating program."
		sys.exit()


	#close control connection
	if clientSocket.close() == -1:
		print "Error closing control connection."
		sys.exit()

if __name__ == "__main__":
	main()

