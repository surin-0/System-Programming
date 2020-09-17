#include <linux/init.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/fs.h> 
#include <linux/uaccess.h> 
#include <linux/slab.h> 
#include <linux/delay.h>

#include <asm/mach/map.h> 
#include <asm/uaccess.h> 


#define MOTOR_MAJOR_NUMBER   500
#define MOTOR_DEV_NAME      "motor_dev" 

#define GPIO_BASE_ADDR      0x3F200000
#define PWM_BASE_ADDR      0x3F20C000

#define GPFSEL1            0x04
#define PWM_CTL            0x00
#define PWM_RNG2            0x20
#define PWM_DAT2         0x24
#define PWM_STA            0x04

#define CLK_BASE_ADDR      0x3F101000
#define CLK_PWM_CTL         0xA0
#define CLK_PWM_DIV         0xA4
#define BCM_PASSWORD      0x5A000000

#define IOCTL_MOTOR_MAGIC_NUMBER 'm'
#define IOCTL_CMD_ROTATE_MOTOR _IOW(IOCTL_MOTOR_MAGIC_NUMBER, 0, int)

#define us 1/1000000

static void __iomem* gpio_base;
static void __iomem* pwm_base;
static void __iomem* clk_base;

volatile unsigned int *gpsel1;
volatile unsigned int *pwm_ctl, *pwm_dat2, *pwm_rng2, *pwm_sta;
volatile unsigned int *clk_pwm_ctl, *clk_pwm_div;

int motor_open(struct inode * inode, struct file * filp){
    printk(KERN_ALERT "motor driver open\n"); 
    
    gpio_base = ioremap(GPIO_BASE_ADDR, 0xFF);
    pwm_base = ioremap(PWM_BASE_ADDR, 0xFF);
    clk_base = ioremap(CLK_BASE_ADDR, 0xFF);
    
    gpsel1 = (volatile unsigned int*)(gpio_base + GPFSEL1);
   pwm_ctl = (volatile unsigned int*)(pwm_base + PWM_CTL);
   pwm_rng2 = (volatile unsigned int*)(pwm_base + PWM_RNG2);
   pwm_dat2 = (volatile unsigned int*)(pwm_base + PWM_DAT2);
   pwm_sta = (volatile unsigned int*)(pwm_base + PWM_STA);
   clk_pwm_ctl = (volatile unsigned int*)(clk_base + CLK_PWM_CTL);
   clk_pwm_div = (volatile unsigned int*)(clk_base + CLK_PWM_DIV);
   
    return 0; 
}

int motor_release(struct inode * inode, struct file * filp) {
    printk(KERN_ALERT "motor driver closed\n"); 
    iounmap((void*)gpio_base); 
    iounmap((void*)pwm_base);
    iounmap((void*)clk_base);
    return 0; 
}

long motor_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{   
    *gpsel1 &= ~(0x07<<9);
   *gpsel1 |= (0x04<<9);      // gpio13 ALT0 (PWM1)
   
   int pwm_ctrl = *pwm_ctl;
   *pwm_ctl = 0;         // store PWM control and stop PWM;
   ssleep(10*us);
   
   *clk_pwm_ctl = BCM_PASSWORD | (0x01 << 5);   // stop PWM Clock
   ssleep(10*us);
   
   int idiv = (int)(19200000.0f / 16000.0f);   // Oscilloscope to 16kHz
   *clk_pwm_div = BCM_PASSWORD | (idiv << 12);   // integer part of division register
   *clk_pwm_ctl = BCM_PASSWORD | (0x11);      // set source to oscilloscope & enable PWM_CLK
   
   *pwm_ctl = pwm_ctrl;
   *pwm_ctl |= (1<<8);        // PWEN2 - channel 2 M/S tranmission is used
   *pwm_ctl |= (1<<15);      // MSEN2 - channel 2 M/S tranmission is used
   *pwm_rng2 = 320;

   int angle;
    switch (cmd){ 
        case IOCTL_CMD_ROTATE_MOTOR: 
        printk(KERN_ALERT "MOTOR ROTATE ON\n");
        copy_from_user(&angle, (const void*)arg, 4);
        printk(KERN_INFO "%d\n", angle);
        angle = (angle*16/180) + 24;
        *pwm_dat2 = angle;
      break; 

        default : 
        printk(KERN_ALERT "ioctl : command error\n");
    }
    
    return 0; 
}

static struct file_operations motor_fops = { 
    .owner = THIS_MODULE,  
    .open = motor_open, 
    .release = motor_release,
    .unlocked_ioctl = motor_ioctl
}; 

int __init motor_init (void) { 
    if(register_chrdev(MOTOR_MAJOR_NUMBER, MOTOR_DEV_NAME, &motor_fops) < 0)
        printk(KERN_ALERT "motor driver initalization failed\n"); 
    else 
        printk(KERN_ALERT "motor driver initalization succeed\n");
    return 0; 
}

void __exit motor_exit(void){ 
    unregister_chrdev(MOTOR_MAJOR_NUMBER, MOTOR_DEV_NAME); 
    printk(KERN_ALERT "motor driver exit"); 
}

module_init(motor_init); 
module_exit(motor_exit);  

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sungkyu"); 
MODULE_DESCRIPTION("motor device driver");
