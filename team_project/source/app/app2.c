#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/sysmacros.h>

#define IOCTL_BUZZER_MAJIC_NUMBER	'b'
#define IOCTL_CMD_SET_BUZZER_ON     _IO(IOCTL_BUZZER_MAJIC_NUMBER, 0)
#define IOCTL_CMD_SET_BUZZER_OFF	_IO(IOCTL_BUZZER_MAJIC_NUMBER, 1)

#define BUZZER_DEV_PATH 	"/dev/buzzer_dev"
#define GAS_DEV_PATH    	"/dev/gas_dev"

#define INTERVAL			50000


int main(void)
{
	int buzzer_dev, gas_dev;
	int gas_data, buzzer_data;
	
	buzzer_dev = open(BUZZER_DEV_PATH, O_WRONLY);
	gas_dev = open(GAS_DEV_PATH, O_RDONLY);
	
	if(gas_dev < 0)
		printf("fail to open co2 sensor device\n");
	if(buzzer_dev < 0)
		printf("fail to open buzzer device\n");
		
	/* gas detection section */
	printf("===HI! CO2 DETECTION SECTION===\n");
	int count = 0;
	long nowgas = 0;
	long storegas = 0;
	int i = 0;
	
	ioctl(buzzer_dev, IOCTL_CMD_SET_BUZZER_OFF);
	 
	while(count < 10){	
		while(i<5){
			storegas = nowgas;
			read(gas_dev, &gas_data, sizeof(long));
			float gas_volts = (gas_data/1024.0)*3.3;
			nowgas = (gas_volts*5000)/3;
			storegas=storegas+nowgas;
			i++;
			sleep(1);
		}
		i = 0;
		nowgas=storegas/5;
		printf("ppm = %d\n", nowgas);
      
		if (nowgas>1300) {
			while(i<5){
				ioctl(buzzer_dev, IOCTL_CMD_SET_BUZZER_ON);
				sleep(1);
				ioctl(buzzer_dev, IOCTL_CMD_SET_BUZZER_OFF);
				if(i == 4)
					break;
				sleep(1);
				i++;
			}
			i = 0;
		}
		else if(nowgas>1000) { 
			while(i<5){
				ioctl(buzzer_dev, IOCTL_CMD_SET_BUZZER_ON);
				sleep(2);
				ioctl(buzzer_dev, IOCTL_CMD_SET_BUZZER_OFF);
				if(i == 4)
					break;
				sleep(2);
				i++;
			} 
			i = 0;
		}
		ioctl(buzzer_dev, IOCTL_CMD_SET_BUZZER_OFF);
		count++;
	}	
	
	ioctl(buzzer_dev, IOCTL_CMD_SET_BUZZER_OFF);
	
	close(buzzer_dev);
	close(gas_dev);
	return 0;
}

