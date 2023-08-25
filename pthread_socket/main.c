#include "common.h"

char send_text[DATA_MAX_LEN];
char rcv_text[DATA_MAX_LEN];

// ./main 0 ==> server
// ./main 1 ==> client
void main(int argc, char* argv[])
{
{
	char choose[CHOOSE_MAX_LINE];
	pthread_t tid;
	super_msg tx_msg = {0};
	super_msg rx_msg = {0};	
	int ret;

    if(argc < 2)
    {
        printf("usage: ./main 1 \n");
        exit(1);
    }

    printf("main --> argv[0]=%d  argv[1]=%d  argc=%d \n", atoi(argv[0]), atoi(argv[1]), argc);

	data_trans_init(atoi(argv[1]));

	while (1) {
		printf("pls choose: 1.send 2.recv 3.exit \n");
		memset(choose , 0 , CHOOSE_MAX_LINE);
		if ((fgets(choose , CHOOSE_MAX_LINE , stdin)) != NULL) {
			if(strcmp(choose , "1\n") == 0)
			{
				printf("input msg: \n");
				memset(send_text , 0 , DATA_MAX_LEN);
				fgets(send_text , DATA_MAX_LEN , stdin);
				tx_msg.type = 0x1234;
				tx_msg.len = strlen(send_text);
				tx_msg.buffer = send_text;
				if (send_message(&tx_msg) < 0) {
					printf("send_message fail \n");
				} else {
					printf("send_message ok \n");
				}
			}
			if(strcmp(choose , "2\n") == 0)
			{
				memset(rcv_text , 0 , DATA_MAX_LEN);
				rx_msg.buffer = rcv_text;
				rx_msg.len = DATA_MAX_LEN;
				ret = recv_message(&rx_msg, 1000);
				printf("recv total len:%d \n", ret);
				printf("rx msg:type(0x%x) \n", rx_msg.type);
				printf("rx msg:len(%d) \n", rx_msg.len);
				printf("rx msg:buffer==>%s\n", rx_msg.buffer);
			}
			if(strcmp(choose , "3\n") == 0)
			{
				printf("Client closed.\n");
				close(connfd);
				close(listenfd);
				exit(1);
			}
		}
	}
	return 0;
}