//g++ -o motorTest motorTest.cpp -lwiringPi
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//sudo　つけて実行すること！！！！！！
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>

#define MOTOROUT1 14
#define MOTOROUT2 15
#define MOTORPWM 18
#define POW 1024

int main(void) {
int i = 0;
double time = 1000; //micro sec

  if ( wiringPiSetupGpio() == -1) {
    printf("setup error");
    return 1;
  }

  pinMode(MOTOROUT1, OUTPUT);
  pinMode(MOTOROUT2, OUTPUT);
  pinMode(MOTORPWM, PWM_OUTPUT);

  digitalWrite(MOTOROUT1, 0);
  digitalWrite(MOTOROUT2, 0);
  digitalWrite(MOTORPWM, 0);

  printf("forward\n");
  digitalWrite(MOTOROUT1, 1);
  pwmWrite(MOTORPWM, POW);
  usleep(time);

  pwmWrite(MOTORPWM, 0);
  digitalWrite(MOTOROUT1, 0);
  usleep(50000);

  printf("back\n");
  digitalWrite(MOTOROUT2, 1);
  pwmWrite(MOTORPWM, POW);
  usleep(time);

  pwmWrite(MOTORPWM, 0);
  digitalWrite(MOTOROUT2, 0);
  printf("STOP\n");
  usleep(time);

  return 0;
}
