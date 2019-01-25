# File Transfer Client/Server 

A simple file transfer program for a client/server model which makes use of a TCP connection to allow the user to download a file from the server.  

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

What things you need to install the software and how to install them

```
GNU Compiler (Recommend to install with package manager such as Homebrew)
Python3
```

### Installing (Bash Terminal)

In the command line within your desired directory, run the following command to download the program:

```
#Downloads the project directory to the current working directory
git clone https://github.com/venGaza/fileTransfer
```

Then run the following command to build the project:

```
#Build
make all
```

### Running the server

The client should be set up first with the following command (make sure to place on different server):

```
Compile: gcc -o server server.c
Run: 
		./server PORT
Example		./server 31343
```

The server will check the directory that it is installed in for the names of the files in the directories. It will also be able to transfer a text file that is located within this directory if the client requests it by name. 

The client should be setup second:

```
Run: 
		python ./client flip2 SERVER PORT -l
		python ./client flip2 SERVER PORT -g FILENAME

        ** the -l flag will list the contents of the server directory
        ** the -g flag will transfer the FILENAME if available from server to client
        ** flip2 is the name of the server the client is targetting and is currently hardcoded

Example: 	python ./client flip2 31343 31342 -g test.txt
```

## Built With

* [Visual Studio Code](https://code.visualstudio.com/) - Visual Studio Code is a code editor redefined and optimized for building and debugging modern web and cloud applications.
* [Vim](https://www.vim.org/) - Vim is a highly configurable text editor for efficiently creating and changing any kind of text.

## Authors

* **Dustin Jones** - [venGaza](https://github.com/venGaza)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* [Beej's Guide to Socket Programming](https://beej.us/guide/bgnet/html/multi/index.html) 
* Oregon State University