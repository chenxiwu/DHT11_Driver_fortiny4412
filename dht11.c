#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <mach/regs-gpio.h>
#include <linux/device.h>
#include <mach/hardware.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/errno.h>

#include "dht11.h"

//#define DEBUG
/* 相关引脚定义,方便以后移植 */
#define DEVICE_NAME "dht11"
#define DQ         2
#define CFG_IN     0
#define CFG_OUT    1

// dht11主次设备号（动态分配）
int dht11_major = 0;
int dht11_minor = 0;
int dht11_nr_devs = 1;

 // 定义设备类型
static struct dht11_device {
    struct cdev cdev;
}; 
struct dht11_device dht11_dev;

static struct class *dht11_class;

/* 函数声明 */
static int dht11_open(struct inode *inode, struct file *filp);
static int dht11_init(void);
static ssize_t dht11_read(struct file *filp, char __user *buf,
                            size_t count, loff_t *f_pos);
void dht11_setup_cdev(struct dht11_device *dev, int index);


/*
 函数名称: dht11_open()
 函数功能: 打开设备，初始化dht11
 入口参数: inode:设备文件信息; filp: 被打开的文件的信息
 出口参数: 成功时返回0,失败返回-1
*/

static int dht11_open(struct inode *inode, struct file *filp)
{
    int flag = 0;
    struct dht11_device *dev;
    dev = container_of(inode->i_cdev, struct dht11_device, cdev);
    filp->private_data = dev;
///////////////////////////从这开始改///////////////////////////
    flag = dht11_init();
    return flag;
}

/*
 函数名称: dht11_init()
 函数功能: 复位dht11
 入口参数: 无
 出口参数: retval:成功返回0,失败返回1
 备    注: 操作时序见dht11 datasheet
*/
static int dht11_init(void)
{
    int retval = 1;
//	s3c6410_gpio_setpin();
	s3c6410_gpio_cfgpin(DQ, CFG_OUT);
	return 0;
	
/* 	s3c6410_gpio_pullup(DQ, 0);
	s3c6410_gpio_setpin(DQ, 1);//主机把总线拉高，作为空闲状态
	
	
	s3c6410_gpio_setpin(DQ, 0);//主机把总线拉低
	udelay(22000);//	要求拉低时间大于18ms
	s3c6410_gpio_setpin(DQ, 1);//拉高，表示开始信号结束
	udelay(40);
	retval = s3c6410_gpio_getpin(DQ);//信号存在应该为0 */
	
	
	
	
/*    s3c6410_gpio_cfgpin(DQ, CFG_OUT);
    s3c6410_gpio_pullup(DQ, 0);

    s3c6410_gpio_setpin(DQ, 1);
    udelay(2);
    s3c6410_gpio_setpin(DQ, 0);        // 拉低dht11总线，复位dht11
    udelay(500);                       // 保持复位电平500us

    s3c6410_gpio_setpin(DQ, 1);        // 释放dht11总线
    udelay(60);

    // 若复位成功，dht11发出存在脉冲（低电平，持续60~240us）
    s3c6410_gpio_cfgpin(DQ, CFG_IN);
    retval = s3c6410_gpio_getpin(DQ);

    udelay(500);
    s3c6410_gpio_cfgpin(DQ, CFG_OUT);
    s3c6410_gpio_pullup(DQ, 0);
    s3c6410_gpio_setpin(DQ, 1);        // 释放总线*/
    
   
}
//dht11的读取内容判断部分
//用来判断读取回来的是高电平还是低电平
static char dht11_drv_read_byte ( void )
{
	char dht11_byte ;
	unsigned char i ;
	unsigned char temp ;
	
	dht11_byte = 0 ;
	/*接收数据，并利用|和&存储到dht11_byte*/
	for ( i = 0 ; i < 8 ; i ++ )
	{
		temp = 0 ;
		/*50us传输间隔时间*/
		while ( ! (s3c6410_gpio_getpin(DQ) ) )
		{
			temp ++ ;
			if ( temp > 12 )
				return 1 ;
			udelay ( 5 ) ;
		}
		temp = 0 ;
		
		/*根据接收到高电平持续的时长判断数据是0还是1*/
		while ( s3c6410_gpio_getpin(DQ))
		{
			temp ++ ;
			if ( temp > 20 )
				return 1 ;
			udelay ( 5 ) ;
		}
		/*大于6的话，也就是大于35us，数据为1，否则数据为0*/
		if ( temp > 6 )
		{
			dht11_byte <<= 1 ;
			dht11_byte |= 1 ;
		} 
		else
		{
			dht11_byte <<= 1 ;
			dht11_byte |= 0 ;
		}
	}
	return dht11_byte ;
}

