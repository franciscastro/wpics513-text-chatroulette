/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 01 Oct 2015

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
- CONNECT
- CHAT
- QUIT
- TRANSFER
- FLAG
- HELP
*/

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) 
    {
        n = send(s, buf+total, bytesleft, 0);
        
        if (n == -1) { break; }
        
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}

// Get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
	// sockaddr is IPv4
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	// else, sockaddr is IPv6
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
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

	// Space to hold the IPv6 string
	char s[INET6_ADDRSTRLEN];

	// Buffer to read the information into
	char buf[MAXDATASIZE];


	// Trigger error if client was incorrectly run
	if (argc != 2) 
	{
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}


	// [ Load up address structs with getaddrinfo() ]
	//=================================================================================

	// Setup values in hints
	memset(&hints, 0, sizeof hints);	// make sure the struct is empty
	hints.ai_family = AF_UNSPEC;		// don't care if IPv4 (AF_INET) or IPv6 (AF_INET6)
	hints.ai_socktype = SOCK_STREAM;	// use TCP stream sockets
	
	// Call getaddrinfo() to setup the structures in hints
	// - error: getaddrinfo() returns non-zero
	// - success: *servinfo will point to a linked list of struct addrinfo,
	//			  each of which contains a struct sockaddr to be used later
	if ((error_status = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error_status));
		return 1;
	}

	//=================================================================================


	// [ Make a socket, connect() to destination ]
	//=================================================================================

	// Loop through all the results in *servinfo and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		// Make a socket
		// - assign a socket descriptor to sockfd on success, -1 on error
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("client: socket");
			continue;
		}

		// Connect to a remote host in the destination port and IP address
		// - returns -1 on error and sets errno to the error's value
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	// Free the linked list when all done with *servinfo
	freeaddrinfo(servinfo);

	// If *servinfo is empty, then fail to connect
	if (p == NULL) 
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	//=================================================================================


	// Convert a struct in_addr to numbers-and-dots notation (IP address) for printing
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("Client: connecting to %s\n", s);


	// [ Send data (?) ]
	//=================================================================================

	printf("sending to %i\n", sockfd);
	char msg[5] = "hello";
	printf("msg is %s\n", msg);
	int sent = send(sockfd, msg, strlen(msg) + 1, 0);

	printf("data sent? %i\n", sent != -1);

	// sendall(sockfd, msg, sizeof msg);

	//=================================================================================


	// Receive data: recv() returns the number of bytes actually read into the buffer, or -1 on error
	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) 
	{
		perror("recv");
		exit(1);
	}

	// Output the data received
	buf[numbytes] = '\0';					// Terminate string
	printf("Client: received '%s'\n",buf);	// Print data received
	
	// Close the connection on socket descriptor
	close(sockfd);
	
	return 0;

}
