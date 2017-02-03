#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#define MOTOROUT1 14

int main()
{
  int fd,ret;
  int ID;

  ID = 0x6b;

  /* WHO AM I */
  fd = wiringPiI2CSetup(ID);
  printf("setup return : %d\n",fd);

  /* start senser */
  if((wiringPiI2CWriteReg8(fd,0x20,0x0F))<0){
  printf("write error register 0x20");
  }
  printf("write register:0x20 = 0x0F\n");

  if ( wiringPiSetupGpio() == -1) {
    printf("setup error");
    //exit(1);
  }
  pinMode(MOTOROUT1, OUTPUT);

  int flag = 1;
  while(1) {

    if (flag==1) {
      digitalWrite(MOTOROUT1, 1);
      flag = 0;
    }
    else {
      digitalWrite(MOTOROUT1, 0);
      flag = 1;
    }


    /* read OUT_X_L */
    ret = wiringPiI2CReadReg8(fd,0x28);
    printf("OUT_X_L : %d\n",ret);

  }

return;
}

