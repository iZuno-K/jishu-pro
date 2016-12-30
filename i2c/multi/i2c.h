#ifndef _I2C_H_
#define _I2C_H_

void i2c_open(int *, char *);
void i2c_setAddress(int *,int);
void i2c_write(unsigned char , unsigned char , int );
unsigned char i2c_read(unsigned char , int );

#endif //_I2C_H_
