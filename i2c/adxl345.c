#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "adxl345.h"

#define BW_RATE     0x2c
#define POWER_CTL   0x2d
#define INT_SOURCE  0x30
#define DATA_FORMAT 0x31
#define DATAX0      0x32

#define DATA_READY 0x80

static int fd;

static int adxl345_open(char *device, char sdo)
{
  if ((fd = open(device, O_RDWR)) < 0) {
    printf("Faild to open i2c port\n");
    exit(1);
  }

  if (ioctl(fd, I2C_SLAVE, sdo) < 0) {
    printf("Unable to get bus access to talk to slave\n");
    exit(1);
  }

  return fd;
}

static void adxl345_write(unsigned char addr, unsigned char data)
{
  unsigned char buf[2];

  buf[0] = addr;
  buf[1] = data;

  if (write(fd, buf, 2) != 2) {
    fprintf(stderr, "Error writing to i2c slave\n");
    exit(1);
  }
}

static void adxl345_read(unsigned char addr, int num_byte, unsigned char *data)
{
  int i;

  for (i = 0; i < num_byte; ++i, ++addr) {
    /* write address */
    if (write(fd, &addr, 1) != 1) {
      fprintf(stderr, "Error writing to i2c slave\n");
      exit(1);
    }

    /* read data */
    if (read(fd, &data[i], 1) != 1) {
      fprintf(stderr, "Error reading from i2c slave\n");
      exit(1);
    }
  }
}

void adxl345_init(char *device, char sdo, adxl345_datarate rate)
{
  adxl345_open(device, sdo);
  adxl345_write(POWER_CTL, 0x08); // 測定モード
  adxl345_write(DATA_FORMAT, 0x03); // 16g
  adxl345_write(BW_RATE, rate); 
}

void adxl345_default_init(void)
{
  adxl345_init(I2C_DEVICE_0, ADXL345_ADDR_LOW, ADXL345_DATARATE_12_5_HZ);
}

three_d_space* adxl345_get_acceleration(void)
{
  static three_d_space acceleration;
  unsigned char values[6];

  for (values[0] = 0; !(values[0] & DATA_READY); ) {
    adxl345_read(INT_SOURCE, 1, values);
  }

  adxl345_read(DATAX0, 6, values);

  acceleration.x  = ((int) values[1] << 8) | (int) values[0];
  acceleration.y  = ((int) values[3] << 8) | (int) values[2];
  acceleration.z  = ((int) values[5] << 8) | (int) values[4];

  return &acceleration;
}

void adxl345_finish(void)
{
  close(fd);
}
