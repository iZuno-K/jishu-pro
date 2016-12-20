//g_から始まるのはglobal変数
//169行目あたりの「//」を消すと端末上に他クライアントのメッセージが表示される
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "defs.h"

#include "opencv2/opencv.hpp"

static int g_socket;//このクライアントのソケット

static pthread_t g_tr, g_ts;
static pthread_mutex_t g_mutex;
static int g_width=640, g_height=480, g_depth=8, g_nChannel=3;
static int g_lena_width=640, g_lena_height=480, g_lena_depth=8, g_lena_nChannel=3;
static char g_image_buf[640*480*3];
static int g_switch = 1; //for logout

//-----------------------------------------------------------------------------------
// socket_open
//-----------------------------------------------------------------------------------
void
socket_open (const char* ip_addr)
{
  struct sockaddr_in sin;

  // ソケットを生成する
  //---------------------------------------------------
  if ((g_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("client: socket");
    exit(1);
  }

  // サーバの情報を与える
  //---------------------------------------------------
  sin.sin_family = AF_INET;  // アドレスの型の指定
  sin.sin_port = PORT;  // ポート番号
  sin.sin_addr.s_addr = inet_addr(ip_addr);  // 宛先のアドレスの指定 IPアドレスの文字列を変換

  // サーバに接続する
  //---------------------------------------------------
  if ((connect(g_socket, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("client: connect");
    exit(1);
  }
}

//-----------------------------------------------------------------------------------
// socket_close 使ってない
//-----------------------------------------------------------------------------------
void
socket_close()
{
  close( g_socket );
}

//-----------------------------------------------------------------------------------
// socket_read
//-----------------------------------------------------------------------------------
int
socket_read( int socket, void* buf, int buf_size )
{
  int size;
  int left_size = buf_size;
  char* p = (char*) buf;

  // 指定されたサイズのバッファを受信完了するまで、
  // ループで読み込み続ける
  //---------------------------------------------------
  while( left_size > 0 ) {
    size = read(socket, p, left_size);
    p += size;
    left_size -= size;

    if( size <= 0 ) break;
  }

  return buf_size - left_size;
}

//-----------------------------------------------------------------------------------
// socket_write
//-----------------------------------------------------------------------------------
int
socket_write( int socket, void* buf, int buf_size )
{
  int size;
  int left_size = buf_size;
  char* p = (char*) buf;

  // 指定されたサイズのバッファを送信完了するまで、
  // ループで書き込み続ける
  //---------------------------------------------------
  while( left_size > 0 ) {
    size = write(socket, p, left_size);
    p += size;
    left_size -= size;

    if( size <= 0 ) break;
  }

  return buf_size - left_size;
}

//-----------------------------------------------------------------------------------
// send_sock
//-----------------------------------------------------------------------------------
void*
thread_send(void *arg)
{
  char buf[1024];
  printf("chat start\n");

  while(1){
    fgets(buf, sizeof(buf), stdin);
    buf[strlen(buf)-1] = '\0'; //改行文字消す
    int size = strlen(buf)+1;

    if ( strcmp(buf,"_logout") == 0 ) g_switch= 0;//for logout

    if (socket_write( g_socket, &size, sizeof(int) ) < 0){
      return NULL;
    }
    if (socket_write( g_socket, buf, size ) < 0){
      return NULL;
    }
  }

  return NULL;
}

//-----------------------------------------------------------------------------------
// recv_sock
//-----------------------------------------------------------------------------------
void*
thread_recv(void *arg)
{
  char buf[1024];
  int size;

  socket_read(g_socket, &g_width, sizeof(int) );
  socket_read(g_socket, &g_height, sizeof(int) );
  socket_read(g_socket, &g_depth, sizeof(int) );
  socket_read(g_socket, &g_nChannel, sizeof(int) );


  while(1){

    if(g_switch){
      socket_read(g_socket, &size, sizeof(int));
      // ダミーではないとき
      if( size > 0 ) {
	socket_read(g_socket, buf, size );
	printf("%s\n", buf);

      } else {//画像受信
	socket_read(g_socket, g_image_buf, g_width*g_height*(g_depth/8)*g_nChannel);
	//image_buf, width*nChannel
      }
    } else{ //after input logout
      socket_read(g_socket, &size, sizeof(int));
      if (size == 0){
	socket_read(g_socket, g_image_buf, size);
	break;
	  }
      else socket_read(g_socket, buf, size );
    }
  }
  socket_close();
  return NULL;
}

//-----------------------------------------------------------------------------------
// keyboard
//-----------------------------------------------------------------------------------
/* ARGSUSED1 */
void keyboard(unsigned char key, int x, int y)
{
  //Escキーで終了(_logoutと打ち込んでも終了できる...はず)
    switch (key) {
    case 27:
	socket_close();
	exit(0);
	break;
    }
}

//-----------------------------------------------------------------------------------
// main
//-----------------------------------------------------------------------------------
int
main(int argc, char *argv[])
{
  int c;
  cv::VideoCapture cap(0);//デバイスのオープン
  if(!cap.isOpened()){//カメラデバイスが正常にオープンしたか確認．
      //読み込みに失敗したときの処理
      return -1;
  }

  if(argc == 1){
    printf("usage: client <server IP address>\n");
    exit(1);
  }

  // サーバーへ接続
  //---------------------------------------------------
  socket_open( argv[1] );

  pthread_mutex_init( &g_mutex, NULL );
  pthread_create( &g_tr, NULL, thread_recv, NULL );
  pthread_create( &g_ts, NULL, thread_send, NULL );

  while(1){

  }


  return 0;
}
