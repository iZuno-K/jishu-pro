#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <wiringPi.h>


#define RSpin 5 // レジスタ選択ピン指定
#define REpin 6// リセット信号ピン指定

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static uint8_t SPIMode = 3,SPIbit=8;   //SPIモード3 bit数8
static uint32_t SPISpeed = 20000000;  // 20MHz

// SPIデバイス定義
// # ls -l /dev/spidev* で確認できる。
// Pi2の場合CE1に接続した場合は0.0
//          CE2に接続した場合は0.1 だった。
static char *SPIDevice = "/dev/spidev0.0";
int fd,ret=0;

int main(){

  // wiringPi初期化
  if(wiringPiSetup()==-1) printf("wiringPi初期化エラー\n");

  // SPIデバイスの初期化
  fd = open(SPIDevice,O_RDWR);if(fd<0) printf("SPI デバイス初期化エラー\n");
  ret = ioctl(fd,SPI_IOC_WR_MODE,&SPIMode);if(ret <0) printf("SPI Mode設定エラー\n");
  ret =ioctl(fd,SPI_IOC_WR_BITS_PER_WORD,&SPIbit);if(ret <0) printf("SPI bit/Word設定エラー\n");
  ret =ioctl(fd,SPI_IOC_WR_MAX_SPEED_HZ,&SPISpeed);if(ret <0) printf("SPI Speed設定エラー\n");

  int rt;
  rt = DataSend(0,0x0f);
  printf("%d\n", rt);
  close(fd);

}

// LCDへのデータ転送
// DataSend(RS,TrData);
// RS＝レジスタ選択
//     0＝コマンド  1＝データ
// TrData＝転送するデータ
//         8bit値
int DataSend(int RS,unsigned long TrData){
  digitalWrite(RSpin, RS); // RS信号切替
	uint8_t ReData;         //受信用変数

	// 転送するデータの準備
	struct spi_ioc_transfer tr={
	.tx_buf       =(unsigned long)&TrData,
	.rx_buf       =(unsigned long)&ReData,
	.len          = 1,
	.delay_usecs  = 1,
	.speed_hz     = SPISpeed,
	.bits_per_word= SPIbit,
	};

	// 転送
	ret =ioctl(fd,SPI_IOC_MESSAGE(1),&tr);
	if(ret<1){printf("SPI 転送エラー\n");exit(1);}
	return ReData; // 受信データを返す
}
