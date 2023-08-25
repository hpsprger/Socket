#include "common.h"

int listenfd = -1;
int connfd = -1;
int connfd_new = -1;
unsigned int link_status = 0;

char rcv_buf[DATA_MAX_LEN];
char send_text[DATA_MAX_LEN];
char rcv_text[DATA_MAX_LEN];

socket_device_ops data_trans_dev_ops[] = {
	eth_server_dev_ops,
	eth_client_dev_ops,
	//
};

socket_device_ops *data_trans_ops = 0;

void * sync_fsm_translation()
{
	int ret;
	unsigned int delay;
	int err_count;
	super_msg tx_msg = {0};
	super_msg rx_msg = {0};	
	unsigned int link_fsm = SYNC_LINK_SETUP;

	printf(" =========fsm_translation======= \n");

	while (1) {
		switch(link_fsm) {
		case SYNC_LINK_SETUP:
		if (link_status == 1) {
			printf("link_fsm STATUS_0 =========pass=======\n");
			link_fsm = STATUS_1;
		}
		err_count = 0;
		break;
		case STATUS_1:
		tx_msg.type = STATUS_1;
		tx_msg.len = strlen("STATUS_1");
		tx_msg.buffer = "STATUS_1";
		ret = send_message(&tx_msg);
		if (ret < 0) {
			printf("link_fsm send_message err\n");
			err_count++;
			break;
		}

		rx_msg.len = DATA_MAX_LEN;
		rx_msg.buffer = rcv_buf;
		ret = recv_message(&rx_msg, 1000);
		if (ret < 0) {
			printf("link_fsm recv_message err\n");
			break;
		}

		if (rx_msg.type == STATUS_1) {
			printf("link_fsm STATUS_1 =========pass=======\n");
			link_fsm = STATUS_2;
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
			printf("link_fsm send_message err\n");
			err_count++;
			break;
		}

		rx_msg.len = DATA_MAX_LEN;
		rx_msg.buffer = rcv_buf;
		ret = recv_message(&rx_msg, 1000);
		if (ret < 0) {
			printf("link_fsm recv_message err\n");
			break;
		}

		if (rx_msg.type == STATUS_2) {
			printf("link_fsm STATUS_2 =========pass=======\n");
			link_fsm = STATUS_3;
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
			printf("link_fsm send_message err\n");
			err_count++;
			break;
		}

		rx_msg.len = DATA_MAX_LEN;
		rx_msg.buffer = rcv_buf;
		ret = recv_message(&rx_msg, 1000);
		if (ret < 0) {
			printf("link_fsm recv_message err\n");
			break;
		}

		if (rx_msg.type == STATUS_3) {
			printf("link_fsm STATUS_3 =========pass=======\n");
			link_fsm = STATUS_DO_WORKING;
		} else {
			err_count++;
		}
		break;
		case STATUS_DO_WORKING:
		srand( (unsigned)time(NULL));
		delay = rand()%20;
		printf("link_fsm STATUS_DO_WORKING ========delay=%d======== \n", delay);
		sleep(delay);
		fsm = STATUS_0;
		break;
		default:
		break;
		}
		if (err_count > 5) {
			fsm = STATUS_0;
		}
		usleep(LINK_FSM_USLEEP);
	}
}

int data_trans_init(unsigned int type)
{
	if (type > DEVICE_TYPE_MAX) {
		return -1;
	}
	data_trans_ops = data_trans_dev_ops[type];

	if(pthread_create(&tid , NULL , sync_fsm_translation, 0) == -1)
	{
		perror("pthread create error.\n");
		return -1;
	}
	return 0;
}
