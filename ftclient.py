#!/bin/python

# Description: File transfer program that demonstrates an FTP connection between
# two hosts.
# REFERENCES:
#   https://docs.python.org/2/library/socket.html
#   https://www.geeksforgeeks.org/socket-programming-python/
#   https://www.tutorialspoint.com/python/python_networking.htm

from socket import *
import sys
import time

# Function: err
# Description: This function prints out a specified error message and exits with provided status
def err(message, exitValue):
	print(message)
	sys.exit(exitValue)

# Function: validateCommand
# Description: This function validates the command given to the ft client. The commands are
#   available in two formats:
# python ftclient flipX SERVER PORT -l (Gets file directory from server)
# python ftclient flipX SERVER PORT -g file.txt (Downloads file from server)
# **Will assume user has put client and server on different flip servers**
def validateCommand():
	if len(sys.argv) > 6 or len(sys.argv) < 5:
	        err("Invalid number of command arguments", 1)
	
	if sys.argv[1] != "flip1" and sys.argv[1] != "flip2" and sys.argv[1] != "flip3":
        	err("Invalid server name", 1)

	if int(sys.argv[2]) < 1024 or int(sys.argv[2]) > 65535:
		err("Invalid server port number", 1)

	if int(sys.argv[3]) < 1024 or int(sys.argv[3]) > 65535:
        	err("Invalid client port number", 1)

	if sys.argv[4] != "-l" and sys.argv[4] != "-g":
        	err("Invalid command flag", 1)

	if sys.argv[4] == "-g":
		if sys.argv[5] == "":
			err("File name was not input", 1)

# Function: createSocket
# Description: This function attempts to connect the client to the server with the designated
#   server name and port number. If a connection cannot be made, an error message is printed out.
#   If the connection is established, the socket is returned to the calling function.
def createSocket(server, port):
	serverAdd = server + ".engr.oregonstate.edu"
	userSocket = socket(AF_INET, SOCK_STREAM)

	try:
		userSocket.connect((serverAdd, port))

	except:
		err("Error connecting to port %d" % port, 1)

	print("Connected to port {} ").format(port)

	return userSocket

# Function: sendCommand
# Description: This function sends the command from the user to the server over the specified
#   socket. If the message cannot be sent, it will print out an error message.
def sendCommand(socket, commandMessage):
	try:
		socket.send(commandMessage)
	except:
		err("Error sending server command", 1)

#####MAIN FUNCTION######
if __name__ == "__main__":
	validateCommand()  #Validate the command input

	serverName = sys.argv[1]    #Extract command line arguments
	serverPort = int(sys.argv[2])
	clientPort = int(sys.argv[3])
	command = sys.argv[4]
	if command == "-g":
		fileName = sys.argv[5]
	else:
		fileName = ""

	commandSocket = createSocket(serverName, serverPort)     #Set up command connection with server
	
	#Send command to ftServer
	if command == "-l":
		serverCommand = sys.argv[3] + "," + command 
		sendCommand(commandSocket, serverCommand)

		#Setup socket for response
		time.sleep(3) #Wait for server to read command, spawn child process, and launch data socket
		dataSocket = createSocket(serverName, clientPort) #Wait for server response on data scoket

		#Output directory
		directory = dataSocket.recv(10000)
		print(directory)

	else:
		serverCommand = sys.argv[3] + "," + command + "," + sys.argv[5]
		sendCommand(commandSocket, serverCommand)

		#Setup socket for response
		time.sleep(3) #Wait for server to read command, spawn child process, and launch data socket
		dataSocket = createSocket(serverName, clientPort) #Wait for server response on data scoket	

		#Write incoming file request from server
		fileData = dataSocket.recv(10000)
		fd = open(fileName, "w+") #Create new file in pwd 
		fd.write(fileData) #Write data to new file
		fd.close 
		commandSocket.close() #Close out the sockets

	dataSocket.close()
