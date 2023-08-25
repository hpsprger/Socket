
#include <stdio.h>
#include "data_trans.h"

char send_text[PAYLOAD_MAX_LEN];
char rcv_text[PAYLOAD_MAX_LEN];

// ./main 0 ==> server
// ./main 1 ==> client
void main(int argc, char* argv[])
{
	char choose[CHOOSE_MAX_LINE];
	pthread_t tid;
	link_msg tx_msg = {0};
	link_msg rx_msg = {0};	
	int ret;

    if(argc < 2)
    {
        printf("usage: ./main 1 \n");
        exit(1);
    }

    printf("main --> argv[0]=%s  argv[1]=%d  argc=%d \n", argv[0], atoi(argv[1]), argc);

	data_trans_init(atoi(argv[1]));

	while (1) {
		printf("pls choose: 1.send 2.recv 3.exit \n");
		memset(choose , 0 , CHOOSE_MAX_LINE);
		if ((fgets(choose , CHOOSE_MAX_LINE , stdin)) != NULL) {
			if(strcmp(choose , "1\n") == 0)
			{
				printf("input msg: \n");
				memset(send_text , 0 , PAYLOAD_MAX_LEN);
				fgets(send_text , PAYLOAD_MAX_LEN , stdin);
				tx_msg.head.type = 0x1234;
				tx_msg.head.len = strlen(send_text);
				tx_msg.payload = send_text;
				if (data_trans_send_msg(&tx_msg) < 0) {
					printf("send_message fail \n");
				} else {
					printf("send_message ok \n");
				}
			}
			if(strcmp(choose , "2\n") == 0)
			{
				memset(rcv_text , 0 , PAYLOAD_MAX_LEN);
				rx_msg.payload = rcv_text;
				ret = data_trans_recv_single_msg(&rx_msg, 1000);
				printf("recv:%s \n", (ret == 0)?"ok":"fail");
				printf("rx msg:type(0x%x) \n", rx_msg.head.type);
				printf("rx msg:len(%d) \n", rx_msg.head.len);
				printf("rx msg:buffer==>%s\n", rx_msg.payload);
			}
			if(strcmp(choose , "3\n") == 0)
			{
				printf("data_trans_close...\n");
				data_trans_close();
				exit(1);
			}
		}
	}
	return 0;
}