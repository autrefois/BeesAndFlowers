#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<errno.h>
#include<stdio.h>
#include<string.h>
#include<sys/time.h>
#include<stdlib.h>

struct message
{
    int id;
};

struct message_s // response from server
{
    struct message m;
    char ip[20];
    long port;
};

int main() {
	struct sockaddr_in server_addr;
	int sock;
	int len, i, k;
	struct message msgm;
	struct message_s msg_s;
        char msg[128];

        struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);

	srand(tv.tv_usec);
	msgm.id = rand()%10000;

//while(1)
//{
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0) {
		perror("[BEE] Error creating socket:");
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(1025); // port

	if (connect(sock, (struct sockaddr*) &server_addr, sizeof(struct sockaddr))<0) {
		perror("[BEE] Bind error:");
	}
	
		printf("[BEE] I'm (%d) and I am going in!\n", msgm.id);
		i = send(sock, &msgm, sizeof(struct message), 0);
		if (i<0) perror("[BEE] send :");

		i = recv(sock, &msg_s, sizeof(msg_s), 0);
		if (i<0) perror("[BEE] recv error :");

		printf("[BEE] I'm (%d) and I'm back %s : %d\n", msg_s.m.id, msg_s.ip, msg_s.port);	
	close(sock);
//}

return 0;
}

