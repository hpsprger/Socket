#include "data_trans.h"

extern device_ops eth_server_dev_ops;
extern device_ops eth_client_dev_ops;

device_ops *data_trans_dev_ops[] = {
	&eth_server_dev_ops,
	&eth_client_dev_ops,
	//
};

data_trans_device data_trans;

int get_link_info(unsigned int *link_status)
{
	int ret;
	ret = data_trans.ops->get(SYNC_LINK_INFO_STATUS, link_status);
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
	ret = data_trans.ops->send(&msg);
	return ret;
}

int data_trans_send_msg(link_msg *pmsg)
{
	int ret;
	if (pmsg == NULL) {
		return -1;
	}
	ret = data_trans.ops->send(pmsg);
	return ret;
}

void data_trans_close(void)
{
	int ret;
	ret = data_trans.ops->close();
	return ret;
}


int data_trans_recv_single_msg(link_msg *pmsg, unsigned int timeout)
{
	int ret;
	ret = data_trans.ops->recv(pmsg, timeout);
	return ret;
}

void * sync_fsm_translation()
{
	int ret;
	unsigned int delay;
	int err_count;
	link_msg msg = {0};
	unsigned int link_fsm = SYNC_LINK_SETUP;
	unsigned int link_status = SYNC_LINK_DISCONNECTED;
	unsigned int task_count;
	unsigned char rx_buffer[PAYLOAD_MAX_LEN];
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
		} else {
			printf("SYNC_LINK_SETUP =========0======= nopass SYNC_LINK_DISCONNECTED\n");
		}
		break;

		case SYNC_LINK_START_TX:
		ret = data_trans_send_single_msg(SYNC_MSG_START);
		if (ret < 0) {
			printf("SYNC_LINK_START_TX =========1======= no pass send fail\n");
			err_count++;
			break;
		} else {
			printf("SYNC_LINK_START_TX =========1======= pass \n");
			link_fsm = SYNC_LINK_START_RX;
			err_count = 0;
		}
		break;

		case SYNC_LINK_START_RX:
		msg.payload = rx_buffer;
		ret = data_trans_recv_single_msg(&msg, RX_TIMEOUT);
		if (ret < 0) {
			printf("SYNC_LINK_START_RX =========2======= nopass recv fail\n");
			err_count++;
			break;
		}

		if (msg.head.type == SYNC_MSG_START) {
			printf("SYNC_LINK_START_RX =========2======= pass \n");
			link_fsm = SYNC_LINK_HIGH_TX;
			err_count = 0;
		} else {
			printf("SYNC_LINK_START_RX =========2======= nopass tyoe mis \n");
			err_count++;
		}
		break;

		case SYNC_LINK_HIGH_TX:
		ret = data_trans_send_single_msg(SYNC_MSG_HIGH);
		if (ret < 0) {
			printf("SYNC_LINK_HIGH_TX =========3======= nopass send fail\n");
			err_count++;
			break;
		} else {
			printf("SYNC_LINK_HIGH_TX =========3======= pass \n");
			link_fsm = SYNC_LINK_HIGH_RX;
			err_count = 0;
		}
		break;
	
		case SYNC_LINK_HIGH_RX:
		msg.payload = rx_buffer;
		ret = data_trans_recv_single_msg(&msg, RX_TIMEOUT);
		if (ret < 0) {
			printf("SYNC_LINK_HIGH_RX =========4======= nopass recv fail\n");
			err_count++;
			break;
		}

		if (msg.head.type == SYNC_MSG_HIGH) {
			printf("SYNC_LINK_HIGH_RX =========4======= pass \n");
			link_fsm = SYNC_LINK_LOW_TX;
			err_count = 0;
		} else {
			printf("SYNC_LINK_HIGH_RX =========4======= nopass tyoe mis\n");
			err_count++;
		}
		break;


		case SYNC_LINK_LOW_TX:
		ret = data_trans_send_single_msg(SYNC_MSG_LOW);
		if (ret < 0) {
			printf("SYNC_LINK_LOW_TX =========5======= nopass send fail\n");
			err_count++;
			break;
		} else {
			printf("SYNC_LINK_LOW_TX =========5======= pass \n");
			link_fsm = SYNC_LINK_LOW_RX;
			err_count = 0;
		}
		break;

		case SYNC_LINK_LOW_RX:
		msg.payload = rx_buffer;
		ret = data_trans_recv_single_msg(&msg, RX_TIMEOUT);
		if (ret < 0) {
			printf("SYNC_LINK_LOW_RX =========6======= nopass recv fail\n");
			err_count++;
			break;
		}

		if (msg.head.type == SYNC_MSG_LOW) {
			printf("SYNC_LINK_LOW_RX =========6======= pass \n");
			link_fsm = SYNC_LINK_TASKING;
			err_count = 0;
		} else {
			printf("SYNC_LINK_LOW_RX =========6======= nopass tyoe mis\n");
			err_count++;
		}
		break;

		case SYNC_LINK_TASKING:
		delay = rand()%10 * 150;

		clock_gettime(0, &time);
		sec = (unsigned long long int)time.tv_sec;
		usec = (unsigned long long int)time.tv_usec;
		//printf("SYNC_LINK_TASKING =====7===delay:%d====task_count=%d(time:0x%lld)===== \n", delay, task_count++, sec*1000000 + usec/1000);
		printf("SYNC_LINK_TASKING =====7===delay:%dus===task_count=%d==time:sec:%lld--usec:%lld===== \n", delay, task_count++, sec, usec);
		usleep(delay);
		printf("SYNC_LINK_TASKING ========7==done======= \n", task_count);
		link_fsm = SYNC_LINK_START_TX;
		err_count = 0;

		if (task_count % 2000 == 0) {
			sleep(5); //观察用
		}

		break;

		case SYNC_LINK_STOP:
		printf("SYNC_LINK_STOP ========8======== \n");
		sleep(1);
		break;

		default:
		break;
		}
		if (err_count > 5) {
			get_link_info(&link_status);
			data_trans.excep_info.link_status = link_status;
			data_trans.excep_info.last_fsm = link_fsm;
			data_trans.excep_info.last_fsm_result = ret;
			data_trans.excep_info.task_count = task_count;
			clock_gettime(0, &data_trans.excep_info.time);
			link_fsm = SYNC_LINK_STOP;
		}
		usleep(LINK_FSM_USLEEP);
	}

}

int data_trans_init(unsigned int type)
{
	int ret;

	if (type > DEVICE_TYPE_MAX) {
		return -1;
	}

	srand((unsigned)time(NULL)); //保证随机数的随机性

	data_trans.ops  = data_trans_dev_ops[type];

	ret = data_trans.ops->init();
	if(ret < 0)
	{
		perror("init failed.\n");
		return -1;
	}

	if(pthread_create(&data_trans.tid , NULL , sync_fsm_translation, 0) == -1)
	{
		perror("pthread create error.\n");
		return -1;
	}

	return 0;
}
