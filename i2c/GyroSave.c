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

void L3GD20_readData(int *gyroData, int fd);
void L3GD20_write(unsigned char address, unsigned char data, int fd);
unsigned char L3GD20_read(unsigned char address, int fd);
void L3GD20_init(int fd);

int main(int argc, char **argv){
  int i2c_fd; //i2cへのファイルディスクプタ
  char* i2cFileName = "/dev/i2c-1";//リビジョンに合わせて変更
  int i2cAddress = 0x6a;
  int gyroData[3]; //(x,y,z)
  float x,y,z; //degree per seconds
  FILE *fp;
  char *fname = "gyroData.csv";


  printf("i2c Gyro(L3GD20) test program\n");
  delay(500);
  // I2Cデバイスファイルをオープン
  if ((i2c_fd = open(i2cFileName, O_RDWR)) < 0){
    printf("Faild to open i2c port\n");
    exit(1);
  }
  //L3GD20用に設定
  if((ioctl(i2c_fd, I2C_SLAVE, i2cAddress)) < 0){
    printf("Unable to get bus access to talk to slave\n");
    exit(1);
  }
  //デバイス初期化
  L3GD20_init(i2c_fd);

  fp = fopen( fname, "w" );
  if( fp == NULL ){
    printf( "%sファイルが開けません¥n", fname );
    return -1;
  }
  fprintf(fp, "%c,%c,%c",'x','y','z');
  //１ms毎,２０回センサ情報取得
  int i;
  for (i=0;i<1000;i++){
    L3GD20_readData(gyroData, i2c_fd);
    //データを校正して表示
    x = (float)gyroData[0]*0.00875;
    y = (float)gyroData[1]*0.00875;
    z = (float)gyroData[2]*0.00875;
    printf("(x, y, z) time = (%5.2f, %5.2f, %5.2f) \n", x,y,z);
    fprintf(fp, "%5.2f, %5.2f, %5.2f\n", x,y,z);
    delay(1);
  }

  fclose(fp);
  return;
}


//L3GD20用１byte書き込みルーチン:addressで示すレジスタにdataを書き込む
void L3GD20_write(unsigned char address, unsigned char data, int fd){
  unsigned char buf[2];
  buf[0] = address;
  buf[1] = data;
    if ((write(fd, buf, 2)) != 2){
    printf("Error writing to i2c slave\n");
    exit(1);
  }
  return;
}

//L3GD20用1byte読み込み用ルーチン:addressで示すレジスタの値を読み出す
//戻り値がレジスタ値
unsigned char L3GD20_read(unsigned char address, int fd){
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

//L3GD20用ジャイロデータ読み出し用ルーチン
//整数値配列へのポインタを使ってデータを読みだす
void L3GD20_readData(int *gyroData, int fd){
  unsigned char data[6];
  //センサから３軸に対して２バイトずつデータを読み出す
  int i;
  for (i=0;i<6;i++){
    data[i] = L3GD20_read(0x28+i,fd);
  }
  //各数値をint型に整形する
  //センサの値が16bitかつ２の補数表現の出力のため、bitシフトで加工
  //符号も残すため、右シフトを必ず最後に一度用いること！
  //example b10=-1 としたいとき。 data[0]=0,data[1]=1とする
  //(data[1]<<1 | data[0]) == 10(2bit) == 0010(4bit) == 2
  //(data[1]<<2 | data[0]<<1) >>1 == 10(2bit) == 1110(4bit) == 2
  //右シフトにより、左側の桁数を符号bitで埋められる
  gyroData[0] = ((int)data[1]<<24|(int)data[0]<<16)>>16;
  gyroData[1] = ((int)data[3]<<24|(int)data[2]<<16)>>16;
  gyroData[2] = ((int)data[5]<<24|(int)data[4]<<16)>>16;
  return;
}

//L3GD20初期化ルーチン
void L3GD20_init(int fd){
  unsigned char data;
  printf("L3GD20 initialize seqence start\n");
  //L3GD20はレジスタ0x0fに常に0xd4がセットされている仕様なので
  //それを利用して通信チェックする
  data = L3GD20_read(0x0f, fd);
  if (data != 0xd4){
    printf("L3GD20 is not working\n");
    exit(1);
  }
  delay(10);
  //レジスタへの書き込みチェックとイニシャライズを同時に行う
  printf("read OK. Now writing check ...\n");
  //0x20に0x0fを書き込むことでパワーオン。動作可能に
  L3GD20_write(0x20, 0x0f, fd);
  //0x20に正しく0x0fが書き込まれたか確認
  data = L3GD20_read(0x20, fd);
  if (data != 0x0f){
    printf("writing error\n");
    exit(1);
  }
  printf("Working OK\n");
  delay(10);
  return;
}
