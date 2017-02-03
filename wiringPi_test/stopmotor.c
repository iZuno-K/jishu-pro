#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#define MOTOROUT1 14
#define MOTOROUT2 15
#define MOTORPWM 18

int main(){

  //gpioピン初期設定
  if ( wiringPiSetupGpio() == -1) {
    printf("setup error");
    exit(1);
  }

  pinMode(MOTOROUT1, OUTPUT);
  pinMode(MOTOROUT2, OUTPUT);
  pinMode(MOTORPWM, PWM_OUTPUT);

  digitalWrite(MOTOROUT1, 0);
  digitalWrite(MOTOROUT2, 0);
  pwmWrite(MOTORPWM, 0);

  printf("stop motor\n");
  return 0;
}

