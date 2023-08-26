#include "phy_eth_client.h"

socket_device client_dev = {0};

int client_init(void);
int client_send_single_message(link_msg *pmsg);
int client_recv_single_message(link_msg *pmsg, unsigned int timeout);
int client_get_socket_info(unsigned int type, void *info);
void client_close(void);

device_ops eth_client_dev_ops = {
	.init = client_init,
	.send = client_send_single_message,
	.recv = client_recv_single_message,
	.get = client_get_socket_info,
	.set = NULL,
	.close = client_close,
};

int client_send_single_message(link_msg *pmsg)
{
	int ret;
	ret = send_single_message(client_dev.conn_fd, pmsg);
	return ret;
}

int client_recv_single_message(link_msg *pmsg, unsigned int timeout)
{
	int ret;
	ret = recv_single_message(client_dev.conn_fd, pmsg, timeout);
	return ret;
}

void client_close(void)
{
	close(client_dev.conn_fd);
}

int client_get_socket_info(unsigned int type, void *info)
{
	if (info == NULL) {
		return -1;
	}

	if (type == SYNC_LINK_INFO_STATUS) {
		return 	*((unsigned int *)info) = GET_ENTRY(&client_dev.conn_fd, socket_device, conn_fd)->link_status;
	}

	return 0;
}

void *client_entry()
{
    struct sockaddr_in serv_addr;
	struct tcp_info info; 
	int len=sizeof(info);
	struct timeval timeout;

	printf("client_entry...\n");

	while (1) {
		client_dev.link_status = SYNC_LINK_DISCONNECTED;
		if((client_dev.conn_fd = socket(AF_INET , SOCK_STREAM , 0)) == -1)
		{
			perror("socket error");
			exit(1);
		}

		bzero(&serv_addr , sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(CONNECT_PORT);
		if(inet_pton(AF_INET , "127.0.0.1" , &serv_addr.sin_addr) < 0)
		{
			printf("inet_pton error\n");
			close(client_dev.conn_fd);
			continue;
		}

		if(connect(client_dev.conn_fd , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0)
		{
			perror("connect error");
			close(client_dev.conn_fd);
			sleep(1);
			continue;
		}

		printf("client connect ok\n");

		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		if (setsockopt(client_dev.conn_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == -1) 
		{
			printf("setsockopt:%08x  SO_RCVTIMEO error\n", client_dev.conn_fd);						
		}
		if (setsockopt(client_dev.conn_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) == -1) 
		{
			printf("setsockopt:%08x  SO_SNDTIMEO error\n", client_dev.conn_fd);						
		}
		client_dev.link_status = SYNC_LINK_CONNECTED;
		while (1) {
			getsockopt(client_dev.conn_fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
			if (info.tcpi_state == TCP_ESTABLISHED) {
				sleep(1);
				continue;
			} else {
				client_dev.link_status = SYNC_LINK_DISCONNECTED;
				close(client_dev.conn_fd);
				break;
			}
		}
	}
}

int client_init(void)
{
	pthread_t tid;

	printf("client_init...\n");

	client_dev.link_status = SYNC_LINK_DISCONNECTED;

	if(pthread_create(&tid , NULL , client_entry, 0) == -1)
	{
		perror("client_entry pthread create error.\n");
		return -1;
	}
	return 0;
}