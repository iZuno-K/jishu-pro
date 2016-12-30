// gcc -o i2cGyro i2cGyro.c -lwiringPi
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
#include "l3gd20.h"

//L3GD20用ジャイロデータ読み出し用ルーチン
//整数値配列へのポインタを使ってデータを読みだす
void L3GD20_readData(float *gyroData, int fd){
  unsigned char data[6];
  //センサから３軸に対して２バイトずつデータを読み出す
  int i;
  int rawData[3];
  for (i=0;i<6;i++){
    data[i] = i2c_read(0x28+i,fd);
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

  //精度をかけて実際の値に補正する。
  //initの仕方によって精度は変更可能
  gyroData[0] = rawData[0]*0.00875;
  gyroData[1] = rawData[1]*0.00875;
  gyroData[2] = rawData[2]*0.00875;
  return;
}

//L3GD20初期化ルーチン
void L3GD20_init(int fd){
  unsigned char data;
  printf("L3GD20 initialize seqence start\n");
  //L3GD20はレジスタ0x0fに常に0xd4がセットされている仕様なので
  //それを利用して通信チェックする
  data = i2c_read(0x0f, fd);
  if (data != 0xd4){
    printf("L3GD20 is not working\n");
    exit(1);
  }
  delay(10);
  //レジスタへの書き込みチェックとイニシャライズを同時に行う
  printf("read OK. Now writing check ...\n");
  //0x20に0x0fを書き込むことでパワーオン。動作可能に
  i2c_write(0x20, 0x0f, fd);
  //0x20に正しく0x0fが書き込まれたか確認
  data = i2c_read(0x20, fd);
  if (data != 0x0f){
    printf("writing error\n");
    exit(1);
  }
  printf("Working OK\n");
  delay(10);
  return;
}
