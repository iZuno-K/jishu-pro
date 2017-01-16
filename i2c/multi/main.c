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

#include "adxl345.h"
#include "l3gd20.h"
#include "i2c.h"

#define L3GD20_ADDRESS 0x6a
#define ADXL345_ADDRESS 0x53
#define MOTOROUT1 14
#define MOTOROUT2 15
#define MOTORPWM 18

void* motor_thread(void+ arg);

int main(void){
  int i2c_fd; //i2cへのファイルディスクリプタ
  char *i2cFileName = "/dev/i2c-1"; //リビジョンに合わせてかえること
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
  float pwm=0; //pwm
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
    filtergyroData[j]=0.0;
  }

  //スレーブアドレスをセットしてから読むこと
  //最初500個の平均をoffsetとする
  //動作直後は安定しないので５００回くらい空読みする
  for (i=0; i< 500; i++) {
    i2c_setAddress(&i2c_fd, i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_setAddress(&i2c_fd, i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);
  }
  timer = micros();
  start = micros();
  for (i=0; i< 500; i++) {
    i2c_setAddress(&i2c_fd, i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_setAddress(&i2c_fd, i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);
    ms = micros();
    dt = (ms - timer) * m;
    timer = ms;
    for (j=0;j<3;j++){
      gyrooffset[j] += gyroData[j];
      acceloffset[j]+= accelData[j];
      // a[j] += gyroData[j]*dt;
    }
  }
  dt = (micros() - start)*m;
  for (j=0;j<3;j++){
    gyrooffset[j] = gyrooffset[j] / 500.0;
    acceloffset[j]= acceloffset[j] / 500.0;
    // a[j] /= dt;
  }
  acceloffset[2] -= 1.0; //重力の分
  printf("(%5.2f, %5.2f, %5.2f)\n", gyrooffset[0],gyrooffset[1],gyrooffset[2]);
  printf("(%5.2fg, %5.2fg, %5.2fg)\n", acceloffset[0],acceloffset[1],acceloffset[2]);
  delay(500);

  digitalWrite(MOTOROUT1, 1);
  flag=1;
  // start = micros();
  for(i=0;i<3;i++) {
    prevGyro[i] = gyroData[i];
  }
  timer = micros();
  for (i=0; i< 10000; i++) {
    i2c_setAddress(&i2c_fd, i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_setAddress(&i2c_fd, i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);
    ms = micros();
    dt = (ms - timer)*m;
    timer = ms;
    for (j=0;j<3;j++){ji
      gyroData[j] -= gyrooffset[j];
      //50Hz low pass
      // gyroData[j] = prevGyro[j]*0.3679 + gyroData[j]*0.6321;
      filtergyroData[j] = filtergyroData[j] + gyroData[j] - prevGyro[j]*0.999371878820;
      prevGyro[j] = gyroData[j];
      theta[j] += filtergyroData[j]*dt;
      // theta[j] += gyroData[j]*dt - a[j]*passed- b[j];
    }


    pwm = kp*(theta[2] - y) + kd*(filtergyroData[2] - dy);
    printf("%5.5f\n",pwm);

    printf("Gyro: (%5.2f, %5.2f, %5.2f)\n", filtergyroData[0],filtergyroData[1],filtergyroData[2]);
    printf("Accel: (%2.5fg, %2.5fg, %2.5fg)\n", accelData[0],accelData[1],accelData[2]);
    printf("Theta: (%5.5f, %5.5f, %5.5f)\n", theta[0],theta[1],theta[2]);
    printf("Time: %1.9f\n", dt);
    // delay(1);
  }

  pwm = 0;
  digitalWrite(MOTOROUT1, 0);
  digitalWrite(MOTOROUT2, 0);
  pwmWrite(MOTORPWM, pwm);
  printf("fileCLOSE\n");
  return ;
}


void* motor_thread(void* arg){

  while (1) {
    pthread_mutex_lock(&g_mutex):
    {
      if (pwm > 0){
          digitalWrite(MOTOROUT1, 1);
          digitalWrite(MOTOROUT2, 0);
          pwmWrite(MOTORPWM, pwm);
        } else {
          digitalWrite(MOTOROUT2, 1);
          digitalWrite(MOTOROUT1, 0);
          pwmWrite(MOTORPWM,-pwm);
        }
    }
    pthread_mutex_unlock(&g_mutex);
  }
}
