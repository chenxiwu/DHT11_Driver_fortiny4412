#ifndef __DHT11_H__
#define __DHT11_H__

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <mach/map.h> 
#include <mach/regs-gpio.h>
//#include <mach/gpio-bank-n.h>
#include <mach/gpio-bank-b.h>
#include <mach/regs-clock.h> 
#include <asm/irq.h>

#include <plat/gpio-core.h>
#include <plat/gpio-cfg.h>

void s3c6410_gpio_cfgpin(unsigned int pin, unsigned int function)
{
    //s3c_gpio_cfgpin(pin,function);
		unsigned int tmp; 
		tmp = readl(S3C64XX_GPBCON); 
		tmp = (tmp & ~(3<<pin*2))|(function<<pin*2); 
		writel(tmp, S3C64XX_GPBCON); 
}

void s3c6410_gpio_pullup(unsigned int pin, unsigned int to)
{
		//s3c_gpio_setpull(pin,to);
		unsigned int tmp; 
		tmp = readl(S3C64XX_GPBPUD); 
		tmp = (tmp & ~(3<<pin*2))|(to<<pin*2); 
		writel(tmp, S3C64XX_GPBPUD); 
}

unsigned int s3c6410_gpio_getpin(unsigned int pin) 
{ 
		unsigned int tmp;
    tmp = readl(S3C64XX_GPBDAT); 
    tmp = tmp & (1 << (pin));  

    return tmp; 
}

void s3c6410_gpio_setpin(unsigned int pin, unsigned int dat)
{
		unsigned int tmp; 
    tmp = readl(S3C64XX_GPBDAT); 
    tmp &= ~(1 << (pin)); 
    tmp |= ( (dat) << (pin) ); 
    writel(tmp, S3C64XX_GPBDAT); ;
}

#endif
