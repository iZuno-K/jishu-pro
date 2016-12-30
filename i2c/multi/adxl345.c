// gcc -o i2cAccel i2cAccel.c -lwiringPi
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <linux/i2c-dev.h> //I2C用インクルード
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <wiringPi.h> //delay関数用

#include "i2c.h"
#include "adxl345.h"

//各レジスタ
#define BW_RATE     0x2c
#define POWER_CTL   0x2d
#define INT_SOURCE  0x30
#define DATA_FORMAT 0x31
#define DATAX0      0x32

//adxl345用ジャイロデータ読み出し用ルーチン
//整数値配列へのポインタを使ってデータを読みだす
void adxl345_readData(float *accelData, int fd){
  unsigned char data[6];
  //センサから３軸に対して２バイトずつデータを読み出す
  int i;
  int rawData[3];
  float accuracy;
  //測定範囲が±16gのとき
  //10bitの2の補数表現で、先頭が符号ビットなので16.0 / 2^(10-1)
  accuracy = 16.0 / 512;
  for (i=0;i<6;i++){
    data[i] = i2c_read(0x32+i,fd);
  }
  //各数値をint型に整形する
  //センサの値が16bitかつ２の補数表現の出力のため、bitシフトで加工
  //符号も残すため、右シフトを必ず最後に一度用いること！
  //example b10=-1 としたいとき。 data[0]=0,data[1]=1とする
  //(data[1]<<1 | data[0]) == 10(2bit) == 0010(4bit) == 2
  //(data[1]<<2 | data[0]<<1) >>1 == 10(2bit) == 1110(4bit) == 2
  //右シフトにより、左側の桁数を符号bitで埋められる
  rawData[0] = ((int)data[1]<<24|(int)data[0]<<16)>>16;
  rawData[1] = ((int)data[3]<<24|(int)data[2]<<16)>>16;
  rawData[2] = ((int)data[5]<<24|(int)data[4]<<16)>>16;

  accelData[0] = rawData[0] * accuracy;
  accelData[1] = rawData[1] * accuracy;
  accelData[2] = rawData[2] * accuracy;
  return;

}

//adxl345初期化ルーチン
void adxl345_init(int fd){
  unsigned char data;
  printf("adxl345 initialize seqence start\n");
  //adxl345のPOWER_CTLをMeasureモードに
  i2c_write(POWER_CTL, 0x08, fd);
  delay(10);
  //測定範囲±16g
  i2c_write(DATA_FORMAT, 0x03, fd);
  delay(10);
  //サンプリング周波数adxl345.h参照
  i2c_write(BW_RATE, 0x0A, fd);
  delay(10);
  printf("adxl345 initialize seqence finish\n");
  return;
}
