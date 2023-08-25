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

int get_link_info(unsigned int *link_status)
{
	int ret;
	ret = data_trans_ops->get(SYNC_LINK_INFO_STATUS, link_status);
	return ret;
}

int data_trans_send_single_msg(unsigned int msg_type)
{
	int ret;
	link_msg msg;
	if (msg_type >= SYNC_MSG_MAX) {
		return -1;
	}
	msg.head.type = msg_type;
	msg.head.len = strlen(DATA_COMM_STR);
	msg.payload = DATA_COMM_STR;
	ret = data_trans_ops->send(&msg);
	return ret;
}

int data_trans_send_msg(link_msg *pmsg)
{
	int ret;
	if (pmsg == NULL) {
		return -1;
	}
	ret = data_trans_ops->send(pmsg);
	return ret;
}

void data_trans_close(void)
{
	int ret;
	ret = data_trans_ops->close();
	return ret;
}


int data_trans_recv_single_msg(link_msg *pmsg, unsigned int timeout)
{
	int ret;
	ret = data_trans_ops->recv(pmsg, timeout);
	return ret;
}

void * sync_fsm_translation()
{
	int ret;
	unsigned int delay;
	int err_count;
	link_msg msg = {0};
	unsigned char rx_buffer[PAYLOAD_MAX_LEN];
	unsigned int link_fsm = SYNC_LINK_SETUP;
	unsigned int link_status = SYNC_LINK_DISCONNECTED;
	unsigned int task_count;

	struct timeval time = { 
			.tv_sec=0, /*单位：s*/
			.tv_usec=0 /*单位：ns*/
	};

	unsigned long long int sec,usec = 0;

	printf(" =========fsm_translation======= \n");

	while (1) {
		switch(link_fsm) {
		case SYNC_LINK_SETUP:
		get_link_info(&link_status);
		if (link_status == SYNC_LINK_CONNECTED) {
			printf("SYNC_LINK_SETUP =========0======= pass\n");
			link_fsm = SYNC_LINK_START_TX;
		}
		break;

		case SYNC_LINK_START_TX:
		ret = data_trans_send_single_msg(SYNC_MSG_START);
		if (ret < 0) {
			printf("SYNC_LINK_SETUP =========1======= send fail\n");
			err_count++;
			break;
		} else {
			printf("SYNC_LINK_SETUP =========1======= send ok\n");
			link_fsm = SYNC_LINK_START_RX;
			err_count = 0;
		}
		break;

		case SYNC_LINK_START_RX:
		msg.payload = rx_buffer;
		ret = data_trans_recv_single_msg(&msg, RX_TIMEOUT);
		if (ret < 0) {
			printf("SYNC_LINK_START_RX =========2======= recv fail\n");
			err_count++;
			break;
		}

		if (msg.type == SYNC_MSG_START) {
			printf("SYNC_LINK_START_RX =========2======= pass \n");
			link_fsm = SYNC_LINK_HIGH_TX;
			err_count = 0;
		} else {
			err_count++;
		}
		break;

		case SYNC_LINK_HIGH_TX:
		ret = data_trans_send_single_msg(SYNC_MSG_HIGH);
		if (ret < 0) {
			printf("SYNC_LINK_HIGH_TX =========3======= send fail\n");
			err_count++;
			break;
		} else {
			printf("SYNC_LINK_HIGH_TX =========3======= send ok\n");
			link_fsm = SYNC_LINK_START_RX;
			err_count = 0;
		}
		break;
	
		case SYNC_LINK_HIGH_RX:
		msg.payload = rx_buffer;
		ret = data_trans_recv_single_msg(&msg, RX_TIMEOUT);
		if (ret < 0) {
			printf("SYNC_LINK_HIGH_RX =========4======= recv fail\n");
			err_count++;
			break;
		}

		if (msg.type == SYNC_MSG_HIGH) {
			printf("SYNC_LINK_HIGH_RX =========4======= pass \n");
			link_fsm = SYNC_LINK_LOW_TX;
			err_count = 0;
		} else {
			err_count++;
		}
		break;


		case SYNC_LINK_LOW_TX:
		ret = data_trans_send_single_msg(SYNC_MSG_LOW);
		if (ret < 0) {
			printf("SYNC_LINK_LOW_TX =========5======= send fail\n");
			err_count++;
			break;
		} else {
			printf("SYNC_LINK_LOW_TX =========5======= send ok\n");
			link_fsm = SYNC_LINK_LOW_RX;
			err_count = 0;
		}
		break;

		case SYNC_LINK_LOW_RX:
		msg.payload = rx_buffer;
		ret = data_trans_recv_single_msg(&msg, RX_TIMEOUT);
		if (ret < 0) {
			printf("SYNC_LINK_LOW_RX =========6======= recv fail\n");
			err_count++;
			break;
		}

		if (msg.type == SYNC_MSG_LOW) {
			printf("SYNC_LINK_LOW_RX =========6======= pass \n");
			link_fsm = SYNC_MSG_TASKING;
			err_count = 0;
		} else {
			err_count++;
		}
		break;

		case SYNC_LINK_TASKING:
		delay = rand()%10;

		clock_gettime(0, &time);
		sec = (unsigned long long int)time.tv_sec;
		usec = (unsigned long long int)time.tv_usec;

		printf("SYNC_LINK_TASKING ========task_count=%d(time:0x%lld)======== \n", task_count++, sec*1000000 + usec/1000);
		sleep(delay);
		printf("SYNC_LINK_TASKING ========done======= \n", task_count);
		link_fsm = SYNC_LINK_START_TX;
		err_count = 0;
		break;

		case SYNC_LINK_STOP:
		printf("SYNC_LINK_STOP ================ \n");
		sleep(1);
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

	srand((unsigned)time(NULL)); //保证随机数的随机性

	data_trans_ops = data_trans_dev_ops[type];

	if(pthread_create(&tid , NULL , sync_fsm_translation, 0) == -1)
	{
		perror("pthread create error.\n");
		return -1;
	}
	return 0;
}
