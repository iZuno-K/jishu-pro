#include "mraa.h"
#include <stdio.h>

#define L3GD20_ADDR 0x6a


void main() {
  //i2c
  mraa_i2c_context i2c;
  uint8_t buf[1]; //input buffer
  mraa_init();
  printf("init\n");
  i2c = mraa_i2c_init(1);
  if (i2c == NULL) {
    printf("i2c init error\n");
    exit(1);
  }
  printf("object made\n");
  mraa_i2c_address(i2c, L3GD20_ADDR);
  //L3GD20init
  buf[0] = 0x0f;
  if ((mraa_i2c_write(i2c, buf, 1)) != 1){
    printf("Error writing to i2c slave\n");
    exit(1);
  }
  if ((mraa_i2c_read(i2c, buf, 1)) != 1){
    printf("Error reading to i2c slave\n");
    exit(1);
  }
  printf("%u\n", buf);

}
