/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 11 Oct 2015

This is the TCR client header file.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>


#define MAXMESSAGESIZE 1024
#define MAXCOMMANDSIZE 20

// Socket file descriptor for communicating to remote host
int sockfd;

// 1 if already connected to server, 
// 0, -1, -2, otherwise (see connectToHost())
int isconnected = 0;


struct packet {
	char command[MAXCOMMANDSIZE];	// Command triggered
	char message[MAXMESSAGESIZE];	// Data
};

struct threaddata {
	pthread_t thread_ID;	// This thread's pointer
};

void *get_in_addr(struct sockaddr *sa);
int commandTranslate(char *command);
int connectToHost(char *PORT, struct addrinfo *hints, struct addrinfo **servinfo, int *error_status, char *hostname, struct addrinfo **p);
void *receiver(void *param);
int sendDataToServer(struct packet *packet);
int sendFilePackets();
struct packet createPacket(char *command);
void allCaps(char *command);

//=================================================================================


// Get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa) {

	// sockaddr is IPv4
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	// else, sockaddr is IPv6
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Translates user's command into this program's integer representation
int commandTranslate(char *command) {

	if (strcmp(command,"CONNECT") == 0)
	{
		return 1;
	}
	else if (strcmp(command, "CHAT") == 0)
	{
		return 2;
	}
	else if (strcmp(command,"QUIT") == 0)
	{
		return 3;
	}
	else if (strcmp(command, "TRANSFER") == 0)
	{
		return 4;
	}
	else if (strcmp(command, "FLAG") == 0)
	{
		return 5;
	}
	else if (strcmp(command, "HELP") == 0)
	{
		return 6;
	}
	else if (strcmp(command, "MESSAGE") == 0)
	{
		return 7;
	}
	else if (strcmp(command, "EXIT") == 0)
	{
		return 8;
	}
	else
	{
		return -1;
	}
}

