//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//sudo　つけて実行すること！！！！！！
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <linux/i2c-dev.h> //I2C用インクルード
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <pthread.h>

#include <wiringPi.h>

#include "adxl345.h"
#include "l3gd20.h"
#include "i2c.h"

#define L3GD20_ADDRESS 0x6a
#define ADXL345_ADDRESS 0x53
#define MOTOROUT1 14
#define MOTOROUT2 15
#define MOTORPWM 18

void* io_modules(void* arg){
  int i2c_fd; //i2cへのファイルディスクリプタ
  char *i2cFileName = "/dev/i2c-1"; //リビジョンに合わせてかえること
  int i2cAddress[2];
  float gyroData[3]; //(x,y,z)
  float accelData[3]; //(x,y,z)
  float gyrooffset[3];
  float acceloffset[3];
  float theta[3];
  unsigned int timer=0;
  float dt=0;
  int pwm=0; //pwm

  pthread_detach(pthread_self());

  //gpioピン初期設定
  if ( wiringPiSetupGpio() == -1) {
    printf("setup error");
    exit(1);
  }
  pinMode(MOTOROUT1, OUTPUT);
  pinMode(MOTOROUT2, OUTPUT);
  pinMode(MOTORPWM, PWM_OUTPUT);

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


  int i,j;
  //offset検出
  for (j=0;j<3;j++){
    gyrooffset[j] = 0.0;
    acceloffset[j]= 0.0;
    theta[j]=0.0;
  }

  //スレーブアドレスをセットしてから読むこと
  //最初100個の平均をoffsetとする
  for (i=0; i< 1000; i++) {
    i2c_setAddress(&i2c_fd, i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_setAddress(&i2c_fd, i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);
    for (j=0;j<3;j++){
      gyrooffset[j] += gyroData[j];
      acceloffset[j]+= accelData[j];
    }
  }
  for (j=0;j<3;j++){
    gyrooffset[j] = gyrooffset[j] / 1000.0;
    acceloffset[j]= acceloffset[j] / 1000.0;
  }
  acceloffset[2] -= 1.0; //重力の分
  printf("(%5.2f, %5.2f, %5.2f)\n", gyrooffset[0],gyrooffset[1],gyrooffset[2]);
  printf("(%5.2fg, %5.2fg, %5.2fg)\n", acceloffset[0],acceloffset[1],acceloffset[2]);
  delay(1000);


  digitalWrite(MOTOROUT1, 1);
  int flag=1;
  timer = micros();
  for (i=0; i< 1000; i++) {
    i2c_setAddress(&i2c_fd, i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_setAddress(&i2c_fd, i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);

    dt = (micros() - timer)*0.000001;
    timer = micros();
    for (j=0;j<3;j++){
      gyroData[j] -= gyrooffset[j];
      theta[j] += gyroData[j]*dt;
    }

    if ((gyroData[2] > 0.5) || (gyroData[2] < -0.5)) {
      printf("detect\n");
      if (flag==0){
        digitalWrite(MOTOROUT2, 1);
        digitalWrite(MOTOROUT1, 0);
        pwm = 1000;
        pwmWrite(MOTORPWM, pwm);
        flag = 1;
      } else {
        digitalWrite(MOTOROUT1, 1);
        digitalWrite(MOTOROUT2, 0);
        pwm = 500;
        pwmWrite(MOTORPWM,pwm);
        flag = 0;
      }
    }

    printf("Gyro: (%5.2f, %5.2f, %5.2f)\n", gyroData[0],gyroData[1],gyroData[2]);
    printf("Accel: (%2.5fg, %2.5fg, %2.5fg)\n", accelData[0],accelData[1],accelData[2]);
    printf("Theta: (%5.2f, %5.2f, %5.2f)\n", theta[0],theta[1],theta[2]);
    printf("Time: %lf\n", dt);
    // delay(1);
  }

  pwm = 0;
  digitalWrite(MOTOROUT1, 0);
  digitalWrite(MOTOROUT2, 0);
  pwmWrite(MOTORPWM, pwm);
}
