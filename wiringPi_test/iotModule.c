//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//sudo　つけて実行すること！！！！！！
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <wiringPi.h>

#include "adxl345_wiringPi.h"
#include "l3gd20_wiringPi.h"

#define L3GD20_ADDRESS 0x6b
#define ADXL345_ADDRESS 0x53
#define MOTOROUT1 14
#define MOTOROUT2 15
#define MOTORPWM 18

//PD制御
float kp=120.0,kd=0.05;
float y=0.0,dy=0.0;

void* iot_module(void* arg){
  //for write data to file /////////////
  FILE *fp;
  char *fname = "gyroData.csv";
  fp = fopen( fname, "w" );
  if( fp == NULL ){
    printf( "%sファイルが開けません¥n", fname );
    exit(1);
  }
  else{
    printf("fileOPEN\n");
  }
  ///////////////////////////

  int i2c_fd; //i2cへのファイルディスクリプタ
  int i2cAddress[2];
  float gyroData[3]; //(x,y,z)
  float accelData[3]; //(x,y,z)
  float gyrooffset[3];
  float acceloffset[3];
  float theta[3];
  float prevGyro[3];
  double dif[3];

  unsigned int timer=0;
  float dt=0;
  float pwm=0; //pwm
  unsigned int start;
  float ms; //micros()
  float m=0.000001; //micro

  pthread_detach(pthread_self());

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
  i2c_fd = wiringPiI2CSetup(i2cAddress[0]);
  L3GD20_init(i2c_fd);

  //adxl345
  i2cAddress[1] = ADXL345_ADDRESS;
  i2c_fd = wiringPiI2CSetup(i2cAddress[1]);
  adxl345_init(i2c_fd);


  int i,j;
  //offset検出
  for (j=0;j<3;j++){
    gyroData[j]=0.0;
    gyrooffset[j] = 0.0;
    acceloffset[j]= 0.0;
    theta[j]=0.0;
    dif[j]=0.0;
  }

  fprintf(fp,"time,theta_x,theta_y,theta_z,d_theta_x,dt_heta_y,d_theta_z,pwm\n");
  //スレーブアドレスをセットしてから読むこと
  //最初500個の平均をoffsetとする
  //動作直後は安定しないので30秒くらい空読みする
  start = micros();
  ms = 0.0; //ここでは経過時間
  while (ms < 15) {
    i2c_fd = wiringPiI2CSetup(i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_fd = wiringPiI2CSetup(i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);
    ms = (micros() - start)*m;
    printf("rest time: %2.3fs\n", ms);
  }
  printf("calibration start\n");
  timer = micros();
  start = micros();
  for (i=0; i< 500; i++) {
    i2c_fd = wiringPiI2CSetup(i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_fd = wiringPiI2CSetup(i2cAddress[1]);
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

  for(i=0;i<3;i++) {
    prevGyro[i] = gyroData[i];
  }
  int phase=0;
  int stop_count=0;
  int wt=0; //wait time
  int prev_pwm=0;
  start = micros();
  timer = micros();
  for (i=0; i< 10000; i++) {
    i2c_fd = wiringPiI2CSetup(i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_fd = wiringPiI2CSetup(i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);
    ms = micros();
    dt = (ms - timer)*m;
    timer = ms;
    for (j=0;j<3;j++){
      gyroData[j] -= gyrooffset[j];
      dif[j] = theta[j];
      theta[j] = theta[j]*0.9998 + prevGyro[j]*0.0053;
      prevGyro[j] = gyroData[j];
      dif[j] = (dif[j] - theta[j])/dt;
    }


    prev_pwm = pwm;
    pwm = kp*(y - theta[0]) + kd*(dy - dif[0]);
    fprintf(fp, "%1.9f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f\n",
    (micros()-start)*m,theta[0],theta[1],theta[2],dif[0],dif[1],dif[2],pwm);
    if (pwm > 0){
      //順回転中はphase == 1
        if (pwm > 1024) pwm = 1024;
        //減速時は一瞬停止させる
        if (pwm < prev_pwm) {
	digitalWrite(MOTOROUT2, 1);
        continue;
        }
        //何ステップかは止めるのに使う
        if (phase <= 0) {
          digitalWrite(MOTOROUT1,1);
          pwmWrite(MOTORPWM,1024);
          if (stop_count==wt){
            phase = 1;
            stop_count = 0;
          }
          else{
            stop_count++;
          }
        }
        else {
          pwmWrite(MOTORPWM, pwm);
          digitalWrite(MOTOROUT2, 0);
        }

    }
    else {
      //逆回転中はphanse == -1
      if (pwm < -1024) pwm = -1024;
      if (-pwm < -prev_pwm) {
	digitalWrite(MOTOROUT1, 1);
        continue;
      }
      if (phase >= 0) {
        //回転が逆になった瞬間はとまるのに１ステップ使う
        digitalWrite(MOTOROUT2, 1);
        pwmWrite(MOTORPWM,1024);
        if (stop_count == wt){
          phase = -1;
          stop_count=0;
        }
        else{
          stop_count++;
        }
      }
      else {
        digitalWrite(MOTOROUT1, 0);
        pwmWrite(MOTORPWM,-pwm);
      }

    }

    printf("Gyro: (%5.2f, %5.2f, %5.2f)\n", gyroData[0],gyroData[1],gyroData[2]);
    // printf("Accel: (%2.5fg, %2.5fg, %2.5fg)\n", accelData[0],accelData[1],accelData[2]);
    printf("Theta: (%5.5f, %5.5f, %5.5f)\n", theta[0],theta[1],theta[2]);
    printf("%5.5f\n",pwm);
    // printf("Time: %1.9f\n", dt);
    // delay(1);
  }

  pwm = 0;
  digitalWrite(MOTOROUT1, 0);
  digitalWrite(MOTOROUT2, 0);
  pwmWrite(MOTORPWM, pwm);

  fclose(fp);
  return ;
}

