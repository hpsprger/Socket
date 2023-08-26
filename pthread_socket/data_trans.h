#ifndef __DATA_TRANS_H__
#define __DATA_TRANS_H__

#include "phy_eth_common.h"

enum DEVICE_TYPE {
	DEVICE_TYPE_TCP_SERVER = 0,
	DEVICE_TYPE_TCP_CLIENT,
	DEVICE_TYPE_MAX,
};

typedef struct _data_trans_device {
	pthread_t tid;
	device_ops *ops;
} data_trans_device;

#endif