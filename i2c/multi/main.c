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
  float accelData[3]; //(x,y,z)
  float gyrooffset[3];
  float acceloffset[3];

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
  int i,j;
  //offset検出
  for (j=0,j<3;j++){
    gyrooffset[j] = 0;
    acceloffset[j]= 0;
  }
  //最初100個の平均をoffsetとする
  for (i=0; i< 100; i++) {
    i2c_setAddress(&i2c_fd, i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_setAddress(&i2c_fd, i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);
    for (j=0,j<3;j++){
      gyrooffset[j] += gyroData[j];
      acceloffset[j]+= accelData[j];
    }
  }
  for (j=0,j<3;j++){
    gyrooffset[j] = gyrooffset / 100.0;
    acceloffset[j]= acceloffset / 100.0;
  }
  acceloffset[2] -= 1.0; //重力の分
  printf("(%5.2f, %5.2f, %5.2f)\n", gyrooffset[0],gyrooffset[1],gyrooffset[2]);
  printf("(%5.2fg, %5.2fg, %5.2fg)\n", acceloffset[0],acceloffset[1],acceloffset[2]);
  delay(1000);

  for (i=0; i< 100; i++) {
    i2c_setAddress(&i2c_fd, i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_setAddress(&i2c_fd, i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);

    printf("(%5.2f, %5.2f, %5.2f)\n", gyroData[0]-gyrooffset[0],gyroData[1]-gyrooffset[1],gyroData[2]-gyrooffset[2]);
    printf("(%2.5fg, %2.5fg, %2.5fg)\n", accelData[0]-acceloffset[0],accelData[1]-acceloffset[1],accelData[2]-acceloffset[2]);
    delay(100);
  }

  return 0;
}
