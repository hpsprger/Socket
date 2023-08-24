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


int connfd;

#define DATA_MAX_LEN 20 
#define CHOOSE_MAX_LINE 50

typedef struct _super_msg {
	unsigned short type;
	unsigned short len;
	unsigned char  *buffer;
} super_msg;

typedef struct _ctrl_msg {
	unsigned short type;
	unsigned short len;
} ctrl_msg;

/*处理接收客户端消息函数*/
int send_message(super_msg *pmsg)
{
	struct msghdr tx_msg = {0};
	ctrl_msg cmsg = {0};
	struct iovec io[2] = {0};

	if (pmsg == NULL) {
		return -1;
	}

	cmsg.len = pmsg->len;
	cmsg.type = pmsg->type;

	io[0].iov_base = &cmsg;
	io[0].iov_len = sizeof(cmsg);

	io[1].iov_base = pmsg->buffer;
	io[1].iov_len = pmsg->len;

	tx_msg.msg_iov = &io[0];
	tx_msg.msg_iovlen = 2;
	tx_msg.msg_control = 0;
	tx_msg.msg_controllen = 0;

	if(sendmsg(connfd, &tx_msg, 0) < 0) {
		perror("send error.\n");
		return -1;
	}
	return 0;
}

/* 返回接收的总长度 */
int recv_message(super_msg *pmsg, unsigned int timeout)
{
	int ret;
	int flag;
	struct msghdr rx_msg = {0};
	struct iovec io[2] = {0};
	struct timeval time;
	ctrl_msg cmsg = {0};

	if (pmsg == NULL)
	{
		perror("recv_message pmsg null\n");
		return -1;
	}

	io[0].iov_base = &cmsg;
	io[0].iov_len = sizeof(cmsg);

	io[1].iov_base = pmsg->buffer;
	io[1].iov_len = pmsg->len;

	rx_msg.msg_iov = &io[0];
	rx_msg.msg_iovlen = 2;
	rx_msg.msg_control = 0;
	rx_msg.msg_controllen = 0;

	if (timeout == 0) {
		flag = MSG_DONTWAIT;
	} else {
		flag = MSG_WAITALL;
		time.tv_sec = 0;
		time.tv_usec = timeout;
		if (setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&time, sizeof(time)) == -1) {
			printf("recv_message setsockopt:%08x  SO_RCVTIMEO error\n", connfd);			
		}
	}

	ret = recvmsg(connfd, &rx_msg, flag);
	if(ret < 0) {
		pmsg->type = 0;
		pmsg->len = 0;
		perror("recv error.\n");
		return -1;
	}

	pmsg->type = cmsg.type;
	pmsg->len = cmsg.len;

	return ret;
}

unsigned int link_status = 0;

