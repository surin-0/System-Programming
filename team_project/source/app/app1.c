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

#define IOCTL_VMOTOR_MAJIC_NUMBER	'v'
#define IOCTL_CMD_MOTOR_ON			_IOW(IOCTL_VMOTOR_MAJIC_NUMBER, 0, int)
#define IOCTL_CMD_MOTOR_OFF			_IO(IOCTL_VMOTOR_MAJIC_NUMBER, 1)

#define VMOTOR_DEV_PATH		"/dev/vmotor_dev"
#define IR_DEV_PATH			"/dev/ir_dev"

#define INTERVAL			50000

int main(void)
{
	int vmotor_dev, ir_dev;
	int vmotor_data, ir_data;
	
	vmotor_dev = open(VMOTOR_DEV_PATH, O_WRONLY);
	ir_dev = open(IR_DEV_PATH, O_RDONLY);
	
	if(vmotor_dev < 0)
		printf("fail to open vibration motor device\n");
	if(ir_dev < 0)
		printf("fail to open infrared distance sensor device\n");
	
	int count = 0;
	/* infrared distance section */
	printf("===HI! INFRARED DISTANCE MEASUREMENT SECTION===\n");
	
	while(count < 200){
		
		read(ir_dev, &ir_data, sizeof(int));
		float volts = (ir_data/1024.0)*3.3; 
		printf("volts : %f\n", volts);
		
		float cm = 61.681*pow(volts, -1.133);
		
		if(cm > 150 || cm < 20){
			ioctl(vmotor_dev, IOCTL_CMD_MOTOR_OFF);
			continue;
		}
		else
			printf("cm = %.2f\n", cm);
			
		vmotor_data = (int)(-1)*(cm-20)+130;
		
		ioctl(vmotor_dev, IOCTL_CMD_MOTOR_ON, &vmotor_data);
		usleep(INTERVAL);
		count++;
	}
	ioctl(vmotor_dev, IOCTL_CMD_MOTOR_OFF);
	
	close(vmotor_dev);
	close(ir_dev);
	
	return 0;
}
