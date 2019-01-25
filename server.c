/*
 * Description: This program creates a FTP server
 * **SOURCE: Much of the code is based from Beej's guide to Socket Programming at https://beej.us/guide/bgnet/html/multi/clientserver.html**
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>

/*Function Prototypes*/
void error(const char* message);
int createSocket(int portNum);
void readCommand(int establishedConnectionFD, int* dataPort, char* dataCommand, char* dataFile, char* buffer);
void sendDirectory(char* buffer, int dataSocketFD);
void sendReqFile(char* buffer, char* dataFile, int dataSocketFD);

/*Main Function*/
int main(int argc, const char * argv[]) {
    int listenSocketFD, listenDataSocketFD, establishedConnectionFD, dataSocketFD, portNumber;
    int dataPort = 3333;
    socklen_t sizeOfClientInfo, sizeOfClientInfo2;
    char buffer[10000];
    memset(buffer, '\0', 10000);
    char dataCommand[1024];
    memset(dataCommand, '\0', 1024);
    char dataFile[1024];
    memset(dataFile, '\0', 1024);
    struct sockaddr_in clientAddress, clientAddress2;
    
    //Check if port number was given as an argument
    if (argc < 2) {
        fprintf(stderr,"Usage: %s port\n", argv[0]);
        exit(1);
    } else {
        portNumber = atoi(argv[1]);  //Get port number from command argument
    }
    
    listenSocketFD = createSocket(portNumber); //Create listening socket
    
    //Keep server in infinite accept loop
    while (1) {
        //Accept connection (block while waiting for one to connect)
        sizeOfClientInfo = sizeof(clientAddress); //Gets client address size
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr*)&clientAddress, &sizeOfClientInfo); //Accepts connection
        if (establishedConnectionFD < 0) {
            error("Hull breach: accept()");
        }
        printf("Server: Connected to client at port %d\n", ntohs(clientAddress.sin_port));
        
        int spawnPID = fork(); //Spawn new process for data handling
        switch(spawnPID) {
            case -1:                                //Error
                error("Hull breach: fork()");
                exit(1);
                
            case 0:                                 //Child handling data request
                readCommand(establishedConnectionFD, &dataPort, dataCommand, dataFile, buffer);
                listenDataSocketFD = createSocket(dataPort);
                dataSocketFD = accept(listenDataSocketFD, (struct sockaddr*)&clientAddress2, &sizeOfClientInfo2);
                
                //Execute specified command or return error if uknown
                if ((strstr(dataCommand, "-l")) != NULL) {              //Send Directory
                    sendDirectory(buffer, dataSocketFD);
                } else if ((strstr(dataCommand, "-g")) != NULL) {       //Send File
                    printf("Sending file: %s\n", dataFile);
                    sendReqFile(buffer, dataFile, dataSocketFD);
                } else {                                                //Error
                    printf("Data Command is: %s\n", dataCommand);
                    fflush(stdout);
                    error("Error reading command\n");
                }
                
                close(dataSocketFD);
                break;
                
            default:;                                //Parent
                    //Originally planned to have parent intialize values but realized it has sep copy
        }
       
        
        close(establishedConnectionFD); //Close the socket
    }
    
    close(listenSocketFD); //Close the listening socket
    
    printf("Hello, World!\n");
    return 0;
}

/*
 * Function: error
 * Usage: error(message)
 *  -------------------------
 *  This function prints takes a string parameter which contains a message and outputs
 *  it as an error. It then exits the program with a status code of 1.
 *  SOURCE: Code based off CS344-400 lecture material 4.3
 */
void error(const char* message) {
    perror(message);
    exit(1);
}

/*
 * Function: createSocket
 * Usage: createSocket(12345)
 *  -------------------------
 *  This function creates a listening socket on the server and returns this socket
 *  back to the calling function. It takes one parameter which is the desired port
 *  number.
 */
