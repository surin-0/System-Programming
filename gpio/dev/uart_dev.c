#include <linux/init.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/fs.h> 
#include <linux/uaccess.h> 
#include <linux/slab.h> 
#include <linux/delay.h>

#include <asm/mach/map.h> 
#include <asm/uaccess.h> 


#define ASCII_A		0x61

#define UART_MAJOR_NUMBER   505
#define UART_DEV_NAME      	"uart_dev" 

#define GPIO_BASE_ADDRESS	0x3F200000

#define GPFSEL1		0x04
#define GPSET0     	0x1C
#define GPCLR0      0x28
#define GPLEV0      0x34

static void __iomem* gpio_base;

volatile unsigned int* gpfsel1;
volatile unsigned int* gpset0;
volatile unsigned int* gpclr0; 
volatile unsigned int* gplev0; 

static char* buffer = NULL;

int uart_open(struct inode * inode, struct file * filp){
    printk(KERN_ALERT "uart driver open\n"); 
    gpio_base = ioremap(GPIO_BASE_ADDRESS, 0xFF);
    
    gpfsel1 = (volatile unsigned int*)(gpio_base + GPFSEL1);
    gpset0 = (volatile unsigned int*)(gpio_base + GPSET0);
    gpclr0 = (volatile unsigned int*)(gpio_base + GPCLR0);
    gplev0 = (volatile unsigned int*)(gpio_base + GPLEV0);
    
    return 0; 
}

int uart_release(struct inode * inode, struct file * filp) { 
    printk(KERN_ALERT "uart driver closed\n"); 
    iounmap((void *)gpio_base); 
    return 0; 
}

ssize_t uart_read (struct file* filp, char *buf, size_t count, loff_t *f_pos) {
    printk(KERN_ALERT "uart read function called current lev: %d\n", (*gplev0 >> 15) & 0x1);
    
    int current_value = 0;
    int pre_value = 0;
    int flag = 0;
    int temp[7];
    int data = 0;
    size_t i = 0, j = 0;
    while(1){
		if(((*gplev0 >> 15) & 0x1)){
			mdelay(5);
			temp[0] = 1;
			current_value = 1;
			printk(KERN_ALERT "START 1!!\n");
		}
		else{
			mdelay(5);
			continue;
		}
		
		j = 1;
		pre_value = 0;
		flag = 0;
		
		printk(KERN_ALERT "data RECEIVE START!\n");
		while(j < 7){
			pre_value = current_value;
			current_value = (*gplev0 >> 15) & 1;
			mdelay(5);
			if(pre_value != current_value){
				printk(KERN_ALERT "RECEIVE %d : %d !!\n",j, current_value);
				temp[j++] = current_value;
				flag = pre_value;
			}
			else{
				if(flag != pre_value){
					printk(KERN_ALERT "RECEIVE %d : %d !!\n",j, pre_value);
					temp[j++] = pre_value;
				}
			}
		}
		if(temp[0] != 1)
			break;
		if(temp[6] != 0)
			break;
				
		for(j=1; j<6; j++){
			data = data*2 + temp[j];
			temp[j] = 0;
		}
		printk(KERN_INFO "received data : %d\n", data);
		temp[0] = 0;
		temp[6] = 0;
		
		/* end letter 11010 */
		if(data == 26)
			break;
			
		buffer[i++] = data + ASCII_A;
		if(i == 32)
			break;
		data = 0;
		printk(KERN_INFO "converted data : %c\n", buffer[i-1]);
	}
	buffer[i] = '\0';
	printk(KERN_ALERT "converted string : %s\n", buffer);
	copy_to_user(buf, buffer, 1024); 
    return i;
}

ssize_t uart_write(struct file * filp, const char * buf, size_t count, loff_t * f_pos){
    copy_from_user(buffer, buf, 1024);
    printk(KERN_ALERT "uart write function called %s\n", buffer);
    
	/* end letter transmit */
    buffer[count] = '{';
    buffer[count+1] = '\0';
    *gpfsel1 |= (1<<12);
    size_t i;
    int j;
    for(i = 0; i < count + 1; i++){
		*gpset0 |= (1<<14);		// start
		mdelay(5);
		
		/* data transmit */
		char data = buffer[i] - ASCII_A;
		for(j = 4; j >= 0; j --){
			if((data >> j) & 0x1){
				*gpset0 |= (1<<14);
				printk(KERN_INFO "%d\n", 1);
				mdelay(5);
			}
			else{
				*gpclr0 |= (1<<14);	
				printk(KERN_INFO "%d\n", 0);
				mdelay(5);
			}
		}
		*gpclr0 |= (1<<14);		// stop
		mdelay(5);
		printk(KERN_INFO "data %c transmit completed\n", buffer[i]);
	}
	printk(KERN_ALERT "Transmit completed!!!\n");
    return count; 
}

static struct file_operations uart_fops = { 
    .owner = THIS_MODULE,  
    .read = uart_read,
    .write = uart_write,
    .open = uart_open, 
    .release = uart_release,
}; 

int __init uart_init (void) { 
    if(register_chrdev(UART_MAJOR_NUMBER, UART_DEV_NAME, &uart_fops) < 0)
        printk(KERN_ALERT "uart driver initalization failed\n"); 
    else 
        printk(KERN_ALERT "uart driver initalization succeed\n");
    buffer = (char*)kmalloc(1024, GFP_KERNEL);
    if(buffer != NULL)
        memset(buffer, 0, 1024);
    return 0; 
}

void __exit uart_exit(void){ 
    unregister_chrdev(UART_MAJOR_NUMBER, UART_DEV_NAME); 
    printk(KERN_ALERT "uart driver exit"); 
    kfree(buffer);
}

module_init(uart_init); 
module_exit(uart_exit);  

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TEAM3"); 
MODULE_DESCRIPTION("uart software program"); 
