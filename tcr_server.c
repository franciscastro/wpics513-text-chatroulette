/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 28 Sept 2015

This is the server process file.
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

#define BACKLOG 10
#define PORT "3490" // the port client will be connecting to
#define STDIN 0

/*
TO DO:
- ACKN
- IN_SESSION
*/

int shutdown_socket(int sockfd, int flag, int free_socket)
{
	// for freeing socket descripter
	if (free_socket)
	{
		close(sockfd);
		return 1;
	}

	// change socket usability with shutdown
	int success = shutdown(sockfd, flag);
	if (success == 0)
		return 1;
	return 0;
}

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, save and restore it
	int saved_errno = errno;
	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
	// IPv4
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void read_bytes(int socket, unsigned int len, void* buffer)
{
    int bytesRead = 0;
    int result;
    while (bytesRead < len)
    {
    	printf("read %i bytes\n", bytesRead);
        result = read(socket, buffer + bytesRead, len - bytesRead);
        if (result < 1 )
        {
            perror("read_bytes");
        }
        bytesRead += result;
    }
}

int main() 
{
	fd_set master; // master file descriptor
	/*
		select() changes the set passed into it to reflec 
		which sockets are ready to read

		store connections somewhere -> copy master into the
		read_fds (read file descriptor) before calling select()
	*/

	fd_set read_fds; // temp file descriptor list for select()
	int fdmax; // maximum file descriptor number

	int listener; // listening socket descriptor
	int newfd; // newly accepted (via accept()) socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;

	char buf[256]; // buffer for client data
	int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

	int yes=1; // for setsockopt() SO_REUSEADDR, below
	int i, j, rv;
	
	struct addrinfo hints, *ai, *p;

	FD_ZERO(&master); // clear the master and temp sets
	FD_ZERO(&read_fds);
	
	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) 
	{
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	for(p = ai; p != NULL; p = p->ai_next) 
	{
		// instantiate socket and keep track of it
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0)
		{
			continue;
		}
		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) 
		{
			close(listener);
			continue;
		}

		break;
	}	

	// if we got here, it means we didn't get bound
	if (p == NULL) 
	{
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

	// listen
	if (listen(listener, 10) == -1) 
	{
		perror("listen");
		exit(3);
	}

	// add the listener to the master set
	/*
		new connection = add to the master set
		close connection = remove from the master set
	*/
	FD_SET(listener, &master);

	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	// main loop for multi-client chat server
	for(;;) 
	{
		read_fds = master; // copy it
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) 
		{
			perror("select");
			exit(4);
		}
		// run through the existing connections looking for data to read
		for(i = 0; i <= fdmax; i++) 
		{
			printf("checking %i\n",i);
			if (FD_ISSET(i, &read_fds)) 
			{ // we got one!!
				if (i == listener) 
				{
					// handle new connections
					addrlen = sizeof remoteaddr;
					newfd = accept(listener,
					(struct sockaddr *)&remoteaddr,
					&addrlen);
					if (newfd == -1) 
					{
						perror("accept");
					} 
					else 
					{
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) 
						{
							// keep track of the max
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on "
						"socket %d\n",
						inet_ntop(remoteaddr.ss_family,
						get_in_addr((struct sockaddr*)&remoteaddr),
						remoteIP, INET6_ADDRSTRLEN),
						newfd);
					}
				} 
				else 
				{
					// handle data from a client
					if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) 
					{
						// got error or connection closed by client
						if (nbytes == 0) 
						{
							// connection closed
							printf("selectserver: socket %d hung up\n", i);
						} 
						else 
						{
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // remove from master set
					} 
					else 
					{
						printf("data received %s\n", buf);
						for(j = 0; j <= fdmax; j++) 
						{
							// send to everyone!
							if (FD_ISSET(j, &master)) 
							{
								// except the listener and ourselves
								if (j != listener && j != i) 
								{
									if (send(j, buf, nbytes, 0) == -1) 
									{
										perror("send");
									}
								}
							}
						}
					}
				} // END handle data from client
			} // END got new incoming connection
		} // END looping through file descriptors
	} // END for(;;)--and you thought it would never end!
return 0;

}