int createSocket(int portNum) {
    int listenSocketFD;
    struct sockaddr_in serverAddress;
    
    //Fill in address struct for the server
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //Initialize address struct
    serverAddress.sin_family = AF_INET; //Set type to TCP
    serverAddress.sin_port = htons(portNum); //Convert portNum to network byte order
    serverAddress.sin_addr.s_addr = INADDR_ANY; //Accept from any IP address
    
    //Setup the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocketFD < 0) {
        error("Hull Breach: socket()");
    }
    
    //Bind the socket
    if (bind(listenSocketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("Hull breach: bind()");
    }
    listen(listenSocketFD, 5); //Socket turned on and listens for up to 5 connections
    
    printf("Server socket open on port %d\n", portNum);    
    return listenSocketFD;
}

/*
 * Function: readCommand
 * Usage: readCommand(socket, dataPort, command, buffer)
 *  -------------------------
 *  This function reads in a command line from a client. The format of the request
 *  must be passed in the following format "portNumber , command." This function
 *  makes use of the tokenizer to parse the command and determine the port number
 *  to send data back on as well as what data needs to be sent.
 */
void readCommand(int establishedConnectionFD, int* dataPort, char* dataCommand, char* dataFile, char* buffer) {
    int charRead;
    memset(buffer, '\0', 10000);
    char* token;
    
    //Read in command from client
    charRead = recv(establishedConnectionFD, buffer, 9999, 0);
    if (charRead < 0) {
        error("Error: read()");
    }
    
    //Extract port number
    token = strtok(buffer, ",");
    *dataPort = atoi(token);
    printf("Here is dataPort: %d\n", *dataPort);
    fflush(stdout);
    
    //Extract command
    token = strtok(NULL, ",");
    strcpy(dataCommand, token);
    printf("Here is data command: %s\n", dataCommand);
    fflush(stdout);
    
    //Extract file name if the file transfer command given
    if (strcmp(dataCommand, "-g") == 0) {
        token = strtok(NULL, ",");
        strcpy(dataFile, token);
    }
}

/*
 * Function: sendDirectory
 * Usage: sendDirectory(buffer, dataSocketFD)
 *  -------------------------
 *  This function reads the contents of the current directory and send them over
 *  to the client.
 */
void sendDirectory(char* buffer, int dataSocketFD) {
    int charSend;
    memset(buffer, '\0', sizeof(buffer));
    
    //Open current directory
    DIR* pwd;
    pwd = opendir(".");
    
    //Read directory into buffer
    struct dirent *dir;
    while ((dir = readdir(pwd))) {
        if (dir->d_name[0] != '.') {                //Ignore hidden files
            strcat(buffer, "\n");                   //Spacer
            strcat(buffer, dir->d_name);            //Add directory name
        }
    }
    
    //Send directory to client
    charSend = send(dataSocketFD, buffer, strlen(buffer), 0);
    if(charSend < 0) {
        error("Hull breach: error in sending directory");
    }
}

/*
 * Function: sendReqFile
 * Usage: sendReqFile(buffer, dataSocketFD)
 *  -------------------------
 *  This function reads the contents specified file and returns it to the user.
 */
void sendReqFile(char* buffer, char* dataFile, int dataSocketFD) {
    int charSend;
    char name[100];
    memset(name, '\0', 100);
	
    //Create target file name
    sprintf(name,"./%s", dataFile);
    
    //Open the file
    FILE* file;
    file = fopen(name, "r");
    if (file == NULL) {                 //Error occurred trying to open file
        printf("Could not open file");
        strcpy(buffer, "ERROR: Could not send file\n");
        charSend = send(dataSocketFD, buffer, strlen(buffer), 0);
    }
    
    if (fgets(buffer, 9999, file) == NULL) { //Error reading file
        error("Error reading file");
    }
    fclose(file);
   
    //Send file to client
    charSend = send(dataSocketFD, buffer, strlen(buffer), 0);
    if(charSend < 0) {
        error("Hull breach: error in sending file");
    }
    
    //Output completion message
    printf("Sending file complete\n");
    fflush(stdout);
}


