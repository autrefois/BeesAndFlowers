/* Bee = Node 
 * CLient for the flower field (hub)
 * Server (provider) for the beehive (client) */

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<errno.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Message sent to hub */
struct hubmsg_t {
    int mtype;
    long port;
};

/* Message to/from client */
struct clientmsg_t {
    int mtype;
    char name[4000];
    long from; // pos or length
};

struct sockaddr_in address, client_addr, hub_addr;

void* handleClient(void* arg) 
{
	printf("[BEE] Accepted connection...\n");	

	int sock = *((int*) arg);
	int i, len, f;
	char file[256];

	struct clientmsg_t clientmsg;
	struct clientmsg_t sent;

	struct sockaddr_in address; // client
	len = sizeof(address);	
	
	getpeername(sock, (struct sockaddr*)&address, &len);
	// new socket structure

	i = recv(sock, &clientmsg, sizeof(struct clientmsg_t), 0);
	if (i < 0) 
	{
		perror("[BEE] recv from client error:");
		return NULL;
	}
	else printf("[BEE] Received request...\n");

	if (clientmsg.mtype == 4)
	{
		/* Client requested stuff */

		strcpy(file, "source/");
		strcat(file,clientmsg.name);

		printf("[BEE] Discard polen to hive (%s)...\n", file);

		/* Open file, bin mode, read-only. */
		f = open(file, O_RDONLY);

		if (f < 0)
		{
			printf("[DRONE] Error opening the file '%s'\n", file);
			perror(":");
			return NULL;
		}

		if (lseek(f, clientmsg.from, SEEK_SET) < 0)
		{
			printf("[BEE] Can't seek in %s.\n",file);
			return NULL;
		}
		
		sent.mtype = 5;
		sent.from = read(f, sent.name, 100);

		if (sent.from < 0)
		{
			printf("[BEE] Error reading file %s.\n", file);
			return NULL;
		}

		int ok;
		ok = send(sock, &sent, sizeof(struct clientmsg_t), 0);

		if (ok<0) 
		{
			perror("[BEE] Send to hive error:"); 
			return NULL;
		}
		
	}
	else
	{
		/* Unrecognized message (0 = Client finished) */
	}

return NULL;
}


int main() {
	int sock, sockhub, sock_client;
	int i, len;

	struct hubmsg_t hubmsg;
	struct clientmsg_t clientmsg;

	pthread_t thr;

	printf("[BEE] Starting...\n");

	sockhub = socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0) {
		perror("[BEE] Error creating:");
	}	

	printf("[BEE] Created sock...\n");

	/* First of all, the node must connect, i.e. send message to the hub. */

	hub_addr.sin_family = AF_INET;
	hub_addr.sin_addr.s_addr = INADDR_ANY;
	hub_addr.sin_port = htons(1026);

	if (connect(sockhub, (struct sockaddr*) &hub_addr, sizeof(struct sockaddr))<0) {
		perror("[BEE] Connect error:");
	}

	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);

	srand(tv.tv_usec);

	hubmsg.mtype = 1;
	hubmsg.port = 1026 + rand()%1000;

	printf("[BEE] Created request for hub...\n");

        i = send(sockhub, &hubmsg, sizeof(struct hubmsg_t), 0);
	if (i<0) perror("[BEE] can't send to hub :");

	printf("[BEE] Sent.\n");
	
	/* Second of all, the node must be able to accept connections from clients.
	I.e. create a new socket, as the socket used for the communication with the hub cannot be reused.
	(as it already acts like a client, and we need one to act like a server) */

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0) {
		perror("[BEE] Error creating:");
	}

	printf("[BEE] Created second socket...\n");
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(hubmsg.port);

	if (bind(sock, (struct sockaddr*) &address, sizeof(struct sockaddr))<0) {
		perror("[BEE] Error binding");
	}

	printf("[BEE] Binded...\n");

	if (listen(sock, 20) < 0)
		perror("[BEE] won't listen");

	printf("[BEE] Listening...\n");
	
	while(1) {
		len = sizeof(struct sockaddr_in);
		sock_client = accept(sock, (struct sockaddr*) &client_addr, &len);
		pthread_create(&thr, 0, handleClient, &sock_client);
	sleep(1);
	} 
close(sock_client);
close(sockhub);
close(sock);
return 0;
}
