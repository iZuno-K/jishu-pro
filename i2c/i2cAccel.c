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

void readData(int *accelData, int fd);
void adxl345_write(unsigned char address, unsigned char data, int fd);
unsigned char adxl345_read(unsigned char address, int fd);
void adxl345_init(int fd);

//各レジスタ
#define BW_RATE     0x2c
#define POWER_CTL   0x2d
#define INT_SOURCE  0x30
#define DATA_FORMAT 0x31
#define DATAX0      0x32

int main(int argc, char **argv){
  int i2c_fd; //i2cへのファイルディスクプタ
  char* i2cFileName = "/dev/i2c-1";//リビジョンに合わせて変更
  int i2cAddress = 0x53;
  int accelData[3]; //(x,y,z)

  printf("i2c accelalation(adxl345) test program\n");
  delay(500);
  // I2Cデバイスファイルをオープン
  if ((i2c_fd = open(i2cFileName, O_RDWR)) < 0){
    printf("Faild to open i2c port\n");
    exit(1);
  }
  //adxl345用に設定
  if((ioctl(i2c_fd, I2C_SLAVE, i2cAddress)) < 0){
    printf("Unable to get bus access to talk to slave\n");
    exit(1);
  }
  //デバイス初期化
  adxl345_init(i2c_fd);

  int i;
  for (i=0;i<40;i++){
    readData(accelData, i2c_fd);
    //データを校正して表示
    printf("(x, y, z) = (%5.2f, %5.2f, %5.2f) \n",
    (float)accelData[0]*0.00875, (float)accelData[1]*0.00875, (float)accelData[2]*0.00875);
    delay(100);
  }
  return;
}


//adxl345用１byte書き込みルーチン:addressで示すレジスタにdataを書き込む
void adxl345_write(unsigned char address, unsigned char data, int fd){
  unsigned char buf[2];
  buf[0] = address;
  buf[1] = data;
  if ((write(fd, buf, 2)) != 2){
    printf("Error writing to i2c slave\n");
    exit(1);
  }
  return;
}

//adxl345用1byte読み込み用ルーチン:addressで示すレジスタの値を読み出す
//戻り値がレジスタ値
unsigned char adxl345_read(unsigned char address, int fd){
  unsigned char buf[1];
  buf[0] = address;
  //まずはアドレスを一度書き込む！
  if ((write(fd, buf, 1)) != 1) {
    printf("Error writing to i2c slave\n");
    exit(1);
  }
  if ((read(fd, buf, 1)) != 1){
    printf("Error reading to i2c slave\n");
    exit(1);
  }
  return buf[0];
}

//adxl345用ジャイロデータ読み出し用ルーチン
//整数値配列へのポインタを使ってデータを読みだす
void readData(int *accelData, int fd){
  unsigned char data[6];
  //センサから３軸に対して２バイトずつデータを読み出す
  int i;
  for (i=0;i<6;i++){
    data[i] = adxl345_read(0x32+i,fd);
  }
  //各数値をint型に整形する
  //センサの値が16bitかつ２の補数表現の出力のため、bitシフトで加工
  //符号も残すため、右シフトを必ず最後に一度用いること！
  //example b10=-1 としたいとき。 data[0]=0,data[1]=1とする
  //(data[1]<<1 | data[0]) == 10(2bit) == 0010(4bit) == 2
  //(data[1]<<2 | data[0]<<1) >>1 == 10(2bit) == 1110(4bit) == 2
  //右シフトにより、左側の桁数を符号bitで埋められる
  accelData[0] = ((int)data[1]<<24|(int)data[0]<<16)>>16;
  accelData[1] = ((int)data[3]<<24|(int)data[2]<<16)>>16;
  accelData[2] = ((int)data[5]<<24|(int)data[4]<<16)>>16;
  return;
}

//adxl345初期化ルーチン
void adxl345_init(int fd){
  unsigned char data;
  printf("adxl345 initialize seqence start\n");
  //adxl345のPOWER_CTLをMeasureモードに
  adxl345_write(POWER_CTL, 0x08, fd);
  delay(10);sudo
  //測定範囲±16g
  adxl345_write(DATA_FORMAT, 0x03, fd);
  delay(10);
  //サンプリング周波数adxl345.h参照
  adxl345_write(BW_RATE, 0x0A, fd);
  delay(10);
  printf("adxl345 initialize seqence finish\n");
  return;
}
