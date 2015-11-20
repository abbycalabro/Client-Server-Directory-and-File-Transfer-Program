'''
Program:		chatclient.py
Assignment:     CS 372, Project 1
Author:			Abby Meunier
Email:			meuniera@oregonstate.edu
Date:			11/1/2015
Description:	Program takes two command line arguments: server hostname and port #.
				Prompts user for a username handle and establishes a TCP connection with server.
				Once connected, client and server exchange messages until either one enters 
				the message "/quit", upon which the connection is closed and the program terminates.
'''

from socket import *
import sys

def get_handle():
	handle = raw_input( "Enter a username up to 10 characters: ")
	while len(handle) < 1 or len(handle) > 10:
		handle = raw_input("Username must be between 1 and 10 characters. Try again: ");
	
	return handle

def init_contact():
	#set up socket using command line arguments
	serverName = sys.argv[1]
	serverPort = int(sys.argv[2])
	clientSocket = socket(AF_INET, SOCK_STREAM)

	#connect to server
	clientSocket.connect((serverName, serverPort))
	print "Client now connected to", serverName, "port #", serverPort
	
	return clientSocket
	
def send_msg(msg, clientSocket, handle):
	#client quits or sends message
	if msg == "/quit":
		print "Closing connection."
		clientSocket.close()
		print "Connection closed. Terminating program."
		exit()
	else:
		clientSocket.send(handle + " > " + msg)

def rec_msg(clientSocket):
	#receive and print message, or end program
	response = clientSocket.recv(500)
	if len(response) == 0:
		print "Server has quit. Closing connection."
		clientSocket.close()
		print "Connection closed. Now terminating."
		exit()
	else:
		print "A >", response

def main():
	#get handle and establish connection to server
	handle = get_handle()
	clientSocket = init_contact()

	#exchange messages until client or server quits	
	while(1):
		#send message
		msg = raw_input(handle + " > ")
		while len(msg) < 1 or len(msg) > 500:
			print "Messages must be between 1 and 500 characters. Try again.\n"
			msg = raw_input(handle + " > ")
		send_msg(msg, clientSocket, handle)
		
		#receive message
		rec_msg(clientSocket)

if __name__ == "__main__":
	main()

