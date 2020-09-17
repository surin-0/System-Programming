#include <linux/init.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/fs.h> 
#include <linux/uaccess.h> 
#include <linux/slab.h> 
#include <linux/delay.h>

#include <asm/mach/map.h> 
#include <asm/uaccess.h> 

typedef struct{
   unsigned int humidity_integer;
   unsigned int humidity_float;
   unsigned int temperature_integer;
   unsigned int temperature_float;
}Info;

#define MAX_DATA                40
#define HIGH                    1   
#define LOW                     0

#define HUMIDITY_MAJOR_NUMBER   501
#define HUMIDITY_DEV_NAME         "humidity_dev" 

#define IOCTL_HUMIDITY_MAGIC_NUMBER    'h'
#define IOCTL_CMD_CHECK_HUMIDITY      _IOR(IOCTL_HUMIDITY_MAGIC_NUMBER, 0, Info)

#define GPIO_BASE_ADDRESS       0x3F200000

#define GPFSEL0                  0x00
#define GPSET0                  0x1C
#define GPCLR0                  0x28
#define GPLEV0                  0x34

static void __iomem* gpio_base;

volatile unsigned int* gpsel0;
volatile unsigned int* gpset0;
volatile unsigned int* gpclr0;
volatile unsigned int* gplev0;

int humidity_open(struct inode * inode, struct file * filp){
    printk(KERN_ALERT "\nhumidity driver open\n"); 
    gpio_base = ioremap(GPIO_BASE_ADDRESS, 0xFF);
    
    gpsel0 = (volatile unsigned int*)(gpio_base + GPFSEL0);
    gpset0 = (volatile unsigned int*)(gpio_base + GPSET0);
    gpclr0 = (volatile unsigned int*)(gpio_base + GPCLR0);
    gplev0 = (volatile unsigned int*)(gpio_base + GPLEV0);

    return 0; 
}

int humidity_release(struct inode * inode, struct file * filp) { 
    printk(KERN_ALERT "humidity driver closed\n\n"); 
    iounmap((void *)gpio_base); 
    return 0; 
}


long humidity_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{ 
    Info info;
    int data[5] = {0};
    int index = 0;
   int pre_data = HIGH;
   int counter = 0;
    int i = 0;
    switch (cmd){ 
        case IOCTL_CMD_CHECK_HUMIDITY: 

            *gpsel0 |= (HIGH << 12);       //GPIO 4 SET OUTPUT MODE

            *gpset0 |= (HIGH << 4);        //GPIO 4 SET HIGH
            mdelay(800);             //SLEEP FOR 800 miliseconds to synchronize bits from loop

            *gpclr0 |= (HIGH << 4);        //GPIO 4 SET LOW
            mdelay(18);              //SLEEP FOR 18 miliseconds

            *gpset0 |= (HIGH << 4);        //GPIO 4 SET HIGH
            udelay(30);             //SLEEP FOR 30 microseconds

           *gpsel0 &= (LOW << 12);       //GPIO 4 SET INPUT MODE

            //except first 3(wait, response, ready) signals
            for(i = 0; i < 3; i++){
                if(((*gplev0 >> 4) & 0x01) == LOW){
                    while(((*gplev0 >> 4) & 0x01) == LOW){} continue;
                }
                if(((*gplev0 >> 4) & 0x01) == HIGH){
                    while(((*gplev0 >> 4) & 0x01) == HIGH){} continue;
                }
            }
            //interpret '0' and '1' bits
           while(index < MAX_DATA){
                if(((*gplev0 >> 4) & 0x01) == LOW) continue;
              counter = 0;
              while(((*gplev0 >> 4) & 0x01) == HIGH){
                 counter++;
                    udelay(1);
              }
              if(counter > 70) break;
             data[index / 8] <<= 1;
             if(counter > 28) data[index / 8] |= 1;
                index++;
           }
            //printk(KERN_INFO "data[0]: %x data[1]: %x data[2]: %x data[3]: %x data[4]: %x\n", data[0], data[1], data[2], data[3], data[4]); 
            if(data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF))
                break;
            info.humidity_integer = data[0];
            info.humidity_float = data[1];
            info.temperature_integer = data[2];
            info.temperature_float = data[3];
            //printk(KERN_INFO "humidity: %d.%d%% temperature: %d.%d%%\n", info.humidity_integer, info.humidity_float, info.temperature_integer, info.temperature_float);

            copy_to_user((void*)arg, (void*)&info, sizeof(info)); 
          break; 

        default : 
            printk(KERN_ALERT "ioctl : command error\n");
    }
    
    return 0; 
}

static struct file_operations humidity_fops = { 
    .owner = THIS_MODULE,  
    .open = humidity_open, 
    .release = humidity_release,
    .unlocked_ioctl = humidity_ioctl
}; 

int __init humidity_init (void) { 
    if(register_chrdev(HUMIDITY_MAJOR_NUMBER, HUMIDITY_DEV_NAME, &humidity_fops) < 0)
        printk(KERN_ALERT "humidity driver initalization failed\n"); 
    else 
        printk(KERN_ALERT "humidity driver initalization succeed\n");
    return 0; 
}

void __exit humidity_exit(void){ 
    unregister_chrdev(HUMIDITY_MAJOR_NUMBER, HUMIDITY_DEV_NAME); 
    printk(KERN_ALERT "humidity driver exit"); 
}

module_init(humidity_init); 
module_exit(humidity_exit);  

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sungkyu"); 
MODULE_DESCRIPTION("humidity device driver");
