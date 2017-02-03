#include <stdio.h>
#include <stdlib.h>

#include <wiringPi.h>

#define MOTOROUT1 14
#define MOTOROUT2 15
#define MOTORPWM 18
int pwm=1024;

int main() {
  //gpioピン初期設定
  if ( wiringPiSetupGpio() == -1) {
    printf("setup error");
    exit(1);
  }
  pinMode(MOTOROUT1, OUTPUT);
  pinMode(MOTOROUT2, OUTPUT);
  pinMode(MOTORPWM, PWM_OUTPUT);

  printf("forward\n");
  digitalWrite(MOTOROUT1, 1);
  digitalWrite(MOTOROUT2, 0);
  pwmWrite(MOTORPWM, pwm);
  delay(5000);

  printf("backward\n");
  digitalWrite(MOTOROUT2, 1);
  digitalWrite(MOTOROUT1, 0);
  pwmWrite(MOTORPWM, pwm);
  delay(5000);


  digitalWrite(MOTOROUT1, 0);
  digitalWrite(MOTOROUT2, 0);
  pwmWrite(MOTORPWM, 0);

  return 0;

}

