/*

Executable name : bhcnet
Designed OS     : Linux
Version         : 2.0
Created date    : 5/7/2017
Last update     : 7/11/2017
Author          : wetw0rk
Inspired by     : Black Hat Python
GCC Version     : 6.3.0
Description     : Basically a custom netcat just like bhpnet. Since
		  the goal is to replicate the structure like bhpnet
                  i did not include any custom options, however they
                  can be easily installed by you; the user. This is a
		  great "skeleton" for something much bigger if you
                  would like to take it that route. Note: depending
		  on file size, if uploading a file change the RESP_SIZE
                  accordingly. Tested binarys and .txt files.

*/

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define BUFF_SIZE 5000
#define RESP_SIZE 4096

// define some global variables (0 == false)
void    usage(void);		// Usage
char	buffer[BUFF_SIZE];	// Buffer
char	*tflag;			// Target
char	*uflag;			// Upload
char	*eflag;			// Execute
int	pflag;			// Port
int	lflag = 0;		// Listen
int	cflag = 0;		// Command

// this runs a command and returns the output
char * run_command(char response[RESP_SIZE])
{
	FILE * run;

	char buff[RESP_SIZE];

	int i=0, size_of_array;			// loop and size of array
	char A[1000][1000];			// array for captured buffer
	static char string[1000];		// store the array into a string

	memset(string,0,sizeof(string));	// zero out

	run = popen(response, "r");		// run the command

	while(fgets(buff, sizeof(buff), run)!=NULL)	// loop and then
	{						// copy the buffer
		strcpy(A[i], buff);			// into an array;
		i++;					// increment array
	}

	pclose(run);					// close the process

	size_of_array = i;				// get size of array

	for (i = 0; i < size_of_array; i++)
	{
		strcat(string, A[i]);			// copy or add into a string
	}

	return string;					// return the output

}

// this handles incoming client connections
void * client_handler(void * tcp_socket)
{
	int sock = *(int*)tcp_socket;
	char response[RESP_SIZE];
	char ret[BUFF_SIZE];
	ssize_t bytes_read;

	// check for upload
	if (uflag != NULL && cflag == 0 && eflag == NULL)
	{
		FILE * file = fopen(uflag, "w+b");

		// keep reading data until none is available
		bytes_read = recv(sock, response, RESP_SIZE, 0);

		// now we take these bytes and try to write them out
		fwrite(response, sizeof(char), bytes_read, file);
		fclose(file);
		exit(0);

	}

	// check for command execution
	if (eflag != NULL && cflag == 0 && uflag == NULL)
	{
		// run the command
		strcat(ret, run_command(eflag));

		write(sock, ret, strlen(ret));
	}

	// now we go into another loop if a command shell was requested
	if (cflag == 1 && eflag == NULL && uflag == NULL)
	{
		while(1)
		{
			// show a simple prompt
			write(sock, "<BHC:#> ", 9);

			// now we receive until we see a linefeed (enter key)
			do {

				recv(sock, response, RESP_SIZE, 0);

			} while (!strchr((response), '\n'));

			// we have a valid command so execute it and send back the results
			strcat(ret, run_command(response));

			// send back the response
			write(sock, ret, strlen(ret));

			// zero out
			memset(response,0,sizeof(response));
			memset(ret,0,sizeof(ret));
		}
	}
}

// this is for incoming connections
int server_loop(char * tflag, int pflag)
{
	int tcp_socket, new_socket, c, *nsock;
	struct sockaddr_in server, client;

	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;

	// if no target is defined, we listen on all interfaces
	if (tflag == NULL)
	{
		server.sin_addr.s_addr = INADDR_ANY;
	}
	else if (tflag != NULL)
	{
		server.sin_addr.s_addr = inet_addr(tflag);
	}

	server.sin_port = htons(pflag);

	bind(tcp_socket,(struct sockaddr *)&server, sizeof(server));
	listen(tcp_socket, 5);

	c = sizeof(struct sockaddr_in);
	while((new_socket = accept(tcp_socket, (struct sockaddr *)&client, (socklen_t*)&c)))
	{
		pthread_t sniffer_thread;
		nsock = malloc(1);
		*nsock = new_socket;

		// spin off a thread to handle our new client
		pthread_create(&sniffer_thread, NULL, client_handler, (void*) nsock);
	}
}

