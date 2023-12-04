/*
 * client.c
 *
 *  Created on: Nov 25, 2023
 *      Author: francesco
 */

#ifdef WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif


#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"




#define NO_ERROR 0

typedef struct sockaddr_in sockAddrIn;
typedef struct sockaddr sockAddr;
typedef struct addrinfo addrInfo;

void clearwinsock();
int generateSocket(size_t domain, size_t streamType, size_t protocol);
sockAddrIn setAddress(char *adress, size_t family, size_t port);
int binding(int socket, sockAddrIn *serverAddress);
int recvMsg(int socket, sockAddrIn *clientAddress, char* buffer, size_t buffer_size);
int sendMsg(int socket, sockAddrIn *clientAddress, char* buffer, size_t buffer_size);
addrInfo setHints(int family, int sockType, int flags, int protocol);
int getInfo(char *serverName, char *portStr, addrInfo* hints, addrInfo** results);
void clearStdin(void);

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./%s servername:port", argv[0]);
		return 1;
	}


#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != NO_ERROR)
	{
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int socketfd;

	char buffer[BUFFER_SIZE];
	addrInfo hints, *results = NULL;

	char *serverName = strtok(argv[1], ":");
	char *portStr = strtok(NULL, ":");

	hints = setHints(AF_INET, SOCK_DGRAM, AI_PASSIVE, 0);

	if (getInfo(serverName, portStr, &hints, &results) != 0) return -1;

	if ((socketfd = generateSocket(PF_INET, results->ai_socktype, results->ai_protocol)) == -1) return -1 ;

	do
	{
		memset(buffer, 0, BUFFER_SIZE);
		printf("Input Operation: ");
		fgets(buffer, BUFFER_SIZE-1, stdin);

		char *returnFinder;
		if((returnFinder = strchr(buffer, '\n')) != NULL) *returnFinder = '\0';
		else clearStdin();


		sendto(socketfd, buffer, BUFFER_SIZE, 0,results->ai_addr, (results->ai_addrlen));

		memset(buffer, 0, BUFFER_SIZE);


		recvfrom(socketfd, buffer, BUFFER_SIZE, 0, results->ai_addr, &(results->ai_addrlen));

		printf("Recived from server: %s, ip: %s: %s\n", serverName ,inet_ntoa(((sockAddrIn *)results->ai_addr)->sin_addr) , buffer);

	}while(strcmp(buffer, "end"));

	freeaddrinfo(results);
}

int getInfo(char *serverName, char *portStr, addrInfo* hints, addrInfo** results)
{
	int s;
	if( (s = getaddrinfo(serverName, portStr, hints, results)) != 0)
	{
		puts(gai_strerror(s));
		return -1;
	}
	if(results == NULL)
	{
		puts("No info found.");
		return -2;
	}

	return 0;
}

addrInfo setHints(int family, int sockType, int flags, int protocol)
{
	addrInfo hints;
	memset(&hints, 0, sizeof(addrInfo));
	hints.ai_family = family;
	hints.ai_socktype = sockType;
	hints.ai_flags = flags;
	hints.ai_protocol = protocol;
	return hints;
}



int recvMsg(int socket, sockAddrIn *clientAddress, char *buffer, size_t buffer_size)
{
	int byteRecived;
	memset(clientAddress, 0, sizeof(*clientAddress));
	socklen_t client_len = sizeof(clientAddress);
	if( (byteRecived = recvfrom(socket, buffer, buffer_size, 0,  (struct sockaddr *)clientAddress, &client_len))  < 0 )
	{
		puts("recived failed");
		return -1;
	}
	return byteRecived;
}

int sendMsg(int socket, sockAddrIn *clientAddress, char* buffer, size_t buffer_size)
{
	if( sendto(socket, buffer , buffer_size, 0, (struct sockaddr *)clientAddress, (socklen_t )sizeof(*clientAddress)) != strlen(buffer))
	{
		puts("send failed");
		return -1;
	}

	return 0;
}




int generateSocket(size_t domain, size_t streamType, size_t protocol)
{
	int sock;
	if(( sock = socket(domain, streamType, protocol)) < 0)
	{
		clearwinsock();
		puts("socket creation failed ");
		return -1;
	}
	else return sock;
}

sockAddrIn setAddress(char *adress, size_t family, size_t port)
{
	sockAddrIn result;
	memset(&result, 0, sizeof(sockAddrIn));
	result.sin_port = htons(port);
	result.sin_addr.s_addr = inet_addr(adress);
	result.sin_family = family;
	return result;
}

int binding(int socket, sockAddrIn * serverAddress)
{
	if(bind(socket, (struct sockaddr *) serverAddress, sizeof(*serverAddress)) < 0)
	{
		puts("Binding failed");
		return -1;
	}
	return 0;
}

int listening(int socket, size_t queueLen)
{
	if(listen(socket, queueLen) < 0)
	{
		puts("Listen failed");
		return -1;
	}
	else return 0;
}

void clearStdin(void)
{
	while(getchar() != '\n');
}

void clearwinsock()
{
#if defined WIN32
	WSACleanup();
#endif
}





