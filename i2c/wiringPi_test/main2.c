//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//sudo　つけて実行すること！！！！！！
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//theta filtered_theta(ハイパス・積分) thetasimple(ハイパス・積分のsを約分)
#include <stdio.h>
#include <stdlib.h>

#include <wiringPi.h>

#include "adxl345_wiringPi.h"
#include "l3gd20_wiringPi.h"

#define L3GD20_ADDRESS 0x6b
#define ADXL345_ADDRESS 0x53
#define MOTOROUT1 14
#define MOTOROUT2 15
#define MOTORPWM 18

//PD制御
float kp=135.0,kd=5.0;
float y=0.0,dy=0.0;

int main(void){
  FILE *fp;
  char *fname = "gyroData.csv";
  fp = fopen( fname, "w" );
  if( fp == NULL ){
    printf( "%sファイルが開けません¥n", fname );
    return -1;
  }
  else{
    printf("fileOPEN\n");
  }

  int i2c_fd; //i2cへのファイルディスクリプタ
  int i2cAddress[2];
  float gyroData[3]; //(x,y,z)
  float accelData[3]; //(x,y,z)
  float gyrooffset[3];
  float acceloffset[3];
  float theta[3];
  float filtered_theta[3];
  float prevGyro[3];
  double filtergyroData[3];
  double predict[3];
  double thetasimple[3];

  unsigned int timer=0;
  float dt=0;
  float pwm=0; //pwm
  unsigned int start;
  float ms; //micros()
  float m=0.000001; //micro

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
    filtered_theta[j]=0.0;
    filtergyroData[j]=0.0;
    predict[j]=0.0;
    thetasimple[j]=0.0;
  }

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
    printf("rest time: %2.3fs\n", 15-ms);
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

  for (i = 0; i < 3; i++) {
    prevGyro[i] = gyroData[i];
    filtergyroData[i] = gyroData[i];
  }

  fprintf(fp,"time,theta_x,theta_y,theta_z,filtered_theta_x,filtered_theta_y,filtered_theta_z,thetasimple_x,thetasimple_y,thetasimple_z,omega_x,omega_y,omega_z");
  for(i=0;i<3;i++) {
    prevGyro[i] = gyroData[i];
  }
  start = micros();
  timer = micros();
  for (i=0; i< 5000; i++) {
    i2c_fd = wiringPiI2CSetup(i2cAddress[0]);
    L3GD20_readData(gyroData, i2c_fd);
    i2c_fd = wiringPiI2CSetup(i2cAddress[1]);
    adxl345_readData(accelData, i2c_fd);
    ms = micros();
    dt = (ms - timer)*m;
    timer = ms;
    for (j=0;j<3;j++){
      gyroData[j] -= gyrooffset[j];
      //0.03カット　ハイパスフィルタ
      // filtergyroData[j] = filtergyroData[j]*0.999371878820 + gyroData[j] - prevGyro[j];
      filtergyroData[j] = filtergyroData[j]*0.9998 + gyroData[j] - prevGyro[j];
      prevGyro[j] = gyroData[j];
      filtered_theta[j] += filtergyroData[j]*dt;
      theta[j] += gyroData[j]*dt;
      thetasimple[j] = thetasimple[j]*0.9998 + prevGyro[j]*0.005999;
    }

    //
    // pwm = kp*(y - filtered_theta[0]) + kd*(dy - filtergyroData[0]);
    // printf("%5.5f\n",pwm);
    // if (pwm > 0){
    //     digitalWrite(MOTOROUT1, 1);
    //     digitalWrite(MOTOROUT2, 0);
    //     pwmWrite(MOTORPWM, pwm);
    //   } else {
    //     digitalWrite(MOTOROUT2, 1);
    //     digitalWrite(MOTOROUT1, 0);
    //     pwmWrite(MOTORPWM,-pwm);
    //   }

    printf("Gyro: (%5.2f, %5.2f, %5.2f)\n", gyroData[0],gyroData[1],gyroData[2]);
    printf("fijlterd_Gyro: (%5.2f, %5.2f, %5.2f)\n", filtergyroData[0],filtergyroData[1],filtergyroData[2]);
    printf("Accel: (%2.5fg, %2.5fg, %2.5fg)\n", accelData[0],accelData[1],accelData[2]);
    printf("Theta: (%5.5f, %5.5f, %5.5f)\n", theta[0],theta[1],theta[2]);
    printf("filtered_theta: (%5.5f, %5.5f, %5.5f)\n", filtered_theta[0],filtered_theta[1],filtered_theta[2]);
    printf("thetasimplefilter: (%5.5f, %5.5f, %5.5f)\n", thetasimple[0],thetasimple[1],thetasimple[2]);
    printf("Time: %1.9f\n", dt);
    fprintf(fp, "%1.9f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,%5.5f,\n",
    (micros()-start)*m,theta[0],theta[1],theta[2],filtered_theta[0],filtered_theta[1],filtered_theta[2],thetasimple[0],thetasimple[1],thetasimple[2],filtergyroData[0],filtergyroData[1],filtergyroData[2]);
    // delay(1);
  }

  pwm = 0;
  digitalWrite(MOTOROUT1, 0);
  digitalWrite(MOTOROUT2, 0);
  pwmWrite(MOTORPWM, pwm);

  return ;
}
