/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 10 Oct 2015

This is the TCR client header file.
*/

#define MAXMESSAGESIZE 1024
#define MAXCOMMANDSIZE 20

struct packet {
	char command[MAXCOMMANDSIZE];
	char message[MAXMESSAGESIZE];
};

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
//			 1 on successful connect
int connectToHost(char *PORT, struct addrinfo *hints, struct addrinfo **servinfo, int *error_status, char *hostname, struct addrinfo **p, int *sockfd) {

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
		if (((*sockfd) = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) 
		{
			perror("Client socket");
			continue;
		}

		// Connect to a remote host in the destination port and IP address
		// - returns -1 on error and sets errno to the error's value
		if (connect((*sockfd), (*p)->ai_addr, (*p)->ai_addrlen) == -1) 
		{
			close((*sockfd));
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

	return 1;

	//=================================================================================

}

// Send a message to TCR server
int sendDataToServer(struct packet *packet, int sockfd) {

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

int sendFilePackets(int sockfd) {

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

	// Outbound data packet
	struct packet outbound;
	strncpy(outbound.command, "TRANSFER", MAXCOMMANDSIZE);

	// Read and send file packets
	while(!feof(fp)){
		fread(filebuff, 1, MAXMESSAGESIZE, fp);
		strncpy(outbound.message, filebuff, MAXMESSAGESIZE);
		sendDataToServer(&outbound, sockfd);
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
	else if (strcmp(command, "TRANSFER") == 0)
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
		fprintf(stdout, "Message: ");
		fgets(outbound.message, sizeof outbound.message, stdin);
		fprintf(stdout, "Your message: %s", outbound.message);
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