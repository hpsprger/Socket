#ifndef __PHY_ETH_COMMON_H__
#define __PHY_ETH_COMMON_H__

#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define PAYLOAD_MAX_LEN 4096  
#define CHOOSE_MAX_LINE 500
#define CONNECT_PORT 7001

#define SERVER_LISTEN_MAX 6000

#define RX_TIMEOUT 1000
#define LINK_FSM_USLEEP 1000

typedef struct _msg_head {
	unsigned short type;
	unsigned short len;
} msg_head;

typedef struct _link_msg {
	msg_head head;
	unsigned char  *payload;
} link_msg;

typedef struct _socket_device {
	int listen_fd;
	int conn_fd;
	int link_status;
} socket_device;

typedef struct _device_ops {
	int (*init)(void);
	int  (*send)(link_msg *pmsg);
	int  (*recv)(link_msg *pmsg, unsigned int timeout);
	int  (*get)(unsigned int type, void *info);
	int  (*set)(unsigned int type, void *info);
	int  (*close)(void);
} device_ops;

#define GET_ENTRY(ptr, type, member) \
    ((type *)( (char *)(ptr) - (unsigned long)(&((type*)0)->member)))

#define DATA_COMM_STR "DATA_COMM"

enum LINK_FSM {
	SYNC_LINK_SETUP = 0,
	SYNC_LINK_START_TX,
	SYNC_LINK_START_RX,
	SYNC_LINK_HIGH_TX,
	SYNC_LINK_HIGH_RX,
	SYNC_LINK_LOW_TX,
	SYNC_LINK_LOW_RX,	
	SYNC_LINK_TASKING,
	SYNC_LINK_STOP,
	SYNC_LINK_MAX,
};

enum MSG_TYPE {
	SYNC_MSG_START = 0,
	SYNC_MSG_HIGH,
	SYNC_MSG_LOW,
	SYNC_MSG_TASKING,
	SYNC_MSG_STOP,
	SYNC_MSG_MAX,
};

enum LINK_STATUS {
	SYNC_LINK_DISCONNECTED = 0,
	SYNC_LINK_CONNECTED,
	SYNC_LINK_STATUS_MAX,
};

enum LINK_INFO {
	SYNC_LINK_INFO_STATUS= 0,
	SYNC_LINK_INFO_MAX,
};

#endif