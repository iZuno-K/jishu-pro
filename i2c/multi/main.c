#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <linux/i2c-dev.h> //I2C用インクルード
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "adxl345.h"
#include "l3gd20.h"

#define L3GD20_ADDRESS 0x6a
#define ADXL345_ADDRESS 0x53

int main(){
  int i2c_fd; //i2cへのファイルディスクリプタ
  char *i2cFileName = "/dev/i2c-1"; //リビジョンに合わせてかえること
  int i2cAddress[2];
  float gyroData[3]; //(x,y,z)
  int accelData[3]; //(x,y,z)
  float x1,y1,z1,x2,y2,z2;

  //i2cデバイスファイルオープン
  i2c_open(&i2c_fd, i2cFileName);

  //スレーブアドレス設定
  //L3GD20
  i2cAddress[0] = L3GD20_ADDRESS;
  i2c_setAddress(&i2c_fd, i2cAddress[0]);
  L3GD20_init(i2c_fd);

  //adxl345
  i2cAddress[1] = ADXL345_ADDRESS;
  i2c_setAddress(&i2c_fd, i2cAddress[1]);
  adxl345_init(i2c_fd);

  //スレーブアドレスをセットしてから読むこと
  int i;
  for (i=0; i< 100; i++) {
    i2c_setAddress(&i2c_fd, i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_setAddress(&i2c_fd, i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);

    printf("(%5.2f, %5.2f, %5.2f)\n", gyroData[0],gyroData[1],gyroData[2]);
    printf("(%d, %d, %d)\n", accelData[0],accelData[1],accelData[2]);
  }

  return 0;
}
