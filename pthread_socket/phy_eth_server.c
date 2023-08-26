#include "phy_eth_server.h"

socket_device server_dev = {0};

int server_init(void);
int server_send_single_message(link_msg *pmsg);
int server_recv_single_message(link_msg *pmsg, unsigned int timeout);
int server_get_socket_info(unsigned int type, void *info);
void server_close(void);

device_ops eth_server_dev_ops = {
	.init = server_init,
	.send = server_send_single_message,
	.recv = server_recv_single_message,
	.get = server_get_socket_info,
	.set = NULL,
	.close = server_close,
};

int server_send_single_message(link_msg *pmsg)
{
	int ret;
	ret = send_single_message(server_dev.conn_fd, pmsg);
	return ret;
}

int server_recv_single_message(link_msg *pmsg, unsigned int timeout)
{
	int ret;
	ret = recv_single_message(server_dev.conn_fd, pmsg, timeout);
	return ret;
}

int server_get_socket_info(unsigned int type, void *info)
{
	if (info == NULL) {
		return -1;
	}

	if (type == SYNC_LINK_INFO_STATUS) {
		return 	*((unsigned int *)info) = GET_ENTRY(&server_dev.conn_fd, socket_device, conn_fd)->link_status;
	}

	return 0;
}

void server_close(void)
{
	close(server_dev.conn_fd);
	close(server_dev.listen_fd);
}

void *server_entry()
{
	int connfd_new;
	socklen_t client_len;
	pthread_t recv_tid;
	struct timeval timeout;
	struct sockaddr_in serv_addr , client_addr;
	
	printf("server_entry...\n");

	if((server_dev.listen_fd = socket(AF_INET , SOCK_STREAM , 0)) == -1) {
		perror("socket error.\n");
		exit(1);
	}

	bzero(&serv_addr , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(CONNECT_PORT);

	if(bind(server_dev.listen_fd , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0) {
		perror("bind error.\n");
		exit(1);
	}

	if(listen(server_dev.listen_fd , SERVER_LISTEN_MAX) < 0) {
		perror("listen error.\n");
		exit(1);
	}

	while (1) {
		client_len = sizeof(client_addr);
		if((connfd_new = accept(server_dev.listen_fd , (struct sockaddr *)&client_addr , &client_len)) < 0) {
			perror("accept error.\n");
			exit(1);
		}
		if (server_dev.conn_fd != -1) {
			close(server_dev.conn_fd);
		}
		server_dev.conn_fd = connfd_new;
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
		if (setsockopt(server_dev.conn_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == -1) {
			printf("setsockopt:%08x  SO_RCVTIMEO error\n", server_dev.conn_fd);						
		}
		if (setsockopt(server_dev.conn_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) == -1) {
			printf("setsockopt:%08x  SO_SNDTIMEO error\n", server_dev.conn_fd);						
		}
		server_dev.link_status = SYNC_LINK_CONNECTED;
		printf("server: got connection from %s\n", inet_ntoa(client_addr.sin_addr));
	}
}

int server_init(void)
{
	pthread_t tid;

	printf("server_init...\n");

	server_dev.link_status = SYNC_LINK_DISCONNECTED;

	if(pthread_create(&tid , NULL , server_entry, 0) == -1)
	{
		perror("server_entry pthread create error.\n");
		return -1;
	}
	return 0;
}