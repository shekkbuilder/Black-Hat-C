/*

Executable name : tcp_server
Designed OS     : Linux
Version         : 3.0
Created date    : 4/5/2017
Last update     : 5/2/2017
Author          : Milton Valencia (wetw0rk)
Inspired by     : Black Hat Python
GCC Version     : 6.3.0
Description     : A simple TCP server that accepts a response from
		  a client and then proceeds to send ACK! closing
		  the established connection.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BIND_IP "127.0.0.1"
#define BIND_PORT 9999

// this is our client-handling thread
void * handle_client(void * tcp_socket)
{
	// get the socket descriptor
	int sock = *(int*)tcp_socket;
	int read_buf;
	char * data, response[2000];

	// print out what the client sends
	recv(sock, response, 2000, 0);
	printf("[*] Received: %s\n", response);

	// send back a packet
	data = "ACK!\n";
	write(sock, data, strlen(data));

	close(sock); // close the client socket

	return(0);
}


int main()
{
	int tcp_socket, new_socket, c, * nsock;
	struct sockaddr_in server, client;

	// TCP socket (AF_INET -> IPV4, SOCK_STREAM -> TCP, 0 -> IP protocol)
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET; // ipv4
	server.sin_addr.s_addr = inet_addr(BIND_IP); // all interfaces
	server.sin_port = htons(BIND_PORT); // port

	bind(tcp_socket,(struct sockaddr *)&server, sizeof(server)); // bind
	listen(tcp_socket, 3); // listen up to 3

	// Accept incoming connections (not 0.0.0.0 for all interfaces)
	printf("[*] Listening on %s:%d\n", BIND_IP, BIND_PORT);
	c = sizeof(struct sockaddr_in);
	while((new_socket = accept(tcp_socket, (struct sockaddr *)&client, (socklen_t*)&c)))
	{
		printf("[*] Accepted connection from: %s:%d\n", inet_ntoa(client.sin_addr), client.sin_port);

		pthread_t sniffer_thread;
		nsock = malloc(1);
		*nsock = new_socket;

		// join the thread
		pthread_create(&sniffer_thread, NULL, handle_client, (void*) nsock);
	}

	return(0);
}
