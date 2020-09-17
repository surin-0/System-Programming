#include <linux/init.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/fs.h> 
#include <linux/uaccess.h> 
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/mach/map.h> 
#include <asm/uaccess.h> 

#define BUZZER_MAJOR_NUMBER   	202
#define BUZZER_DEV_NAME      	"buzzer_dev" 

#define GPIO_BASE_ADDR	0x3F200000

#define GPFSEL1			0x04
#define GPCLR0			0x28
#define GPSET0			0x1C

#define IOCTL_BUZZER_MAJIC_NUMBER	'b'
#define IOCTL_CMD_SET_BUZZER_ON     _IO(IOCTL_BUZZER_MAJIC_NUMBER, 0)
#define IOCTL_CMD_SET_BUZZER_OFF	_IO(IOCTL_BUZZER_MAJIC_NUMBER, 1)

static void __iomem * gpio_base;

volatile unsigned int *gpsel1, *gpclr0, *gpset0;

int buzzer_open(struct inode * inode, struct file * filp){
	printk(KERN_ALERT "buzzer driver open\n"); 

    gpio_base = ioremap(GPIO_BASE_ADDR, 0xFF);
    
    gpsel1 = (volatile unsigned int *)(gpio_base + GPFSEL1);
    gpset0 = (volatile unsigned int*)(gpio_base + GPSET0);
    gpclr0 = (volatile unsigned int*)(gpio_base + GPCLR0);
    
    *gpsel1 |= (1<<21);
    
    return 0; 
}

int buzzer_release(struct inode * inode, struct file * filp) { 
    printk(KERN_ALERT "BUZZER driver close\n"); 
    iounmap((void *)gpio_base);
     
    return 0; 
}

long buzzer_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{ 
    int data;
    switch (cmd){ 
        case IOCTL_CMD_SET_BUZZER_ON: 
            *gpset0 |= (1<<17);
            break; 
        case IOCTL_CMD_SET_BUZZER_OFF :
			*gpclr0 |= (1<<17);
			break;
        default : 
            printk(KERN_ALERT "ioctl : command error\n");
    }
    return 0; 
}

static struct file_operations buzzer_fops = { 
    .owner = THIS_MODULE, 
    .open = buzzer_open, 
    .release = buzzer_release, 
    .unlocked_ioctl = buzzer_ioctl
}; 

int __init buzzer_init (void) { 
    if(register_chrdev(BUZZER_MAJOR_NUMBER, BUZZER_DEV_NAME, &buzzer_fops) < 0)
        printk(KERN_ALERT "buzzer driver initalization failed\n"); 
    else 
        printk(KERN_ALERT "buzzer driver initalization succeed\n");
    return 0; 
}

void __exit buzzer_exit(void){ 
    unregister_chrdev(BUZZER_MAJOR_NUMBER, BUZZER_DEV_NAME); 
    printk(KERN_ALERT "buzzer driver exit"); 
}

module_init(buzzer_init); 
module_exit(buzzer_exit);  

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sohee"); 
MODULE_DESCRIPTION("foo"); 
