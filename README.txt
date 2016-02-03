File:		README.txt
Author:     Abby Meunier
Email:      meuniera@oregonstate.edu
Date:       11/29/2015


****************
* ftserver.cpp *
****************

To compile:	g++ ftserver.cpp -o serv
To execute:	./serv <SERVER_PORT>


***************
* ftclient.py *
***************

To execute:
		To list server directory:	    python ftclient.py <SERVER_HOST> <SERVER_PORT> -l <DATA_PORT>
		To receive file from server:	python ftclient.py <SERVER_HOST> <SERVER_PORT> -g <FILE_NAME> <DATA_PORT>


****************
* Instructions *
****************

-compile and execute ftserver.cpp on Host A, specifying a valid port #.
-execute ftclient.py on Host B, specifying the server host and port #, the name of the file you wish to receive
 (if using the '-g' command), and a valid port # on which to receive the data.
-ftclient.py will terminate after receiving and listing the directory, receiving and saving the requested file,
 or encountering an error.
-to terminate ftserver.cpp, enter a SIGINT such as control+c.


