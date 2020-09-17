#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/mach/map.h>
#include <asm/uaccess.h>

#define	IR_MAJOR_NUMBER		509
#define IR_DEV_NAME			"ir_dev"

#define GPIO_BASE_ADDR		0x3F200000
#define SPI_BASE_ADDR		0x3F204000

#define GPFSEL0		0x00
#define GPFSEL1		0x04
#define SPI_CS		0x00
#define SPI_FIFO	0x04

static void __iomem *gpio_base;
static void __iomem *spi_base;

volatile unsigned int *gpfsel0, *gpfsel1;
volatile unsigned int *spi_cs, *spi_fifo;

int ir_open(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Infrared distance sensor driver open!!\n");
	
	gpio_base = ioremap(GPIO_BASE_ADDR, 0xFF);
	spi_base = ioremap(SPI_BASE_ADDR, 0xFF);
	
	gpfsel0 = (volatile unsigned int*)(gpio_base + GPFSEL0);
	gpfsel1 = (volatile unsigned int*)(gpio_base + GPFSEL1);
	spi_cs = (volatile unsigned int*)(spi_base + SPI_CS);
	spi_fifo = (volatile unsigned int*)(spi_base + SPI_FIFO);
	
	*gpfsel0 &= ~(0x1FF << 21);
	*gpfsel0 |= (0x24 << 24);
	*gpfsel1 &= ~(0x3F);	// 111111 = 0x3F
	*gpfsel1 |= (0x24);		// 100100 = 0x24
	
	*spi_cs &= ~(0xFFFF);		// cs clear
	
	*spi_cs &= ~(0x01<<2);
	*spi_cs &= ~(0x01<<3);		// spi(0,0)
	*spi_cs |= (0x03<<4);		// clear FIFO
	
	return 0;
}
	
int ir_release(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Infrared distance sensor driver closed!!\n");

	*gpfsel0 &= ~(0x1FF << 21);
	*gpfsel1 &= ~(0x3F);		// 111111 = 0x3F
	*spi_cs &= ~(0xFFFF);		// cs clear
	
	iounmap((void*)gpio_base);
	iounmap((void*)spi_base);
	return 0;
}

ssize_t ir_read(struct file* flip, char* buf, size_t count, loff_t* f_pos){
	
	unsigned char spi_tData[3];
	unsigned char spi_rData[3];
	int tCount = 0; 
	int rCount = 0;
	
	spi_tData[0] = 1;
	spi_tData[1] = (0x08) << 4;
	spi_tData[2] = 0;		// start single d2 d1 d0
	
	*spi_cs |= (0x03<<4);	// clear FIFO
	
	*spi_cs |= (0x01<<7);		// TA = 1
	
	while((tCount < 3) || (rCount < 3)){
		while((*spi_cs & (1<<18)) && (tCount < 3)){
			*spi_fifo = spi_tData[tCount];
			tCount++;
		}
		while((*spi_cs & (1<<17)) && (rCount < 3)){
			spi_rData[rCount] = *spi_fifo;
			rCount++;
		}	
	}	
	
	while((!(*spi_cs)) & (1<<16)){
		// Transfer is in progress
	}
	
	*spi_cs |= (0x03<<4);	// clear FIFO
	*spi_cs &= ~(1<<7);		// TA = 0
	
	int data = ((spi_rData[1]&0x03)<<8) + spi_rData[2];
	
	printk(KERN_ALERT "reading input : %x \n", data);
	
	copy_to_user(buf, &data, sizeof(int));
	
	return count;
}

static struct file_operations ir_fops = {
	.owner = THIS_MODULE,
	.open = ir_open,
	.release = ir_release,
	.read = ir_read
};

int __init ir_init(void){
	if(register_chrdev(IR_MAJOR_NUMBER, IR_DEV_NAME, &ir_fops) < 0)
		printk(KERN_ALERT "Infrared distance sensor initialization failed\n");
	else
		printk(KERN_ALERT "Infrared distance sensor initialization success\n");
	return 0;
}

void __exit ir_exit(void){
	unregister_chrdev(IR_MAJOR_NUMBER, IR_DEV_NAME);
	printk(KERN_ALERT "Infrared distance sensor driver exit done");
}

module_init(ir_init);
module_exit(ir_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("surin");
MODULE_DESCRIPTION("system programming team project (3)");

	
	
	
	
	
	
	
	
	
