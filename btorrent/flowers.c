/* Flower field = Hub
 * Server (provider) for the bee (node)
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

/* Node to hub -> Registration */
struct msgreg_t {
    int mtype;
    short rvport;
};
    
/* Client to hub -> Request node list */
struct msggetnode_t {
    int mtype;
};

struct node {
     struct sockaddr_in address;
     long port;
     int del;
};

/* Hub to client -> list of nodes */
struct listmsg_t {
    int mtype;
    struct node nodeList[10];
};

/* Auxiliary message struct */
struct recvmsg_t {
    int mtype;
    long port;
};

int nodes = 0;
struct node nodeList[10];
struct listmsg_t listmsg;

pthread_mutex_t mtx_msg = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mtxMaxBees = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condMaxBees = PTHREAD_COND_INITIALIZER;

char s[10];
int n;

int addNode(struct sockaddr_in nodeAddr, long port)
{
	nodeList[nodes].address = nodeAddr;
	nodeList[nodes].port = port;
	nodeList[nodes].del = 0;
	listmsg.nodeList[nodes].address = nodeAddr;
	listmsg.nodeList[nodes].port = port;
	listmsg.nodeList[nodes].del = 0;
	nodes++;
	return 0;
}

int remNode(struct sockaddr_in nodeAddr, int port)
{
	int i;
	for (i=0;i<nodes;i++)
	{

		if (nodes == 1)
		{
			nodeList[0].del = 1;
			nodes--;
			return 0;
		}
	
		if (nodeAddr.sin_addr.s_addr == nodeList[i].address.sin_addr.s_addr && nodeAddr.sin_port == nodeList[i].address.sin_port)
		{
			int j;
			int k;
			for (j=i;j<nodes;j++)
			    nodeList[j] = nodeList[j+1];
		        nodeList[nodes].del = 1;
			nodes--;
		}
	}
return 0;
}

void* handleThem(void* arg) 
{
	int sock = *((int*) arg);
	int i, len;

	struct recvmsg_t recvmsg;

	struct sockaddr_in address; // node or client, whatever
	len = sizeof(address);	
	
	getpeername(sock, (struct sockaddr*)&address, &len);
	// new socket structure

	i = recv(sock, &recvmsg, sizeof(struct recvmsg_t), 0);
	if (i < 0) 
	{
		perror("[DRONE] recv error:");
		return NULL;
	}

	if (recvmsg.mtype == 1)
	{
		/* Node requested registration */
		pthread_mutex_lock(&mtxMaxBees);	
		while(nodes >= n)
		{
			printf("[EAGER BEE] Too many bees (nodes); Waiting...\n");			
			pthread_cond_wait(&condMaxBees, &mtxMaxBees);
		}
		// Add node (Register)
		addNode(address, recvmsg.port);
		printf("[DRONE] ++ %d nodes.\n",nodes);	

		printf("[DRONE] Added bee %s : %ld\n",inet_ntoa((*(struct in_addr *)&nodeList[nodes-1].address.sin_addr.s_addr)), nodeList[nodes-1].port);

		i = 0;
		printf("[DRONE] Currently available:\n");	
		while (nodeList[i].address.sin_port != 0)
		{	
			printf("\t[%d] %s : %ld\n", i+1, inet_ntoa(nodeList[i].address.sin_addr),nodeList[i].port);
			i = i + 1;
		}

		pthread_cond_broadcast(&condMaxBees);
		pthread_mutex_unlock(&mtxMaxBees);
	}
	else if (recvmsg.mtype == 2)
	{
		/* Client requested list of nodes */
		int ok;
		listmsg.mtype = 3;
		//listmsg.nodeList = nodeList;

		ok = send(sock, &listmsg, sizeof(struct listmsg_t ), 0);

		if (ok<0) 
		{
			perror("[DRONE] Send error:"); 
		}

		printf("[DRONE] Sent polen to the client.\n");
		i = 0;
		printf("[DRONE] Currently available:\n");	
		while (nodeList[i].address.sin_port != 0)
		{	
			printf("\t[%d] %s : %ld\n", i+1, inet_ntoa(listmsg.nodeList[i].address.sin_addr),listmsg.nodeList[i].port);
			i = i + 1;
		}
		
	}
	else
	{
		/* Unrecognized message */
		printf("[DRONE] This is some kind of weird bee...\n");
	}

/* TODO: Node may also disconnect from the hub
		pthread_mutex_lock(&mtxMaxBees);
		clients--;
		printf("-- %d clients.\n",clients);
		pthread_cond_broadcast(&condMaxBees);
		pthread_mutex_unlock(&mtxMaxBees);
*/
		sleep(1);

close(sock);

return NULL;
}


int main() {
	struct sockaddr_in address, them;
	int sock, sock_them;
	int len;

	printf("[USER] Insert number of flowers:  ");
	gets(s);
	n = atoi(s);

	pthread_t thr;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0) {
		perror("[DRONE] Error creating:");
	}
	printf("[DRONE] Created sock...\n");

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(1026);

	if (bind(sock, (struct sockaddr*) &address, sizeof(struct sockaddr))<0) {
		perror("[DRONE] Error binding:");
	}
	printf("[DRONE] Binded...\n");

	listen(sock, 20);
	
	while(1) {
		len = sizeof(struct sockaddr_in);
		sock_them = accept(sock, (struct sockaddr*) &them, &len);
		printf("[DRONE] Accepted...\n");
		pthread_create(&thr, 0, handleThem, &sock_them);
		sleep(1+rand()%2);
	}

return 0;
}
