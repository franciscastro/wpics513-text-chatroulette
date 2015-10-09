/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 09 Oct 2015

This is the client process file.
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

#define PORT "3490" // the port client will be connecting to
#define MAXDATASIZE 1000 // max number of bytes we can get at once

/*
TO DO:
- CONNECT [DONE]
- CHAT
- QUIT
- TRANSFER
- FLAG
- HELP
- EXIT [DONE]
- MESSAGE
*/

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
	else if (strcmp(command, "EXIT") == 0)
	{
		return 7;
	}
	else
	{
		return -1;
	}
}

// Connect to TCR server
int connectToHost(struct addrinfo *hints, struct addrinfo **servinfo, int *error_status, char *hostname, struct addrinfo **p, int *sockfd) {

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
		return 1;
	}

	//=================================================================================


	// [ Make a socket, connect() to destination ]
	//=================================================================================

	// Loop through all the results in *servinfo and bind to the first we can
	for((*p) = (*servinfo); (*p) != NULL; (*p) = (*p)->ai_next) 
	{
		// Make a socket
		// - assign a socket descriptor to sockfd on success, -1 on error
		if (((*sockfd) = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) 
		{
			perror("client: socket");
			continue;
		}

		// Connect to a remote host in the destination port and IP address
		// - returns -1 on error and sets errno to the error's value
		if (connect((*sockfd), (*p)->ai_addr, (*p)->ai_addrlen) == -1) 
		{
			close((*sockfd));
			perror("client: connect");
			continue;
		}

		break;
	}

	// Free the linked list when all done with *servinfo
	freeaddrinfo(*servinfo);

	// If *servinfo is empty, then fail to connect
	if ((*p) == NULL) 
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	//=================================================================================

}

// Send a message to TCR server
int sendAllDataToHost(char *msg, int *msglen, int sockfd) {

	int total = 0;				// How many bytes we've sent
    int bytesleft = *msglen;	// How many we have left to send
    int n;

    while(total < *msglen) {

        n = send(sockfd, (msg + total), bytesleft, 0);
        
        if (n == -1) { break; }
        
        total += n;
        bytesleft -= n;

    }

    *msglen = total;	// Return number actually sent here

	return n == -1 ? -1 : 0;	// Return 0 on success, -1 on failure
}

// Convert strings to all uppercase
void allCaps(char *command) {
	while(*command != '\0') {
		*command = toupper(*command);
		command++;
	}
}

int main(/*int argc, char *argv[]*/)
{
	// Socket file descriptor for communicating to remote host
	int sockfd;

	// Number of bytes received from the recv() call
	int numbytes;

	// For prepping sockaddrs later: 
	// - hints points to an addrinfo to be filled with information
	// - *servinfo points to a linked list of struct addrinfo
	// - *p serves as a temporary pointer to hold *servinfo's data later
	struct addrinfo hints, *servinfo, *p;
	
	// Will hold the error state when getaddrinfo() is called
	int error_status;

	// Will hold the server's hostname
	char hostname[50];

	// Space to hold the IPv6 string
	char s[INET6_ADDRSTRLEN];

	// Buffer to read the information into
	char buf[MAXDATASIZE];

	// Buffer to send a message out
	char msg[MAXDATASIZE];

	// 1 if already connected to server, 0 otherwise
	int serverconnect = 0;


	// Receive data: recv() returns the number of bytes actually read into the buffer, or -1 on error
	/*
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) 
	{
		perror("recv");
		exit(1);
	}

	// Output the data received
	buf[numbytes] = '\0';					// Terminate string
	printf("Client: received '%s'\n",buf);	// Print data received
	*/
	printf("\nText ChatRoulette chat client started.\n\n");

	//=================================================================================

	char command[50];	// For receiving commands from user
	int exitsignal;		// If user wants to end the application (Command: EXIT, value: 7)

	// Main process loop for client
	printf("Command: "); // Prompt command from the user
	while(fgets(command, sizeof command, stdin)) 
	{
		// Manual removal of newline character
		int len = strlen(command);
		if (len > 0 && command[len-1] == '\n') {
			command[len-1] = '\0';
		}
		allCaps(command);	// Convert command to uppercase for consistency


		// Select appropriate action based on command entered
		switch(exitsignal = commandTranslate(command))
		{

			case 1:	
					if (serverconnect) {
						printf("You are already connected to the TCR server: %s\n\n", s);
					}
					else {
						strncpy(hostname, "francisco-VirtualBox", 50);
						printf("Client: Connecting...\t");
						connectToHost(&hints, &servinfo, &error_status, hostname, &p, &sockfd);
						
						// Convert a struct in_addr to numbers-and-dots notation (IP address) for printing
						inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
						printf("Success! Connected to %s [%s]\n\n", hostname, s);
						serverconnect = 1;
					}
					break;
			case 2: 
					if (serverconnect) {
						printf("Entered CHAT.\n\n"); 
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			case 3: 
					if (serverconnect) {
						printf("Entered QUIT.\n\n"); 
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			case 4: 
					if (serverconnect) {
						printf("Entered TRANSFER.\n\n"); 
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			case 5: 
					if (serverconnect) {
						printf("Entered FLAG.\n\n"); 
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			case 6: 
					if (serverconnect) {
						printf("Entered HELP.\n\n"); 
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			case 7: printf("Closing the chat client...\n\n"); break;
			default: printf("Invalid command. Enter HELP to get the list of valid commands.\n\n");
		}
		
		if (exitsignal == 7) {
			break;
		}

		printf("Command: ");	// Prompt command from the user
	}

	//=================================================================================

	// Close the connection on socket descriptor
	close(sockfd);
	
	return 0;

}
