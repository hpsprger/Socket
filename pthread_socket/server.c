/*
*  服务器端代码实现
*/


#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "config.h"

//声明套接字
int listenfd = -1;
int connfd = -1;
int connfd_new = -1;

#define DATA_MAX_LEN 4096 
#define MAX_LINE 1024

typedef struct _msg {
	unsigned short type;
	unsigned short len;
	unsigned char data[DATA_MAX_LEN];
} msg;

/*处理接收客户端消息函数*/
void *send_message(void)
{
	msg tx_msg;
	memset(&tx_msg , 0 , sizeof(tx_msg));
	tx_msg.type = 0x1234;
	printf("input msg:");
	if (fgets(tx_msg.data, DATA_MAX_LEN, stdin) != NULL) {
		tx_msg.len = strlen(tx_msg.data) + 4;
		if(send(connfd, &tx_msg, tx_msg.len, 0) == -1)
		{
			perror("send error.\n");
			return;
		} else {
			printf("send %d bytes ok!\n", tx_msg.len);
		}
	}
}

char *rx_buffer[DATA_MAX_LEN];
void *recv_message(void)
{
	memset(&rx_buffer, 0, sizeof(rx_buffer));
	int n;
	int cur_len;
	int idx = 0;
	char *cur_pos; 
	if((n = recv(connfd, rx_buffer, DATA_MAX_LEN, 0)) == -1)
	{
		perror("recv error.\n");
		return;
	}
	printf("\nrecv total len:%d\n", n);
	cur_len = n;
	cur_pos = &rx_buffer;
	while (cur_len) {
		printf("msg_idx:%d\n", idx++);
		printf("recv: rcv_msg.type=%d\n", ((msg *)cur_pos)->type);
		printf("recv: rcv_msg.len=%d\n",  ((msg *)cur_pos)->len);
		printf("recv: rcv_msg.data=%s\n", ((msg *)cur_pos)->data);
		cur_len -= ((msg *)cur_pos)->len;
		cur_pos = cur_pos + ((msg *)cur_pos)->len;
	}
}

void *server_entry()
{
	socklen_t clilen;
	pthread_t recv_tid;
	struct timeval timeout;
	struct sockaddr_in servaddr , cliaddr;
	
	if((listenfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
	{
		perror("socket error.\n");
		exit(1);
	}

	bzero(&servaddr , sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	if(bind(listenfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0)
	{
		perror("bind error.\n");
		exit(1);
	}

	if(listen(listenfd , LISTENQ) < 0)
	{
		perror("listen error.\n");
		exit(1);
	}

	while (1) {
		clilen = sizeof(cliaddr);

		if((connfd_new = accept(listenfd , (struct sockaddr *)&cliaddr , &clilen)) < 0)
		{
			perror("accept error.\n");
			exit(1);
		}
		if (connfd != -1) {
			close(connfd);
		}
		connfd = connfd_new;
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == -1) 
		{
			printf("setsockopt:%08x  SO_RCVTIMEO error\n", connfd);						
		}
		if (setsockopt(connfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) == -1) 
		{
			printf("setsockopt:%08x  SO_SNDTIMEO error\n", connfd);						
		}
		printf("server: got connection from %s\n", inet_ntoa(cliaddr.sin_addr));
	}
}

int main()
{
	char msg[MAX_LINE];
	pthread_t tid;

	printf("socket server main...\n");

	if(pthread_create(&tid , NULL , server_entry, 0) == -1)
	{
		perror("pthread create error.\n");
		exit(1);
	}

	while (1) {
		printf("pls choose: 1.send 2.recv 3.exit \n");
		memset(msg , 0 , MAX_LINE);
		if ((fgets(msg , MAX_LINE , stdin)) != NULL) {
			if(strcmp(msg , "1\n") == 0)
			{
				send_message();
			}
			if(strcmp(msg , "2\n") == 0)
			{
				recv_message();
			}
			if(strcmp(msg , "3\n") == 0)
			{
				printf("Client closed.\n");
				close(listenfd);
				exit(1);
			}
		}
	}
	return 0;
}