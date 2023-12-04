/*
 * server.c
 *
 *  Created on: Nov 20, 2023
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
#include "calculator.h"


#define NO_ERROR 0

typedef struct sockaddr_in sockAddrIn;

void clearwinsock();
int generateSocket(size_t domain, size_t streamType, size_t protocol);
sockAddrIn setAddress(char *adress, size_t family, size_t port);
int binding(int socket, sockAddrIn *serverAddress);
int recvMsg(int socket, sockAddrIn *clientAddress, char* buffer, size_t buffer_size);
int sendMsg(int socket, sockAddrIn *clientAddress, char* buffer, size_t buffer_size);
int parser(char *operand, double op[NUM_OPERANDS], char *buffer);





int main(int argc, char *argv[])
{
#ifdef WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	int serverSocket;
	if( (serverSocket = generateSocket(PF_INET, SOCK_DGRAM, IPPROTO_UDP) )== -1) return -1;


	sockAddrIn clientAddress, serverAddress;
	serverAddress = setAddress("127.0.0.1", AF_INET, PORT);

	if(binding(serverSocket, &serverAddress) == -1) return -1;

	char buffer[BUFFER_SIZE];

	for(;;)
	{
		memset(buffer, 0, BUFFER_SIZE);
		puts("Server listening...");


		if(recvMsg(serverSocket, &clientAddress, buffer, BUFFER_SIZE) == -1) continue;
		printf("Received message: \"%s\", from client %s, IP: %s)\n", buffer,  gethostbyaddr((char *)(&clientAddress.sin_addr.s_addr), sizeof(clientAddress.sin_addr.s_addr), AF_INET)->h_name, inet_ntoa(clientAddress.sin_addr));
		double op[2];
		char operand;

		parser(&operand, op, buffer);

		memset(buffer, 0, BUFFER_SIZE);


		sprintf(buffer, (operand != '=')? "%.2lf %c %.2lf = %.2lf" : "end", op[0], operand, op[1], doOperation(op, operand));

		if( sendMsg(serverSocket, &clientAddress, buffer, strlen(buffer)) == -1) continue;

	}
	clearwinsock();
	closesocket(serverSocket);


}


int parser(char *operand, double op[NUM_OPERANDS], char *buffer)
{
	//setto a 0 tutti gli operandi
	memset(op, 0, sizeof(int) * NUM_OPERANDS);

	switch(buffer[0])
	{
	case '+':
		*operand = '+';
		break;
	case '-':
		*operand = '-';
		break;
	case '*':
	case 'x':
		*operand = '*';
		break;
	case '/':
		*operand = '/';
		break;
	case '=':
		*operand = '=';
		return 1;
	default:
		return -1;
	}
	//move the pointer to the buffer to where i should find the first operand
	buffer+=2;
	//strtod converte la stringa in un double e restituisce un puntatore a dove termina il numero trovato
	//se becca una stringa che non è convertibile ritorna 0
	for(int i = 0; i < NUM_OPERANDS && buffer != NULL; ++i)	op[i] = strtod(buffer, &buffer);




	return 0;


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

void clearwinsock()
{
#if defined WIN32
	WSACleanup();
#endif
}

