// gcc -o i2cGyro i2cGyro.c -lwiringPi
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <linux/i2c-dev.h> //I2C用インクルード
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "i2c.h"

//i2c用１byte書き込みルーチン:addressで示すレジスタにdataを書き込む
void i2c_write(unsigned char address, unsigned char data, int fd){
  unsigned char buf[2];
  buf[0] = address;
  buf[1] = data;
    if ((write(fd, buf, 2)) != 2){
    printf("Error writing to i2c slave\n");
    exit(1);
  }
  return;
}

//i2c用1byte読み込み用ルーチン:addressで示すレジスタの値を読み出す
//戻り値がレジスタ値
unsigned char i2c_read(unsigned char address, int fd){
  unsigned char buf[1];
  buf[0] = address;
  //まずはアドレスを一度書き込む！
  if ((write(fd, buf, 1)) != 1) {
    printf("Error writing to i2c slave\n");
    exit(1);
  }
  if ((read(fd, buf, 1)) != 1){
    printf("Error reading to i2c slave\n");
    exit(1);
  }
  return buf[0];
}