void *client_entry()
{
    struct sockaddr_in servaddr;
	struct tcp_info info; 
	int len=sizeof(info);
	struct timeval timeout;

	while (1) {
		link_status = 0;
		if((connfd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
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
			close(connfd);
			continue;
		}

		if( connect(connfd , (struct sockaddr *)&servaddr , sizeof(servaddr)) < 0)
		{
			perror("connect error");
			close(connfd);
			sleep(1);
			continue;
		}

		printf("connect ok\n");

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
		link_status = 1;
		while (1) {
			getsockopt(connfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
			if (info.tcpi_state == TCP_ESTABLISHED) {
				sleep(1);
				continue;
			} else {
				link_status = 0;
				close(connfd);
				break;
			}
		}
	}
}

char rcv_buf[DATA_MAX_LEN];

#define STATUS_0 0x010
#define STATUS_1 0x001
#define STATUS_2 0x002
#define STATUS_3 0x003
#define STATUS_4 0x004
#define STATUS_5 0x005
#define STATUS_DO_WORKING 0x006

void * fsm_translation()
{
	int ret;
	unsigned int delay;
	int err_count;
	super_msg tx_msg = {0};
	super_msg rx_msg = {0};	
	unsigned int fsm = STATUS_0;
	
	printf(" =========fsm_translation======= \n");

	while (1) {
		switch(fsm) {
		case STATUS_0:
		if (link_status == 1) {
			printf("fsm STATUS_0 =========pass======= \n");
			fsm = STATUS_1;
		}
		err_count = 0;
		break;
		case STATUS_1:
		tx_msg.type = STATUS_1;
		tx_msg.len = strlen("STATUS_1");
		tx_msg.buffer = "STATUS_1";
		ret = send_message(&tx_msg);
		if (ret < 0) {
			printf("fsm send_message err\n");
			err_count++;
			break;
		}

		rx_msg.len = DATA_MAX_LEN;
		rx_msg.buffer = rcv_buf;
		ret = recv_message(&rx_msg, 1000);
		if (ret < 0) {
			printf("fsm recv_message err\n");
			break;
		}

		if (rx_msg.type == STATUS_1) {
			printf("fsm STATUS_1 =========pass=======\n");
			fsm = STATUS_2;
		} else {
			err_count++;
		}
		break;
		case STATUS_2:
		tx_msg.type = STATUS_2;
		tx_msg.len = strlen("STATUS_2");
		tx_msg.buffer = "STATUS_2";
		ret = send_message(&tx_msg);
		if (ret < 0) {
			printf("fsm send_message err\n");
			err_count++;
			break;
		}

		rx_msg.len = DATA_MAX_LEN;
		rx_msg.buffer = rcv_buf;
		ret = recv_message(&rx_msg, 1000);
		if (ret < 0) {
			printf("fsm recv_message err\n");
			break;
		}

		if (rx_msg.type == STATUS_2) {
			printf("fsm STATUS_2 =========pass======= \n");
			fsm = STATUS_3;
		} else {
			err_count++;
		}
		break;
		case STATUS_3:
		tx_msg.type = STATUS_3;
		tx_msg.len = strlen("STATUS_3");
		tx_msg.buffer = "STATUS_3";
		ret = send_message(&tx_msg);
		if (ret < 0) {
			printf("fsm send_message err\n");
			err_count++;
			break;
		}

		rx_msg.len = DATA_MAX_LEN;
		rx_msg.buffer = rcv_buf;
		ret = recv_message(&rx_msg, 1000);
		if (ret < 0) {
			printf("fsm recv_message err\n");
			break;
		}

		if (rx_msg.type == STATUS_3) {
			printf("fsm STATUS_3 =========pass======= \n");
			fsm = STATUS_DO_WORKING;
		} else {
			err_count++;
		}
		break;
		case STATUS_DO_WORKING:
		srand( (unsigned)time(NULL));
		delay = rand()%20;
		printf("fsm STATUS_DO_WORKING ========delay=%d======== \n", delay);
		sleep(delay);
		fsm = STATUS_0;
		break;
		default:
		break;
		}
		if (err_count > 5) {
			fsm = STATUS_0;
		}
		sleep(1);
	}
}



char send_text[DATA_MAX_LEN];
char rcv_text[DATA_MAX_LEN];

int main(int argc , char **argv)
{
	char choose[CHOOSE_MAX_LINE];
	pthread_t tid;
	super_msg tx_msg = {0};
	super_msg rx_msg = {0};	
	int ret;

	if(pthread_create(&tid , NULL , client_entry, 0) == -1)
	{
		perror("pthread create error.\n");
		exit(1);
	}

	if(pthread_create(&tid , NULL , fsm_translation, 0) == -1)
	{
		perror("pthread create error.\n");
		exit(1);
	}

	while (1) {
		printf("pls choose: 1.send 2.recv 3.exit \n");
		memset(choose , 0 , CHOOSE_MAX_LINE);
		if ((fgets(choose , CHOOSE_MAX_LINE , stdin)) != NULL) {
			if(strcmp(choose , "1\n") == 0)
			{
				printf("input msg: \n");
				memset(send_text , 0 , DATA_MAX_LEN);
				fgets(send_text , DATA_MAX_LEN , stdin);
				tx_msg.type = 0x1234;
				tx_msg.len = strlen(send_text);
				tx_msg.buffer = send_text;
				if (send_message(&tx_msg) < 0) {
					printf("send_message fail \n");
				} else {
					printf("send_message ok \n");
				}
			}
			if(strcmp(choose , "2\n") == 0)
			{
				memset(rcv_text , 0 , DATA_MAX_LEN);
				rx_msg.buffer = rcv_text;
				rx_msg.len = DATA_MAX_LEN;
				ret = recv_message(&rx_msg, 1000);
				printf("recv total len:%d \n", ret);
				printf("rx msg:type(0x%x) \n", rx_msg.type);
				printf("rx msg:len(%d) \n", rx_msg.len);
				printf("rx msg:buffer==>%s\n", rx_msg.buffer);
			}
			if(strcmp(choose , "3\n") == 0)
			{
				printf("Client closed.\n");
				close(connfd);
				exit(1);
			}
		}
	}
}
