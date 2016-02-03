'''
Program:	    ftclient.py
Author:		    Abby Meunier
Email:		    meuniera@oregonstate.edu
Date:		    11/29/2015
Resources used:     http://www.bogotobogo.com/python/python_network_programming_server_client_file_transfer.php
		    https://docs.python.org/2/tutorial/inputoutput.html
		    http://stackoverflow.com/questions/120656/directory-listing-in-python
'''

from socket import *
import sys
import os

def init_contact(server_name, server_port):
    #set up socket using validated arguments
    client_socket = socket(AF_INET, SOCK_STREAM)

    #connect to server
    if client_socket.connect((server_name, server_port)) == -1:
        print "Error connecting to", server_name, "port #", server_port
        sys.exit()

    return client_socket

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

def make_request(command, client_socket):
    #client quits or sends message
    if command == "/quit":
        print "Closing connection."
        client_socket.close()
        print "Connection closed. Terminating program."
        exit()
    else:
        client_socket.send(command)

def receive_msg(client_socket, server_name, server_port):
    #receive and print message, or end program
    response = client_socket.recv(1024)
    if len(response) == 0 or response == "SUCCESS":
        return "SUCCESS"
    else:
        print "%s:%d says: %s" % (server_name, server_port, response)
        return response

def receive_file(data_socket, file_name):
    #check if duplicate fileName
    for dirname, dirnames, filenames in os.walk('.'):
        for cur_file in filenames:
            if cur_file == file_name:
                print "File '%s' already exists. Received file will be saved as 'dup_%s'" % (file_name, file_name)
                file_name = "dup_" + file_name

    #receive file data and write to new file
    file = open(file_name, 'a')
    data = data_socket.recv(1024)
    while data:
        with open(file_name, 'a') as f:
            f.write(data)
        data = data_socket.recv(1024)
    print "File transfer complete"

def list_directory(data_socket):
    contents = []
    data = data_socket.recv(500)

    while data:
        data = data_socket.recv(500)
        print data	

def main():
    #validate command line arguments
    validate() 

    server_name = sys.argv[1]
    server_port = int(sys.argv[2])
    command = sys.argv[3]

    #request control connection to server
    client_socket = init_contact(server_name, server_port)

    #send command and receive response to control connection
    make_request(str(sys.argv[1:]), client_socket)
    control_response = receive_msg(client_socket, server_name, server_port)

    if control_response == "SUCCESS":
        if command == '-l':
            data_port = int(sys.argv[4])
            data_socket = init_contact(server_name, data_port)
                
            print "Receiving directory structure from %s:%s" % (server_name, data_port)
            list_directory(data_socket)
                
        elif command == '-g':
            data_port = int(sys.argv[5])
            file_name = sys.argv[4]
            data_socket = init_contact(server_name, data_port)
            control_response = receive_msg(client_socket, server_name, server_port)

            if control_response == "SUCCESS":
                print "Receiving '%s' from %s:%s" % (file_name, server_name, data_port)
                receive_file(data_socket, file_name)

    else:
        print "Error sending command. Terminating program."
        sys.exit()

    #close control connection
    if client_socket.close() == -1:
        print "Error closing control connection."
        sys.exit()

if __name__ == "__main__":
    main()
