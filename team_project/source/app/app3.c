#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h> 
#include <sys/types.h>
#include <sys/sysmacros.h>

typedef struct{
   int humidity_integer;
   int humidity_float;
   int temperature_integer;
   int temperature_float;
}Info;

#define MOTOR_DEV_PATH                "/dev/motor_dev"
#define HUMIDITY_DEV_PATH             "/dev/humidity_dev"

#define IOCTL_MOTOR_MAGIC_NUMBER       'm'
#define IOCTL_CMD_ROTATE_MOTOR         _IOW(IOCTL_MOTOR_MAGIC_NUMBER, 0, int)

#define IOCTL_HUMIDITY_MAGIC_NUMBER    'h'
#define IOCTL_CMD_CHECK_HUMIDITY      _IOR(IOCTL_HUMIDITY_MAGIC_NUMBER, 0, Info)

int main(void){
   Info info;
   int motor_dev;
   int humidity_dev;

   int flag;
   int angle;
   double humidity = 0;
   double temperature = 0;
   double discomfortIndex = 0;

   motor_dev = open(MOTOR_DEV_PATH, O_WRONLY);
   humidity_dev = open(HUMIDITY_DEV_PATH, O_RDONLY);
   
   if(motor_dev < 0) printf("fail to open motor device because : %s \n", strerror(errno));
   
   if(humidity_dev < 0) printf("fail to open humidity device because : %s \n", strerror(errno));
   
   
   while(1){
      humidity = 0;
      temperature = 0;
      discomfortIndex = 0;
      flag = 1;

      ioctl(humidity_dev, IOCTL_CMD_CHECK_HUMIDITY, &info);
      
      humidity += info.humidity_float;
      if(humidity >= 100) humidity /= 1000;
      else if(humidity >= 10) humidity /= 100;
      else humidity /= 10;
      humidity += info.humidity_integer;

      temperature += info.temperature_float;
      if(temperature >= 100) temperature /= 1000;
      else if(temperature >= 10) temperature /= 100;
      else temperature /= 10;
      temperature += info.temperature_integer;

      discomfortIndex = 0.81 * temperature + 0.01 * humidity * (0.99 * temperature - 14.3) + 46.3;
      
      printf("humidity: %0.2f%% temperature: %0.2f%% discomfortIndex: %0.2f\n", humidity, temperature, discomfortIndex);
      
      if(discomfortIndex >= 75 && discomfortIndex < 100){
         if(flag){
            angle = 180;
            flag = 0;
            ioctl(motor_dev, IOCTL_CMD_ROTATE_MOTOR, &angle);
         }
         sleep(1);
         if(!flag){
            angle = -180;
            flag = 1;
            ioctl(motor_dev, IOCTL_CMD_ROTATE_MOTOR, &angle);
         }
      }
      sleep(1);
   }
   
   close(motor_dev);
   close(humidity_dev);

   return 0;
}
