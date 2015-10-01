/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 29 Sept 2015

Several pieces of code for reference.
*/

// [ Reap dead processes that appear as the fork()ed child processes exit ]
//=================================================================================

sa.sa_handler = sigchld_handler; // reap all dead processes
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;
if (sigaction(SIGCHLD, &sa, NULL) == -1) 
{
	perror("sigaction");
	exit(1);
}

//=================================================================================


//=================================================================================

// Main accept() loop in server
while(1)
{
	// Accept an incoming connection on the listening socket descriptor, listener
	addrlen = sizeof remoteaddr;
	new_fd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);	// new socket descriptor for new connection

	// If error occurs on accept(), it returns -1 and sets errno
	if (new_fd == -1) 
	{
		perror("accept");
		continue;
	}

	// Convert a struct in_addr to numbers-and-dots notation (IP address) for printing
	inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, sizeof remoteIP);
	printf("Server: got connection from %s\n", remoteIP);

	// This is the child process
	if (!fork())
	{
		close(listener); // child doesn't need the listener

		if (send(new_fd, "Hello, world!", 13, 0) == -1)
		{
			perror("send");
		}

		close(new_fd);
		exit(0);
	}

	close(new_fd); // parent doesn't need this
}

//=================================================================================


//=================================================================================

// create timeoutstruct
struct timeval tv;
tv.tv_sec = 2;
tv.tv_usec = 500000;

// set file descriptors
fd_set readfds;
FD_ZERO(&readfds);
FD_SET(STDIN, &readfds);
	
select(STDIN+1, &readfds, NULL, NULL, &tv);
if (FD_ISSET(STDIN, &readfds))
	printf("A key was pressed!\n");
else
	printf("Timed out.\n");

//=================================================================================

