/*

Executable name : win-tcp-server
Designed OS     : Windows
Version         : 1.0
Created date    : 5/7/2017
Last update     : 5/7/2017
Author          : wetw0rk
Inspired by     : Black Hat Python
GCC Version     : 6.3.0
Description     : A simple TCP server that accepts a response from
                  a client and then proceeds to send ACK! closing
                  the established connection.

*/

#include <stdio.h>
#include <winsock2.h>
#include <windows.h>

#define BIND_IP "192.168.176.128"
#define BIND_PORT 9999

// this is our client-handling thread
DWORD WINAPI handle_client(void * tcp_socket)
{
	int csock = *(int*)tcp_socket;		// get the socket descriptor
	char * data, response[2000];		// size of recieve buffer

	recv(csock, response, 2000, 0);		// receive message from client
	printf("[*] Received: %s\n", response); // print message

	data = "ACK\n";				// send back a packet
	send(csock, data, strlen(data), 0);

	closesocket(csock);			// close client socket

	return(0);

}

int main()
{
	/*
	tcp_socket	: socket descriptor for server
	new_socket	: socket descriptor for client
	server		: local address
	client		: client address
	client_len      : length of client address data structure
	*/
	int tcp_socket, new_socket, client_len, *nsock;
	struct sockaddr_in server, client;

	/*
	WSADATA		: structure for WinSock setup communication
	WSAStartup()	: load winsock 2.0 dll
	*/
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2), &wsa);

	/*
	AF_INET		: IPv4
	SOCK_STREAM	: TCP
	0		: IP Protocol
	*/
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;			// internet address family
	server.sin_addr.s_addr = inet_addr(BIND_IP);	// incoming interface
	server.sin_port = htons(BIND_PORT);		// bind port

	bind(tcp_socket, (struct sockaddr *) &server, sizeof(server)); // bind

	listen(tcp_socket, 3); // listen up to 3

	client_len = sizeof(struct sockaddr_in); // size of in-out parameter

	// accept incoming connections
	printf("[*] Listening on %s:%d\n", BIND_IP, BIND_PORT);
	while((new_socket = accept(tcp_socket , (struct sockaddr *)&client, &client_len)))
	{
		// new socket is connected to the client
		printf("[*] Accepted connection from: %s:%d\n", inet_ntoa(client.sin_addr), client.sin_port);

		nsock = malloc(1);
		*nsock = new_socket;

		// join the thread
		CreateThread(0,0,&handle_client, (void*)nsock, 0,0);
	}

	return(0);
}