// if we don't listen we are a client....make it so
int client_sender(char *tflag, int pflag, char *buffer)
{
	int tcp_socket, recv_len;
	struct sockaddr_in server;
	char response[RESP_SIZE];
	ssize_t bytes_read;

	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_addr.s_addr = inet_addr(tflag);
	server.sin_family = AF_INET;
	server.sin_port = htons(pflag);

	// connect to our target host
	connect(tcp_socket,(struct sockaddr *)&server,sizeof(server));

	// if we detect input from stdin send it
	// if not we are going to wait for the user to punch some in
	if (strlen(buffer) > 1)
	{
		send(tcp_socket, buffer, strlen(buffer), 0);
	}

	while(1)
	{
		// now wait for data back
		recv_len = 1;

		while(recv_len)
		{
			bytes_read = recv(tcp_socket, response, RESP_SIZE, 0);
			recv_len = bytes_read;

			if (recv_len < RESP_SIZE)
			{
				break;
			}
		}

		fputs(response, stdout);

		// wait for more input
		fgets(buffer, BUFF_SIZE, stdin);

		// send it off
		send(tcp_socket, buffer, strlen(buffer), 0);
	}
}

void usage(void)
{
	printf("BHC Net Tool\n\n");
	printf("Usage: bhcnet -t target_host -p port\n");
	printf("-l --listen              - listen on [host]:[port] for incoming connections\n");
	printf("-e --execute file_to_run - execute the given file upon receiving a connection\n");
	printf("-c --command             - initialize a command shell\n");
	printf("-u --upload destination  - upon receiving a connection upload a file and write to [destination]\n\n");
	printf("Examples:\n");
	printf("bhcnet -t 192.168.0.1 -p 5555 -l -c\n");
	printf("bhcnet -t 192.168.0.1 -p 5555 -l -u /root/sploit.out\n");
	printf("bhcnet -t 192.168.0.1 -p 5555 -l -e \"cat /etc/passwd\"\n");
	printf("echo 'ABCDEFGHI' | ./bhcnet -t 192.168.11.12 -p 135\n");
	exit(1);
}


int main(int argc, char * argv[])
{
	int opt = 0;

	// read the commandline options
	static struct option long_options[] =
	{
		{"help",        no_argument,            0,      'h'},
		{"listen",      no_argument,            0,      'l'},
		{"execute",     required_argument,      0,      'e'},
		{"command",     no_argument,            0,      'c'},
		{"upload",      required_argument,      0,      'u'},
		{"target",      required_argument,      0,      't'},
		{"port",        required_argument,      0,      'p'},
		{0,		0,			0,	0}
	};

	int long_index = 0;
	while ((opt = getopt_long(argc, argv,"hle:t:p:cu:", long_options, &long_index)) != -1)
	{
		switch(opt)
		{
			case 'h':
				usage();
				break;
			case 'l':
				lflag = 1;
				break;
			case 'e':
				eflag = optarg;
				break;
			case 'c':
				cflag = 1;
				break;
			case 'u':
				uflag = optarg;
				break;
			case 't':
				tflag = optarg;
				break;
			case 'p':
				pflag = atoi(optarg);
				break;
		}
	}


	if (argc <= 1)
	{
		usage();
	}

	// are we going to listen or just send data from stdin?
	if (tflag != 0 && pflag > 0 && lflag == 0)
	{
		// read in buffer from the command line
		// this will block, so send CTRL-D if not sending input
		// to stdin
		fgets(buffer, BUFF_SIZE, stdin);

		// send data off
		client_sender(tflag, pflag, buffer);
	}

	// we are going to listen and potentially
	// upload things, execute commands and drop a shell back
	// depending on our command line options above
	if (lflag > 0 && pflag > 0)
	{
		server_loop(tflag, pflag);
	}
}
