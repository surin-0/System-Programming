#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/mach/map.h>
#include <asm/uaccess.h>

#define	VMOTOR_MAJOR_NUMBER		508
#define VMOTOR_DEV_NAME			"vmotor_dev"

#define GPIO_BASE_ADDR		0x3F200000
#define PWM_BASE_ADDR		0x3F20C000
#define CLK_BASE_ADDR		0x3F101000
#define BCM_PASSWORD		0x5A000000

#define GPFSEL1				0x04
#define PWM_CTL				0x00
#define PWM_RNG1			0x10
#define PWM_DAT1			0x14
#define PWM_STA				0x04
#define CLK_PWM_CTL			0xA0
#define CLK_PWM_DIV			0xA4

#define IOCTL_VMOTOR_MAJIC_NUMBER	'v'
#define IOCTL_CMD_MOTOR_ON			_IOW(IOCTL_VMOTOR_MAJIC_NUMBER, 0, int)
#define IOCTL_CMD_MOTOR_OFF			_IO(IOCTL_VMOTOR_MAJIC_NUMBER, 1)

#define VMOTOR_RANGE	130

static void __iomem *gpio_base;
static void __iomem *pwm_base;
static void __iomem *clk_base;

volatile unsigned int *gpsel1;
volatile unsigned int *pwm_ctl, *pwm_rng1, *pwm_dat1, *pwm_sta;
volatile unsigned int *clk_pwm_ctl, *clk_pwm_div;

int vmotor_open(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Vibration Motor driver open!!\n");
	
	gpio_base = ioremap(GPIO_BASE_ADDR, 0xFF);
	pwm_base = ioremap(PWM_BASE_ADDR, 0xFF);
	clk_base = ioremap(CLK_BASE_ADDR, 0xFF);
	
	gpsel1 = (volatile unsigned int*)(gpio_base + GPFSEL1);
	pwm_ctl = (volatile unsigned int*)(pwm_base + PWM_CTL);
	pwm_rng1 = (volatile unsigned int*)(pwm_base + PWM_RNG1);
	pwm_dat1 = (volatile unsigned int*)(pwm_base + PWM_DAT1);
	pwm_sta = (volatile unsigned int*)(pwm_base + PWM_STA);
	clk_pwm_ctl = (volatile unsigned int*)(clk_base + CLK_PWM_CTL);
	clk_pwm_div = (volatile unsigned int*)(clk_base + CLK_PWM_DIV);
	
	*gpsel1 &= ~(0x07<<6);
	*gpsel1 |= (0x04<<6);		// gpio12 ALT0
	int temp_pwm_ctr = *pwm_ctl;
	*pwm_ctl = 0;			// store PWM control and stop PWM;
	mdelay(10);
	*clk_pwm_ctl = BCM_PASSWORD | (0x01 << 5);	// stop PWM Clock
	mdelay(10);
	
	int idiv = (int)(19200000.0f / 16000.0f);	// Oscilloscope to 16kHz
	*clk_pwm_div = BCM_PASSWORD | (idiv << 12);	// integer part of division register
	*clk_pwm_ctl = BCM_PASSWORD | (0x11);		// set source to oscilloscope & enable PWM_CLK
	
	*pwm_ctl = temp_pwm_ctr;
	*pwm_ctl |= (1<<0);
	*pwm_ctl |= (1<<7);		// CTL register (M/S transmission is used)
	
	*pwm_rng1 = VMOTOR_RANGE;
	*pwm_dat1 = 0;
	
	return 0;
}
	
int vmotor_release(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Vibration Motor driver closed!!\n");
	
	*pwm_dat1 = 0;
	iounmap((void*)gpio_base);
	iounmap((void*)pwm_base);
	iounmap((void*)clk_base);
	return 0;
}

long vmotor_ioctl(struct file *flip, unsigned int cmd, unsigned long arg){
	
	int kbuf = -1;
	
	switch(cmd){
		case IOCTL_CMD_MOTOR_ON :
			copy_from_user(&kbuf, (const void*)arg, 4);
			*pwm_dat1 = kbuf;
			printk(KERN_ALERT "setting Vibration Motor Data, %d\n", *pwm_dat1);
			break;
		case IOCTL_CMD_MOTOR_OFF :
			printk(KERN_ALERT "setting Vibration Motor OFF\n");
			*pwm_dat1 = 0;
			break;
		default :
			printk(KERN_ALERT "ioctl : command error\n");
	}
	return 0;
}

static struct file_operations vmotor_fops = {
	.owner = THIS_MODULE,
	.open = vmotor_open,
	.release = vmotor_release,
	.unlocked_ioctl = vmotor_ioctl
};

int __init vmotor_init(void){
	if(register_chrdev(VMOTOR_MAJOR_NUMBER, VMOTOR_DEV_NAME, &vmotor_fops) < 0)
		printk(KERN_ALERT "Vibration Motor initialization failed\n");
	else
		printk(KERN_ALERT "Vibration Motor initialization success\n");
	return 0;
}

void __exit vmotor_exit(void){
	unregister_chrdev(VMOTOR_MAJOR_NUMBER, VMOTOR_DEV_NAME);
	printk(KERN_ALERT "Vibration Motor driver exit done");
}

module_init(vmotor_init);
module_exit(vmotor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("surin");
MODULE_DESCRIPTION("system programming team project (3)");

	
	
	
	
	
	
	
	
	
