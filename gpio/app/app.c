#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/sysmacros.h>

#define UART_DEV_PATH	"/dev/uart_dev"

int main(void){
	int uart_dev;
	int mode = -1;
	
	printf("Team 3 Proj --- RaspberryPi Transmission Program\n");
	
	uart_dev = open(UART_DEV_PATH, O_RDWR);
	
	if(uart_dev < 0){
		printf("fail to open uart device because : %s \n", strerror(errno));
		return -1;
	}
	
	char *str = (char*)malloc(sizeof(char)*32);
	char buffer[1024];
	
	printf("select mode :::: 0 = send / 1 = receive\n->>>> ");
	scanf("%d", &mode);
	
	switch(mode){
		case 0 :
			printf("----SEND MODE----\n");
			printf("please enter message to send : ");
			scanf("%s", str);
			write(uart_dev, str, strlen(str));
			printf("----SEND COMPLETED----\n");
			break;
		case 1 :
			printf("----RECEIVE MODE----\n");
			printf("----PLEASE WAIT!----\n");
			read(uart_dev, buffer, 1024);
			printf("----RECEIVE COMPLETED----\n");
			printf("message = \"%s\"\n", buffer);
			break;
		default :
			printf("invalid mode parameter\n");
			break;
	}
	
	free(str);
	close(uart_dev);
	
	return 0;
}
			
			
	
	
	
