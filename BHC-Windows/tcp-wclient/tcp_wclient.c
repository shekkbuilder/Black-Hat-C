/*

Executable name : tcp_wclient
Designed OS     : Windows
Version         : 1.0
Created date    : 5/1/2017
Last update     : 5/1/2017
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
	WSADATA wsa;
	SOCKET tcp_socket;
	struct sockaddr_in server;
	char * data, response[2000];

	// Initialise Winsock
	WSAStartup(MAKEWORD(2,2),&wsa);

	// TCP socket (AF_INET -> IPV4, SOCK_STREAM -> TCP, 0 -> IP protocol)
	tcp_socket = socket(AF_INET , SOCK_STREAM , 0);

	server.sin_addr.s_addr = inet_addr(TARGET_HOST); // host
	server.sin_family = AF_INET; // ipv4
	server.sin_port = htons(TARGET_PORT); // port

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
