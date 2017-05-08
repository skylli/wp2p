#include "../up2p.h"
#ifndef _cust_gpio_pwm_i2c_h
#define  _cust_gpio_pwm_i2c_h

int gpio_init(u8 pin,u8 mode,u8 value);
int readgpio(u8 pin,u8 *value);
int writegpio(u8 pin,u8 value);
int setpwm(u8 pin,u16 freq,u16 duty);
int changepwmfreqduty(u8 pin,u16 freq,u16 duty);
int readpinpwmfreqduty(u8 pin,u16 *freq,u16 *duty);
void gpio_pwm__i2c_init(void);
int SoftI2c_Setup(u8 scl,u8 sda,u16 speek);
int releaseSoftI2c(void);


int I2c_Read(u8 addres,u8 *wbuf,u8 wlen,u8 *readBuf,u8 rlen);
int I2c_write(u8 addr,u8 *wbuf,u8 len);

#ifdef _WIN32
int returngpiomode(u8 pin);

#endif


#endif

