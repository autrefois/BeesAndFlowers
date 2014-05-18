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

struct message
{
    int id;
};

struct message_s
{
    struct message m;
    char ip[20];
    long port;
};

int n;
int bees = 0;
int sock, sock_client;

struct message_s msg_s;

pthread_mutex_t mtx_msg = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mtxN = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condN = PTHREAD_COND_INITIALIZER;

void* handleClient(void* arg) 
{
	int sock = *((int*) arg);
	struct sockaddr_in address; // bee (client)
	int i, len;
	struct message msgm;

	len = sizeof(address);
	getpeername(sock, (struct sockaddr*)&address, &len);
	// bee (client structure)
	
		pthread_mutex_lock(&mtxN);	
		while(bees >= n)
		{
		        printf("[EAGER BEE] Me civilized, me waits!\n");			
			pthread_cond_wait(&condN, &mtxN);
		}
		bees++;
		printf("[DRONE] ++ %d bees.\n",bees);	
		pthread_cond_broadcast(&condN);
		pthread_mutex_unlock(&mtxN);	
		i = recv(sock, &msgm, sizeof(struct message), 0);
		if (i < 0) 
		{
			perror("[DRONE] recv error - ");
			return NULL;
		}

		printf("[DRONE] Bee(%d) is on a flower\n", msgm.id);

		pthread_mutex_lock(&mtx_msg);

		// Collect polen
		sleep(2);

		strcpy(msg_s.ip,inet_ntoa(address.sin_addr)); 
		msg_s.port = ntohs(address.sin_port);
		msg_s.m.id = msgm.id;

       	 	i = send(sock, &msg_s, sizeof(struct message_s), 0);
		
		if (i<0) 
		{
			perror("[DRONE] send error:"); 
		}
		pthread_mutex_unlock(&mtx_msg);
		pthread_mutex_lock(&mtxN);
		bees--;
		printf("[DRONE] Bee(%d) flies away...\n", msgm.id);
		printf("[DRONE] -- %d bees.\n", bees);
		pthread_cond_broadcast(&condN);
		pthread_mutex_unlock(&mtxN);
		sleep(1);

close(sock);

return NULL;
}

int main() {

	//sleep(5);
	struct sockaddr_in client, address;
	int sock, sock_client;
	int len, i;
	//struct message msg;
	pthread_t thr;
	char s[20];
	
	printf("[USER] Insert number of flowers:  ");
	gets(s);
	n = atoi(s);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0) {
		perror("[DRONE] Error creating socket:");
		return 1;
	}


	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(1025);

	if (bind(sock, (struct sockaddr*) &address, sizeof(struct sockaddr))<0) {
		perror("[DRONE] Bind error:");
		return 1;
	}

	if (listen(sock, 5) < 0) 
	{
		perror("[DRONE] Listen ");
		return 1;		
	}
	
	while(1) {
		len = sizeof(struct sockaddr_in);
		sock_client = accept(sock, (struct sockaddr*) &client, &len);

    		pthread_create(&thr, 0, handleClient, &sock_client);
		
	}
exit(0);
return 0;
}
