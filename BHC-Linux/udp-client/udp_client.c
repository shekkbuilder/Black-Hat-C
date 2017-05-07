/*

Executable name : udp_client
Designed OS     : Linux
Version         : 4.0
Created date    : 4/5/2017
Last update     : 5/2/2017
Author          : Milton Valencia (wetw0rk)
Inspired by     : Black Hat Python
GCC Version     : 6.3.0
Description     : A simple UDP client that connects to localhost
                  and sends a message, once send awaits a response.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define TARGET_HOST "127.0.0.1"
#define TARGET_PORT 7891

int main()
{
	int udp_socket;
	struct sockaddr_in server;
	char * data, response[2000];

	// UDP Socket (AF_INET -> IPV4, SOCK_DGRAM -> UDP, 0 -> IP protocol)
	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

	server.sin_addr.s_addr = inet_addr(TARGET_HOST); // host
	server.sin_family = AF_INET; // ipv4
	server.sin_port = htons(TARGET_PORT); // port

	// data to be sent
	data = "wetw0rk can you hear me?\r\n";

	// send the data upon connection
	sendto(udp_socket, data, strlen(data), 0,
		(struct sockaddr *)&server, sizeof(server));

	// recieve data and print it
	recvfrom(udp_socket, response, 2000, 0, NULL, NULL);
	puts(response);

	return(0);
}

