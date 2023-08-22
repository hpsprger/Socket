/*
* 客户端代码
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


int sockfd;

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
	if (fgets(&tx_msg.data , DATA_MAX_LEN , stdin) != NULL) {
		tx_msg.len = strlen(tx_msg.data) + 4;
		if(send(sockfd, &tx_msg, tx_msg.len, 0) == -1)
		{
			perror("send error.\n");
			return;
		} else {
			printf("send %d bytes ok!\n", tx_msg.len);
		}
	}
}

void *recv_message(void)
{
	msg rcv_msg;
	memset(&rcv_msg, 0, sizeof(rcv_msg));
	int n;

	if((n = recv(sockfd, &rcv_msg, DATA_MAX_LEN, 0)) == -1)
	{
		perror("recv error.\n");
		return;
	}
	printf("\nrec: n=%d\n", n);
	printf("recv: rcv_msg.type=%d\n", rcv_msg.type);
	printf("recv: rcv_msg.len=%d\n",  rcv_msg.len);
	printf("recv: rcv_msg.data=%s\n", rcv_msg.data);
}

void *client_entry()
{
    struct sockaddr_in servaddr;
	struct tcp_info info; 
	int len=sizeof(info);

	while (1) {
		if((sockfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
		{
			perror("socket error");
			exit(1);
		}

		bzero(&servaddr , sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(PORT);
		if(inet_pton(AF_INET , "127.0.0.1" , &servaddr.sin_addr) < 0)
		{
			printf("inet_pton error\n");
			close(sockfd);
			continue;
		}

		if( connect(sockfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0)
		{
			perror("connect error");
			close(sockfd);
			continue;
		}

		while (1) {
			getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
			if (info.tcpi_state == TCP_ESTABLISHED) {
				sleep(1);
				continue;
			} else {
				break;
			}
		}
	}
}

int main(int argc , char **argv)
{
	char msg[MAX_LINE];
	pthread_t tid;

	if(pthread_create(&tid , NULL , client_entry, 0) == -1)
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
				close(sockfd);
				exit(1);
			}
		}
	}
}
