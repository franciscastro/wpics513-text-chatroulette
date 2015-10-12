/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 11 Oct 2015

This is the TCR client process file.
*/

#include "tcr_client_header.h"

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


int main(/*int argc, char *argv[]*/)
{

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
	int exitsignal;		// If user wants to end the application (Command: EXIT, value: 8)

	// Main process loop for client
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
			// CONNECT
			case 1:	
					if (isconnected > 0) {
						printf("You are already connected to the TCR server: %s\n", s);
					}
					else {
						strncpy(hostname, "francisco-VirtualBox", 50);
						fprintf(stdout, "Client: Connecting...\n");
						isconnected = connectToHost(PORT, &hints, &servinfo, &error_status, hostname, &p);
						
						if (isconnected > 0) {
							// Convert a struct in_addr to numbers-and-dots notation (IP address) for printing
							inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
							fprintf(stdout, "Success! Connected to %s [%s]\n\n", hostname, s);
						}
					}
					break;
			// CHAT
			case 2: 
					if (isconnected > 0) {
						struct packet toSend;

						if(createPacket(command, &toSend) == -1) {
							fprintf(stderr, "Can't create data to send. Try again.\n");
						}
						else {
							int sent = sendDataToServer(&toSend);
						}

						memset(&toSend, 0, sizeof(struct packet));	// Empty the struct
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			// QUIT
			case 3: 
					if (isconnected > 0) {
						struct packet toSend;

						if(createPacket(command, &toSend) == -1) {
							fprintf(stderr, "Can't create data to send. Try again.\n");
						}
						else {
							int sent = sendDataToServer(&toSend);
						}

						memset(&toSend, 0, sizeof(struct packet));	// Empty the struct
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			// TRANSFER
			case 4: 
					if (isconnected > 0) {
						if (sendFilePackets(sockfd) == 0) {
							printf("File pending in server...\n\n");	// File is in server
						}
						else {
							printf("Failed to send file.\n\n");
						}
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			// FLAG
			case 5: 
					if (isconnected > 0) {
						struct packet toSend;

						if(createPacket(command, &toSend) == -1) {
							fprintf(stderr, "Can't create data to send. Try again.\n");
						}
						else {
							int sent = sendDataToServer(&toSend);
						}

						memset(&toSend, 0, sizeof(struct packet));	// Empty the struct
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			// HELP
			case 6: 
					if (isconnected > 0) {
						struct packet toSend;

						if(createPacket(command, &toSend) == -1) {
							fprintf(stderr, "Can't create data to send. Try again.\n");
						}
						else {
							int sent = sendDataToServer(&toSend);
						}

						memset(&toSend, 0, sizeof(struct packet));	// Empty the struct
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			// MESSAGE
			case 7: 
					if (isconnected > 0) {
						struct packet toSend;

						if(createPacket(command, &toSend) == -1) {
							fprintf(stderr, "Can't create data to send. Try again.\n");
						}
						else {
							int sent = sendDataToServer(&toSend);
						}

						memset(&toSend, 0, sizeof(struct packet));	// Empty the struct
					}
					else {
						printf("You are not connected to the TCR server. CONNECT first.\n\n");
					}
					break;
			// EXIT
			case 8: printf("Closing the chat client...\n"); break;
			default: printf("Invalid command. Enter HELP to get the list of valid commands.\n\n");
		}
		
		if (exitsignal == 8) {
			break;
		}

	}

	//=================================================================================

	// Close the connection on socket descriptor
	close(sockfd);
	
	return 0;

}
