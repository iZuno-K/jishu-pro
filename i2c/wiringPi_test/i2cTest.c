#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

int main()
{
  int fd,ret;
  int ID;

  ID = 0x6a;

  /* WHO AM I */
  fd = wiringPiI2CSetup(ID);
  printf("setup return : %d\n",fd);

  /* start senser */
  if((wiringPiI2CWriteReg8(fd,0x20,0x0F))<0){
  printf("write error register 0x20");
  }
  printf("write register:0x20 = 0x0F\n");

  /* read OUT_X_L */
  ret = wiringPiI2CReadReg8(fd,0x28);
  printf("OUT_X_L : %d\n",ret);

  /* read OUT_X_H */
  ret = wiringPiI2CReadReg8(fd,0x29);
  printf("OUT_X_H : %d\n",ret);

  /* read OUT_Y_L */
  ret = wiringPiI2CReadReg8(fd,0x2A);
  printf("OUT_Y_L : %d\n",ret);

  /* read OUT_Y_H */
  ret = wiringPiI2CReadReg8(fd,0x2B);
  printf("OUT_Y_H : %d\n",ret);

  /* read OUT_Z_L */
  ret = wiringPiI2CReadReg8(fd,0x2C);
  printf("OUT_Z_L : %d\n",ret);

  /* read OUT_Z_H */
  ret = wiringPiI2CReadReg8(fd,0x2D);
  printf("OUT_Z_H : %d\n",ret);

return;
}
