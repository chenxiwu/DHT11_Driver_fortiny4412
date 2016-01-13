#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <mach/hardware.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
 
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

#define dht11_io EXYNOS4_GPX1(0)
static unsigned dht11_major;
static dev_t dht11_id;

static struct cdev dht11_cdev;
static struct class *dht11_class;

static int dht11_open (struct inode *inode,struct file *filp)  
  
{
	s3c_gpio_cfgpin(dht11_io, S3C_GPIO_OUTPUT);// 初始化IO端口
	gpio_set_value(dht11_io, 1);
	return 0;	
}

static char DHT11_read_byte ( void )
{
	char DHT11_byte ;
	unsigned char i ;
	unsigned char temp ;
	
	DHT11_byte = 0 ;
	for ( i = 0 ; i < 8 ; i ++ )
	{
		temp = 0 ;
		while ( ! (gpio_get_value ( dht11_io ) ) )
		{
			temp ++ ;
			if ( temp > 12 )
				return 1 ;
			udelay ( 5 ) ;
		}
		temp = 0 ;
		while ( gpio_get_value ( dht11_io ) )
		{
			temp ++ ;
			if ( temp > 20 )
				return 1 ;
			udelay ( 5 ) ;
		}
		if ( temp > 6 )
		{
			DHT11_byte <<= 1 ;
			DHT11_byte |= 1 ;
		} 
		else
		{
			DHT11_byte <<= 1 ;
			DHT11_byte |= 0 ;
		}
	}
	return DHT11_byte ;
}




static int dht11_read (struct file *filp, char __user *userbuf, size_t count,loff_t *f_pos)  
{   
unsigned char DataTemp;
	unsigned char i;
	unsigned char err;
	char tempBuf[5];

	
	err = 0 ;

	s3c_gpio_cfgpin(dht11_io, S3C_GPIO_OUTPUT);
	gpio_set_value(dht11_io, 0);
	msleep ( 18 );

	gpio_set_value(dht11_io, 1);
	udelay ( 40 );
	s3c_gpio_cfgpin(dht11_io, S3C_GPIO_INPUT);
	if ( !err )
	{
		DataTemp = 10 ;
		while ( !( gpio_get_value ( dht11_io ) ) && DataTemp )
		{
			DataTemp --;
			udelay ( 10 );
		}
		if ( !DataTemp )
		{
			err = 1;
			count = -EFAULT;
		}
	}
	if ( !err )
	{
		DataTemp = 10 ;
		while ( (( gpio_get_value ( dht11_io ) ) ) && DataTemp )
		{
			DataTemp --;
			udelay ( 10 );
		}
		if ( !DataTemp )
		{
			err = 1;
			count = -EFAULT;
		}
	}
	if ( !err )
	{
		for ( i = 0; i < 5; i ++ )
		{
			tempBuf[i] = DHT11_read_byte () ;
		}
		DataTemp = 0 ;
		for ( i = 0; i < 4; i ++ )
		{
			DataTemp += tempBuf[i] ;
		}
		if ( DataTemp != tempBuf[4] )
		{
			count = -EFAULT;
		}

		if ( count > 5 )
		{
			count = 5 ;
		}

		if ( copy_to_user ( userbuf , tempBuf , count ) )
		{
			count = -EFAULT ;
		}
	}
	s3c_gpio_cfgpin(dht11_io, S3C_GPIO_OUTPUT);
	gpio_set_value(dht11_io, 1);
    return count;  
}  

struct file_operations dht11_fops=
{
.owner = THIS_MODULE,
.open  = dht11_open,
.read  = dht11_read,
};


static int dht11_init(void)
{
 
 //申请了一个设备号
    alloc_chrdev_region(&dht11_id, 0, 1,"DHT11" );//返回是否申请成功，第一个次设备号，分配设备数，设备名保存到dev中
	dht11_major = MAJOR(dht11_id);
	
	cdev_init(&dht11_cdev, &dht11_fops);
	cdev_add(&dht11_cdev, dht11_id, 1);       

	dht11_class = class_create(THIS_MODULE, "DHT11"); /*创建设备节点/dev/DHT11*/
	device_create(dht11_class, NULL, MKDEV(dht11_major , 0), NULL, "DHT11"); /*添加1个设备*/
	
  	s3c_gpio_cfgpin(dht11_io, S3C_GPIO_OUTPUT);
	gpio_set_value(dht11_io, 1);
	
    printk("dht11 init succeed\n");
    return 0;
}

static void dht11_exit(void)
{
	device_destroy(dht11_class, MKDEV(dht11_major , 0));//删除一个设备
	class_destroy(dht11_class);	
	cdev_del(&dht11_cdev);//删除一个设备节点
	printk("dht11 exit succeed\n");
}

module_init(dht11_init);
module_exit(dht11_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Billy.Rzy");