// Connect to TCR server
// Returns: -1 on getaddrinfo() fail
//			-2 on fail to connect a socket to remote host
//			-3 on fail to create thread for recv()ing data
//			 1 on successful connect
int connectToHost(char *PORT, struct addrinfo *hints, struct addrinfo **servinfo, int *error_status, char *hostname, struct addrinfo **p) {

	// [ Load up address structs with getaddrinfo() ]
	//=================================================================================

	// Setup values in hints
	memset(hints, 0, sizeof *hints);	// make sure the struct is empty
	(*hints).ai_family = AF_UNSPEC;		// don't care if IPv4 (AF_INET) or IPv6 (AF_INET6)
	(*hints).ai_socktype = SOCK_STREAM;	// use TCP stream sockets
	
	// Call getaddrinfo() to setup the structures in hints
	// - error: getaddrinfo() returns non-zero
	// - success: *servinfo will point to a linked list of struct addrinfo,
	//			  each of which contains a struct sockaddr to be used later
	if (((*error_status) = getaddrinfo(hostname, PORT, hints, &(*servinfo))) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror((*error_status)));
		return -1;
	}

	//=================================================================================


	// [ Make a socket, connect() to destination ]
	//=================================================================================

	// Loop through all the results in *servinfo and bind to the first we can
	for((*p) = (*servinfo); (*p) != NULL; (*p) = (*p)->ai_next) 
	{
		// Make a socket
		// - assign a socket descriptor to sockfd on success, -1 on error
		if ((sockfd = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) 
		{
			perror("Client socket");
			continue;
		}

		// Connect to a remote host in the destination port and IP address
		// - returns -1 on error and sets errno to the error's value
		if (connect(sockfd, (*p)->ai_addr, (*p)->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("Client connect");
			continue;
		}

		break;
	}

	// Free the linked list when all done with *servinfo
	freeaddrinfo(*servinfo);

	// If *servinfo is empty, then fail to connect
	if ((*p) == NULL) 
	{
		fprintf(stderr, "Client: Failed to connect\n");
		return -2;
	}

	// Create pthread for recv()ing data from the socket
	struct threaddata newthread;
	if(pthread_create(&newthread.thread_ID, NULL, receiver, (void *)&newthread)) 
	{
		fprintf(stderr, "Error creating recv() thread. Try connecting again.\n");
		close(sockfd);
		return -3;
	}

	// Prompt client that it is now waiting to recv() on pthread
	fprintf(stdout, "Now waiting for data on [%i]...\n", sockfd);

	// Set isconnected flag since client successfully connected to server
	isconnected = 1;

	return 1;

	//=================================================================================

}

// For recv()ing messages from the socket
void *receiver(void *param) {
	
	int recvd;
	struct packet msgrecvd;

	// While connection with server is alive
	while(isconnected) {

		// recv() data from the socket
		recvd = recv(sockfd, (void *)&msgrecvd, sizeof(struct packet), 0);
		
		if (!recvd) {
			fprintf(stderr, "Server connection lost. \n");
			isconnected = 0;
			close(sockfd);
			break;
		}

		if (recvd > 0) {
			fprintf(stdout, "%s\n", msgrecvd.message);
		}

		// Make sure the struct is empty
		memset(&msgrecvd, 0, sizeof(struct packet));

	}

}

// Handling recv()ed messages from the socket
void receivedDataHandler(struct packet *msgrecvd) {

	if (strcmp((*msgrecvd).command, "ACKN") == 0) {
		fprintf(stdout, "You are added in the chat queue\n");
	}
	else if (strcmp((*msgrecvd).command, "IN SESSION") == 0) {
		fprintf(stdout, "Now on a channel with: %s\n", (*msgrecvd).message);
	}
	else if (strcmp((*msgrecvd).command, "QUIT") == 0) {
		fprintf(stdout, "You have quit the channel \n");
	}
	else if (strcmp((*msgrecvd).command, "HELP") == 0) {
		fprintf(stdout, "Commands available:\n %s\n", (*msgrecvd).message );
	}
	else if (strcmp((*msgrecvd).command, "MESSAGE") == 0) {
		fprintf(stdout, "[ ]: %s\n", (*msgrecvd).message );
	}

}

// Send a message to TCR server
int sendDataToServer(struct packet *packet) {

	int packetlen = sizeof *packet;
	int total = 0;				// How many bytes we've sent
    int bytesleft = packetlen;	// How many we have left to send
    int n;

    // To make sure all data is sent
    while(total < packetlen) {

        n = send(sockfd, (packet + total), bytesleft, 0);
        
        if (n == -1) { break; }
        
        total += n;
        bytesleft -= n;

    }

    packetlen = total;	// Return number actually sent here

	return n == -1 ? -1 : 0;	// Return 0 on success, -1 on failure
}

int sendFilePackets() {

	// Get file name from user
	char filename[50];
	fprintf(stdout, "File to send: ");
	fgets(filename, sizeof filename, stdin);
	
	// Manual removal of newline character
	int len = strlen(filename);
	if (len > 0 && filename[len-1] == '\n') {
		filename[len-1] = '\0';
	}
	
	// Create file pointer
	FILE *fp = fopen(filename, "rb");
	
	// If file does not exist
	if (fp == NULL) {
		fprintf(stdout, "File open error. Check your file name.\n");
		return 1;
	}

	// file buffer to store chunks of files
	char filebuff[MAXMESSAGESIZE];

	// Read and send file packets
	while(!feof(fp)){

		// Outbound data packet
		struct packet outbound;
		strncpy(outbound.command, "TRANSFER", MAXCOMMANDSIZE);

		// Read data from file
		fread(outbound.message, 1, MAXMESSAGESIZE, fp);
		
		// Send data to server
		sendDataToServer(&outbound);

		memset(&outbound, 0, sizeof(struct packet));	// Empty the struct
	}

	fclose(fp);

	return 0;

}

struct packet createPacket(char *command) {

	struct packet outbound;

	strncpy(outbound.command, command, MAXCOMMANDSIZE);

	if (strcmp(command,"CONNECT") == 0)
	{
		return outbound;
	}
	else if (strcmp(command, "CHAT") == 0)
	{
		return outbound;
	}
	else if (strcmp(command,"QUIT") == 0)
	{
		return outbound;
	}
	else if (strcmp(command, "FLAG") == 0)
	{
		return outbound;
	}
	else if (strcmp(command, "HELP") == 0)
	{
		return outbound;
	}
	else if (strcmp(command, "MESSAGE") == 0)
	{
		fprintf(stdout, "[ You ]: ");
		fgets(outbound.message, sizeof outbound.message, stdin);

		// Manual removal of newline character
		int len = strlen(outbound.message);
		if (len > 0 && outbound.message[len-1] == '\n') {
			outbound.message[len-1] = '\0';
		}

		//fprintf(stdout, "Your message: %s", outbound.message);
		return outbound;
	}

}

// Convert strings to all uppercase
void allCaps(char *command) {
	while(*command != '\0') {
		*command = toupper(*command);
		command++;
	}
}