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

#include <wiringPi.h>

#include "adxl345_wiringPi.h"
#include "l3gd20_wiringPi.h"

#define L3GD20_ADDRESS 0x6b
#define ADXL345_ADDRESS 0x53
#define MOTOROUT1 14
#define MOTOROUT2 15
#define MOTORPWM 18

int main(void){
  int i;
  int fd; //i2cへのファイルディスクリプタ
  int i2cAddress[2];
  float gyroData[3]; //(x,y,z)
  float accelData[3]; //(x,y,z)
  float gyrooffset[3];
  float acceloffset[3];
  float theta[3];
  float prevGyro[3];
  double filtergyroData[3];

  unsigned int timer=0;
  float dt=0;
  float pwm=1000; //pwm
  unsigned int start;

  float ms; //micros()
  float m=0.000001; //micro
  int flag = 1;
  float passed;

  //PD制御
  float kp=100.0,kd=5.0;
  float y=30.0,dy=0.0;

  //gpioピン初期設定
  if ( wiringPiSetupGpio() == -1) {
    printf("setup error");
    exit(1);
  }
  pinMode(MOTOROUT1, OUTPUT);
  pinMode(MOTOROUT2, OUTPUT);
  pinMode(MOTORPWM, PWM_OUTPUT);

  //スレーブアドレス設定
  //L3GD20
  i2cAddress[0] = L3GD20_ADDRESS;
  fd = wiringPiI2CSetup(i2cAddress[0]);
  L3GD20_init(fd);

  for (i = 0; i < 1000; i++) {
    L3GD20_readData(gyroData, fd);
    printf("Gyro: %5.3f, %5.3f, %5.3f\n", gyroData[0], gyroData[1], gyroData[2]);
    if (i < 500){
      digitalWrite(MOTOROUT1, 1);
      digitalWrite(MOTOROUT2, 0);
      pwmWrite(MOTORPWM, pwm);
    }
    else {
      digitalWrite(MOTOROUT2, 1);
      digitalWrite(MOTOROUT1, 0);
      pwmWrite(MOTORPWM, pwm);
    }
    //delay(1000);
  }
  pwm = 0;
  digitalWrite(MOTOROUT1, 0);
  digitalWrite(MOTOROUT2, 0);
  pwmWrite(MOTORPWM, pwm);
  return 0;
}

