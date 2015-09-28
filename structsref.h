/*
Authors: Francisco Castro, Antonio Umali
CS 513 Project 1 - Chat Roulette
Last modified: 28 Sept 2015

This is a reference file for the structures being used.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

struct in_addr
{
	uint32_t s_addr;
};

struct sockaddr
{
	unsigned short sa_family;
	char sa_data[14];	
};

struct sockaddr_in
{
	short int sin_family;
	unsigned short int sin_port;
	struct in_addr	sin_addr;
	unsigned char sin_zero[8];	//set to all zeros using memset
};

struct addrinfo
{
	int ai_flags;
	int ai_family;
	int ai_socketype;
	int ai_protocol;
	size_t ai_addrlen;
	struct sockaddr *ai_addr;
	char *ai_canonname;
	struct addrinfo *ai_next;
};


struct sockaddr_storage
{
	sa_familty_t ss_family;

	char __sspad1[_SS_PAD1SIZE];
	int64_t __ss_align;
	char __ss_pad2[_SS_PAD2SIZE];
};
