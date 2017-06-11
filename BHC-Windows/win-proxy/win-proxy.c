/*

Executable name : win-proxy
Designed OS     : Windows
Version         : 2.0
Created date    : 5/23/2017
Last update     : 7/11/2017
Author          : wetw0rk
Inspired by     : Black Hat Python
GCC Version     : 6.3.0
Description     : This proxy an be used for many things like, forwarding
                  traffic to bounce from host to host or modifying traffic
                  being sent to an application. An example usage would be:
                  ./proxy 127.0.0.1 21 <target> 21 True <-- Terminal 1
                  ftp 127.0.0.1 <-- Terminal 2

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <winsock2.h>
#include <windows.h>

#define BUFF_SIZE 5000

// use a struct to access function args.
struct argv_struct
{
	char	*lh;	/* local host  */
	char	*rh;	/* remote host */
	int	lp;	/* local port  */
	int	rp;	/* remote port */
	int	rf;	/* recv flag   */
	int	*ns;	/* client sock */
};

// this is a pretty hex dumping function directly taken from here:
// http://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
void hexdump (char *desc, void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char*)addr;

	// Process every byte in the data.
	for (i = 0; i < len; i++)
	{
		// Multiple of 16 means new line (with line offset)
		if ((i % 16) == 0)
		{
			// Just don't print ASCII for the zeroth line.
			if (i != 0)
			{
				printf ("  %s\n", buff);

			}
			// Output the offset.
			printf ("  %04x ", i);

		}
		// Now the hex code for the specific character.
		printf (" %02x", pc[i]);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
		{
			buff[i % 16] = ' ';
		}
		else
		{
			buff[i % 16] = pc[i];
		}

		buff[(i % 16) + 1] = '\0';
	}
	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0)
	{
		printf ("   ");
		i++;
	}
	// And print the final ASCII bit.
	printf ("  %s\n", buff);

	return;
}

// modify any requests destined for the remote host
char * response_handler(char remote_buffer[BUFF_SIZE])
{
	// perform packet modifications
	return remote_buffer;
}

//modify any responses destined for the local host
char * request_handler(char local_buffer[BUFF_SIZE])
{
	// perform packet modifications
	return local_buffer;
}

DWORD WINAPI proxy_handler(void * arguments)
{
	struct argv_struct *args = arguments;
	int client_socket = *(int*)args->ns;

	int remote_socket;
	struct sockaddr_in remote_sock;
	char local_buffer[BUFF_SIZE], remote_buffer[BUFF_SIZE];

	ssize_t bytes_read;

	// Connect to the remote host
	remote_socket  = socket(AF_INET, SOCK_STREAM, 0);

	remote_sock.sin_addr.s_addr = inet_addr(args->rh);
	remote_sock.sin_family = AF_INET;
	remote_sock.sin_port = htons(args->rp);

	connect(remote_socket,(struct sockaddr *)&remote_sock,sizeof(remote_sock));

	// receive data from the remote end if necessary
	if (args->rf == 1)
	{
		bytes_read = recv(remote_socket, remote_buffer, BUFF_SIZE, 0);
		hexdump(remote_buffer, &remote_buffer, bytes_read);

		// send it to our response handler
		response_handler(remote_buffer);

		// if we have data to send to our local client, send it
		if (bytes_read >  1)
		{
			printf("[<==] Sending %d bytes to localhost.\n",bytes_read);
			send(client_socket, remote_buffer, bytes_read, 0);
		}
	}
	// now lets loop and read from local,
		// send to remote, send to local
	// rinse, wash, repeat
	while(1)
	{
		// read from local host
		bytes_read = recv(client_socket, local_buffer, BUFF_SIZE, 0);

		if (bytes_read > 1)
		{
			printf("[==>] Received %d bytes from localhost.\n", bytes_read);

			hexdump(local_buffer, &local_buffer, bytes_read);
			// send it to our request handler
			request_handler(local_buffer);

			// send off the data to the remote host
			send(remote_socket, local_buffer, bytes_read, 0);
			printf("[==>] Sent to remote.\n");
		}

		// receive back the response
		bytes_read = recv(remote_socket, remote_buffer, BUFF_SIZE, 0);

		if (bytes_read > 1)
		{
			printf("[<==] Received %d bytes from remote.\n", bytes_read);
			hexdump(remote_buffer, &remote_buffer, bytes_read);

			// send to our response handler
			response_handler(remote_buffer);

			// send the response to the local socket
			send(client_socket, remote_buffer, bytes_read, 0);

			printf("[<==] Sent to localhost.\n");
		}

		// if no more data on either side, close the connections
		if (bytes_read <= 0)
		{
			close(client_socket);
			close(remote_socket);
			printf("[*] No more data. Closing connections.");
			break;
		}
	}
}

int server_loop(char *local_host, int local_port, char *remote_host, int remote_port, int receive_flag)
{
	int server_socket, client_socket, client_len, *nsock;
	struct sockaddr_in server, client;
	struct argv_struct argv;

	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2), &wsa);

	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(local_host);
	server.sin_port = htons(local_port);

	if (bind(server_socket,(struct sockaddr *)&server, sizeof(server)) < 0)
	{
		printf("[!!] Failed to listen on %s:%d\n", local_host, local_port);
		printf("[!!] Check for other listening sockets or correct permissions.\n");
		exit(1);
	}

	printf("[*] Listening on %s:%d\n", local_host, local_port);
	listen(server_socket, 5);

	client_len = sizeof(struct sockaddr_in);

	while((client_socket = accept(server_socket, (struct sockaddr *)&client, &client_len)))
	{
		// print out the local connection information
		printf("[==>] Received incoming connection from %s:%d\n", inet_ntoa(client.sin_addr), client.sin_port);

		// struct variables
		argv.rh = remote_host;		// remote host
		argv.rp = remote_port;		// remote port
		argv.rf = receive_flag;		// receive first
		argv.ns = malloc(1);		// allocate mem
		*argv.ns = client_socket;	// client socket

		// start a thread to talk to the remote host
		CreateThread(0,0,&proxy_handler, (void*)&argv, 0,0);
	}

	return(0);
}

int main(int argc, char *argv[])
{
	int local_port, remote_port, receive_flag;
	char *local_host, *remote_host, *receive_first;

	// no fancy command-line parsing here
	if (argc < 5)
	{
		printf("Usage: %s [localhost] [localport] [remotehost] [remoteport] [receive_first]\n", argv[0]);
		printf("Example: %s 127.0.0.1 9000 10.12.132.1 9000 True\n", argv[0]);
		exit(0);
	}

	// setup local listening parameters
	local_host = argv[1];
	local_port = atoi(argv[2]);

	// setup remote target
	remote_host = argv[3];
	remote_port = atoi(argv[4]);

	// this tells our proxy to connect and receive data
	// before sending to the remote host
	receive_first = argv[5];

	if (strlen(receive_first) == 4) // True is 4 chars
	{
		receive_flag = 1;
	}
	else if (strlen(receive_first) != 4) // False is 5 characters
	{
		receive_flag = 0;
	}

	// now spin up our listening socket
	server_loop(local_host, local_port, remote_host, remote_port, receive_flag);

	return 0;
}
