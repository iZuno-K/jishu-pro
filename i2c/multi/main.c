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

#define L3GD20_ADRESS 0x6a
#define ADXL345_ADRESS 0x53

int main(){
  int i2c_fd; //i2cへのファイルディスクリプタ
  char *i2cFileName = "dev/i2c-1"; //リビジョンに合わせてかえること
  int i2cAddress;
  int gyroData[3]; //(x,y,z)
  float x1,y1,z1,x2,y2,z2;

  //i2cデバイ差宇ファイルオープン
  if ((i2c_fd = open(i2cFileName, O_RDWR)) < 0){
    printf("Failed to open i2c port\n");
    exit(1);
  }

  //L3GD20
  i2cAddress = L3GD20_ADRESS;
  if ((ioctl(i2c_fd, I2C_SLAVE, i2cAddress)) < 0){
    printf("Unable to get bus access to talk to slave\n");
    exit(1);
  }
  L3GD20_init(i2c_fd);

  //adxl345
  i2cAddress = ADXL345_ADRESS;
  if ((ioctl(i2c_fd, I2C_SLAVE, i2cAddress)) < 0){
    printf("Unable to get bus access to talk to slave\n");
    exit(1);
  }
  adxl345_init(i2c_fd);

  return 0;
}
