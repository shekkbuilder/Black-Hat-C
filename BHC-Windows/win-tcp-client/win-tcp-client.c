/*

Executable name : win-tcp-client
Designed OS     : Windows
Version         : 2.0
Created date    : 5/1/2017
Last update     : 5/7/2017
Author          : Milton Valencia (wetw0rk)
Inspired by     : Black Hat Python
GCC Version     : 6.3.0
Description     : A simple TCP client that connects to google
		  and recieves a response (Note: IP may change)

*/

#include <stdio.h>
#include <winsock2.h>

#define TARGET_HOST "216.58.217.206"
#define TARGET_PORT 80


int main()
{
	/*
	WSADATA         : structure for WinSock setup communication
	WSAStartup()    : load winsock 2.0 dll
	*/
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
	/*
	tcp_socket      : socket descriptor for client
	server          : local address
	*/
	int tcp_socket;
	struct sockaddr_in server;
	char * data, response[2000];

	/*
	AF_INET         : IPv4
	SOCK_STREAM     : TCP
	0               : IP Protocol
	*/
	tcp_socket = socket(AF_INET , SOCK_STREAM , 0);

	server.sin_addr.s_addr = inet_addr(TARGET_HOST);	// interface
	server.sin_family = AF_INET;				// internet address family
	server.sin_port = htons(TARGET_PORT);			// bind port

	connect(tcp_socket,
		(struct sockaddr *)&server,
		sizeof(server));

	// data to be sent
	data = "GET / HTTP/1.1\r\n\r\n";

	// send the data
	send(tcp_socket, data, strlen(data) , 0);

	// recieve the reply and print it
	recv(tcp_socket, response, 2000, 0);
	puts(response);

	return 0;
}
