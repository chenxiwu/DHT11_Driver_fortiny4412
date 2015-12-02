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
/* ������Ŷ���,�����Ժ���ֲ */
#define DEVICE_NAME "dht11"
#define DQ         2
#define CFG_IN     0
#define CFG_OUT    1

// dht11�����豸�ţ���̬���䣩
int dht11_major = 0;
int dht11_minor = 0;
int dht11_nr_devs = 1;

 // �����豸����
static struct dht11_device {
    struct cdev cdev;
}; 
struct dht11_device dht11_dev;

static struct class *dht11_class;

/* �������� */
static int dht11_open(struct inode *inode, struct file *filp);
static int dht11_init(void);
static ssize_t dht11_read(struct file *filp, char __user *buf,
                            size_t count, loff_t *f_pos);
void dht11_setup_cdev(struct dht11_device *dev, int index);


/*
 ��������: dht11_open()
 ��������: ���豸����ʼ��dht11
 ��ڲ���: inode:�豸�ļ���Ϣ; filp: ���򿪵��ļ�����Ϣ
 ���ڲ���: �ɹ�ʱ����0,ʧ�ܷ���-1
*/

static int dht11_open(struct inode *inode, struct file *filp)
{
    int flag = 0;
    struct dht11_device *dev;
    dev = container_of(inode->i_cdev, struct dht11_device, cdev);
    filp->private_data = dev;
///////////////////////////���⿪ʼ��///////////////////////////
    flag = dht11_init();
    return flag;
}

/*
 ��������: dht11_init()
 ��������: ��λdht11
 ��ڲ���: ��
 ���ڲ���: retval:�ɹ�����0,ʧ�ܷ���1
 ��    ע: ����ʱ���dht11 datasheet
*/
static int dht11_init(void)
{
    int retval = 1;
//	s3c6410_gpio_setpin();
	s3c6410_gpio_cfgpin(DQ, CFG_OUT);
	return 0;
	
/* 	s3c6410_gpio_pullup(DQ, 0);
	s3c6410_gpio_setpin(DQ, 1);//�������������ߣ���Ϊ����״̬
	
	
	s3c6410_gpio_setpin(DQ, 0);//��������������
	udelay(22000);//	Ҫ������ʱ�����18ms
	s3c6410_gpio_setpin(DQ, 1);//���ߣ���ʾ��ʼ�źŽ���
	udelay(40);
	retval = s3c6410_gpio_getpin(DQ);//�źŴ���Ӧ��Ϊ0 */
	
	
	
	
/*    s3c6410_gpio_cfgpin(DQ, CFG_OUT);
    s3c6410_gpio_pullup(DQ, 0);

    s3c6410_gpio_setpin(DQ, 1);
    udelay(2);
    s3c6410_gpio_setpin(DQ, 0);        // ����dht11���ߣ���λdht11
    udelay(500);                       // ���ָ�λ��ƽ500us

    s3c6410_gpio_setpin(DQ, 1);        // �ͷ�dht11����
    udelay(60);

    // ����λ�ɹ���dht11�����������壨�͵�ƽ������60~240us��
    s3c6410_gpio_cfgpin(DQ, CFG_IN);
    retval = s3c6410_gpio_getpin(DQ);

    udelay(500);
    s3c6410_gpio_cfgpin(DQ, CFG_OUT);
    s3c6410_gpio_pullup(DQ, 0);
    s3c6410_gpio_setpin(DQ, 1);        // �ͷ�����*/
    
   
}
//dht11�Ķ�ȡ�����жϲ���
//�����ж϶�ȡ�������Ǹߵ�ƽ���ǵ͵�ƽ
static char dht11_drv_read_byte ( void )
{
	char dht11_byte ;
	unsigned char i ;
	unsigned char temp ;
	
	dht11_byte = 0 ;
	/*�������ݣ�������|��&�洢��dht11_byte*/
	for ( i = 0 ; i < 8 ; i ++ )
	{
		temp = 0 ;
		/*50us������ʱ��*/
		while ( ! (s3c6410_gpio_getpin(DQ) ) )
		{
			temp ++ ;
			if ( temp > 12 )
				return 1 ;
			udelay ( 5 ) ;
		}
		temp = 0 ;
		
		/*���ݽ��յ��ߵ�ƽ������ʱ���ж�������0����1*/
		while ( s3c6410_gpio_getpin(DQ))
		{
			temp ++ ;
			if ( temp > 20 )
				return 1 ;
			udelay ( 5 ) ;
		}
		/*����6�Ļ���Ҳ���Ǵ���35us������Ϊ1����������Ϊ0*/
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
 ��������: dht11_read()
&nbsp;��������: ����dht11���¶�
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

	//����Ϊ������ţ�����͵�ƽ
 //	s3c6410_gpio_pullup(DQ, 0);
	s3c6410_gpio_setpin(DQ, 0);
	//��Ӧ�����ֲᣬ������������18ms
	msleep(18);//�����ӳٺ��������ھ�ȷ��ʵ��ʱ�����18ms
	//��Ӧ�ֲᣬ��������20-40us
	s3c6410_gpio_setpin(DQ,1);
	udelay ( 40 );	//udelay()æ�ȴ�����,�ӳٹ������޷�����������������ӳٵ�ʱ����׼ȷ,�����˷�CPU���������ڳ�ʱ����ʱ��
	//��������Ϊ��������
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
		ͬ�ϣ�����80us�ĸߵ�ƽ�źš�
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

			/*���������������
			�ֱ��ǣ�
			8bitʪ����������
			8bitʪ��С������
			8bi�¶���������
			8bit�¶�С������
			8bitУ���
			*/
			for ( i = 0; i < 5; i ++ )
			{
				tempBuf[i] = dht11_drv_read_byte () ;
			}

			
			DataTemp = 0 ;
			
			/*У�������Ƿ�׼ȷ*/
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

			/*���͸��û��ռ�*/
			if ( copy_to_user ( buf , tempBuf , count ) )
			{
				count = -EFAULT ;
			}

		}
		
	/*һ�����ݶ�ȡ���*/
	
	s3c6410_gpio_cfgpin(DQ, CFG_OUT);
	s3c6410_gpio_setpin(DQ,1);
	return count;		
}

/*
  �ַ���������ĺ��ģ�Ӧ�ó��������õ�open,read�Ⱥ������ջ�
  ��������ṹ�еĶ�Ӧ����
*/
static struct file_operations dht11_dev_fops = {
    .owner = THIS_MODULE,
    .open = dht11_open,
    .read = dht11_read,
};

/*
 ��������: dht11_setup_cdev()
 ��������: ��ʼ��cdev
 ��ڲ���: dev:�豸�ṹ��; index��
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
 ��������: dht11_dev_init()
 ��������: Ϊ�¶ȴ���������ע���豸�ţ���ʼ��cdev
 ��ڲ���: ��
 ���ڲ���: ���ɹ�ִ�У�����0
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


 /*��������: dht11_dev_exit()
 &nbsp; ��������: ע���豸*/

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

asdfasdfas
asdfadsfa
asdfasf
