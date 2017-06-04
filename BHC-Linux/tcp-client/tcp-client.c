/*

Executable name : tcp-client
Designed OS     : Linux
Version         : 5.0
Created date    : 4/5/2017
Last update     : 6/7/2017
Author          : Milton Valencia (wetw0rk)
Inspired by     : Black Hat Python
GCC Version     : 6.3.0
Description     : A simple TCP client that connects to google
		  and recieves a response (Note: IP may change).
                  Heavily commented code for learning purposes.

*/

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TARGET_HOST "216.58.217.206"
#define TARGET_PORT 80

int main()
{
	/*
	tcp_socket      : socket descriptor for client
	client          : local address
	data            : data to be sent
	response        : where our response will be stored
	*/
	int tcp_socket;
	struct sockaddr_in client;
	char data[]= "GET / HTTP/1.1\r\n\r\n", response[4096];

	/*
	AF_INET         : IPv4
	SOCK_STREAM     : TCP
	0               : IP Protocol
	*/
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

	client.sin_addr.s_addr = inet_addr(TARGET_HOST);	// interface
	client.sin_family = AF_INET;				// internet address family
	client.sin_port = htons(TARGET_PORT);			// bind port

	// establish connection
	connect(tcp_socket,(struct sockaddr *)&client,sizeof(client));

	// send the data
	send(tcp_socket, data, strlen(data), 0);

	// recieve the reply and print it
	recv(tcp_socket, response, 4096, 0);
	puts(response);

	return(0);
}
