#ifndef __DATA_TRANS_H__
#define __DATA_TRANS_H__

#include <time.h>
#include "phy_eth_common.h"

#define NORMAL_PRINTF  //printf
#define ERROR_PRRINT   printf

enum DEVICE_TYPE {
	DEVICE_TYPE_TCP_SERVER = 0,
	DEVICE_TYPE_TCP_CLIENT,
	DEVICE_TYPE_MAX,
};

typedef struct _exception_info {
	unsigned int last_fsm;
	unsigned int last_fsm_result;
	unsigned int link_status;
	unsigned int task_count;
	struct timeval time;
} exception_info;

typedef struct _data_trans_device {
	pthread_t tid;
	device_ops *ops;
	exception_info excep_info;
} data_trans_device;

#endif