/*
 函数名称: dht11_read()
&nbsp;函数功能: 读出dht11的温度
*/
static ssize_t dht11_read(struct file *filp, char __user *buf,
                            size_t count, loff_t *f_pos)
{
    int flag;
	unsigned char DataTemp;
	unsigned char i;
	unsigned char err;
	char tempBuf[5];
	err = 0 ;

	//设置为输出引脚，输出低电平
 //	s3c6410_gpio_pullup(DQ, 0);
	s3c6410_gpio_setpin(DQ, 0);
	//对应数据手册，主机至少拉低18ms
	msleep(18);//休眠延迟函数，并于精确，实际时间大于18ms
	//对应手册，主机拉高20-40us
	s3c6410_gpio_setpin(DQ,1);
	udelay ( 40 );	//udelay()忙等待函数,延迟过程中无法运行其他任务．这个延迟的时间是准确,但是浪费CPU，不适用于长时间延时。
	//设置引脚为输入引脚
	s3c6410_gpio_cfgpin(DQ, CFG_IN);
	
			while ( !( s3c6410_gpio_getpin(DQ) ) && DataTemp )
		{
			DataTemp --;
			udelay ( 10 );
		}
		if ( !DataTemp )
		{
			err = 1;
			count = -EFAULT;
		}
	/*
		同上，消掉80us的高电平信号。
		while ( ( gpio_get_value ( GPC_7 ) ) )
			;
*/
			
		if ( !err )
		{
			DataTemp = 10 ;
			while ( (s3c6410_gpio_getpin(DQ)) && DataTemp )
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
				if( !err )
		{

			/*利用数组接收数据
			分别是：
			8bit湿度整数数据
			8bit湿度小数数据
			8bi温度整数数据
			8bit温度小数数据
			8bit校验和
			*/
			for ( i = 0; i < 5; i ++ )
			{
				tempBuf[i] = dht11_drv_read_byte () ;
			}

			
			DataTemp = 0 ;
			
			/*校验数据是否准确*/
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

			/*发送给用户空间*/
			if ( copy_to_user ( buf , tempBuf , count ) )
			{
				count = -EFAULT ;
			}

		}
		
	/*一次数据读取完成*/
	
	s3c6410_gpio_cfgpin(DQ, CFG_OUT);
	s3c6410_gpio_setpin(DQ,1);
	return count;		
}

/*
  字符驱动程序的核心，应用程序所调用的open,read等函数最终会
  调用这个结构中的对应函数
*/
static struct file_operations dht11_dev_fops = {
    .owner = THIS_MODULE,
    .open = dht11_open,
    .read = dht11_read,
};

/*
 函数名称: dht11_setup_cdev()
 函数功能: 初始化cdev
 入口参数: dev:设备结构体; index：
*/
void dht11_setup_cdev(struct dht11_device *dev, int index)
{
    int err, devno = MKDEV(dht11_major, dht11_minor + index);

    cdev_init(&dev->cdev, &dht11_dev_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&(dev->cdev), devno, 1);
    if (err)
    {
#ifdef DEBUG
        printk(KERN_NOTICE "ERROR %d add dht11\n", err);
#endif
    }
} 

/*
 函数名称: dht11_dev_init()
 函数功能: 为温度传感器分配注册设备号，初始化cdev
 入口参数: 无
 出口参数: 若成功执行，返回0
*/

static int __init dht11_dev_init(void)
{
    dht11_major = register_chrdev(dht11_major, DEVICE_NAME, &dht11_dev_fops);
    if (dht11_major<0)
    {
	printk(DEVICE_NAME " Can't register major number!\n");
	return -EIO;
    }

    dht11_class = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(dht11_class, NULL, MKDEV(dht11_major, dht11_minor), NULL, DEVICE_NAME);
#ifdef DEBUG
	printk(KERN_WARNING "register dht11 driver successful!\n");
#endif
    return 0;
}


 /*函数名称: dht11_dev_exit()
 &nbsp; 函数功能: 注销设备*/

static void __exit dht11_dev_exit(void)
{
    device_destroy(dht11_class, MKDEV(dht11_major,dht11_minor));
    class_unregister(dht11_class);
    class_destroy(dht11_class);
    unregister_chrdev(dht11_major, DEVICE_NAME);
#ifdef DEBUG
	printk(KERN_WARNING "Exit dht11 driver!\n");
#endif
}

module_init(dht11_dev_init);
module_exit(dht11_dev_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Rzy");
