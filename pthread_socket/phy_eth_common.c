#include "phy_eth_common.h"

int send_single_message(int sockfd, link_msg *pmsg)
{
	struct msghdr msg = {0};
	msg_head head = {0};
	struct iovec io[2] = {0};

	if (pmsg == NULL) {
		return -1;
	}

	head.len = pmsg->head.len;
	head.type = pmsg->head.type;

	io[0].iov_base = &head;
	io[0].iov_len = sizeof(head);

	io[1].iov_base = pmsg->payload;
	io[1].iov_len = pmsg->head.len;

	msg.msg_iov = &io[0];
	msg.msg_iovlen = 2;
	msg.msg_control = 0;
	msg.msg_controllen = 0;

	if(sendmsg(sockfd, &msg, 0) < 0) {
		perror("send error.\n");
		return -1;
	}
	return 0;
}

//TCP是基于连接的串行数据流，你可以从连接的管道中，通过len控制每次从管道里面读取多少个字节的数据，
//没有读完的数据，不会丢弃，继续保留在管道中，TCP是天然保序的
//你可以首先读出一个固定长度的msg_head，msg_head中的len是用户协议约定好的payload数据的长度，所以读出了
//msg_head中的len，然后再从管道中读出msg_head.len的payload数据，就可以读出一个完整的可变长度的包来了
//所以下面的函数中用一个io就行了
int recv_single_message(int sockfd, link_msg *pmsg, unsigned int timeout)
{
	int ret;
	int flag;
	struct msghdr msg = {0};
	struct iovec io[1] = {0};
	struct timeval time;
	msg_head head = {0};

	if (pmsg == NULL)
	{
		perror("recv_message pmsg null\n");
		return -1;
	}

	if (timeout == 0) {
		flag = MSG_DONTWAIT;
	} else {
		flag = MSG_WAITALL;
		time.tv_sec = 0;
		time.tv_usec = timeout;
		if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&time, sizeof(time)) == -1) {
			perror("recv_message setsockopt:%08x  SO_RCVTIMEO error\n", sockfd);			
		}
	}

	io[0].iov_base = &head;
	io[0].iov_len = sizeof(head);
	msg.msg_iov = &io[0];
	msg.msg_iovlen = 1;
	msg.msg_control = 0;
	msg.msg_controllen = 0;
	ret = recvmsg(sockfd, &msg, flag);
	if(ret < 0) {
		perror("recv error.\n");
		return -1;
	}

	io[0].iov_base = pmsg->payload;
	io[0].iov_len = head.len;
	msg.msg_iov = &io[0];
	msg.msg_iovlen = 1;
	msg.msg_control = 0;
	msg.msg_controllen = 0;
	ret = recvmsg(sockfd, &msg, flag);
	if(ret < 0) {
		perror("recv error.\n");
		return -1;
	}

	pmsg->head.type = head.type;
	pmsg->head.len = head.len;

	return 0;
}