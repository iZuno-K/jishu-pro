//g_で始まるものは必ずグローバル変数
//g_ついてないけどグローバル変数なのもある
//-----------------------------------------------------------------------------------
// niconico_server.c : ビデオチャットプログラム
//-----------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include <cv.h>
#include <highgui.h>

#include "defs.h"
#include <time.h>

int socket_list[16];
int client_num = 0;
char messages[16][1024];

static IplImage* g_frame;
static pthread_mutex_t g_mutex;

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
int socket_write( int socket, void* buf, int buf_size )
{
  int s;
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

    if( size == 0 ) break;
    if( size <  0 ) {
      switch( errno ){
      case EPIPE:
        for(s = 0; s < client_num; s++) {
          if ( socket_list[s] == socket ) {
            int j;
            for ( j = s; j < client_num-1; j++) {
              socket_list[j] = socket_list[j+1];
            }
            client_num--;
            if (client_num <= 0) {
              printf("end\n");
              exit(1);
            }
          }
        }
        printf("Disconnected : %d\n", socket);
        return -1;
      }
      break;
    }
  }

  return buf_size - left_size;
}

//-----------------------------------------------------------------------------------
// comm：クライアントごとの送受信スレッド
//-----------------------------------------------------------------------------------
void* comm(void* arg){
/* pthread_create(&thread, NULL, comm, (void*)&ns); で作成したスレッド
から、呼び出される */
  int s = *(int *)arg;
  int size;
  char buf[1024];
  pthread_t char_thread;
  int new_client_index;

  pthread_detach(pthread_self());

  pthread_mutex_lock( &g_mutex );
  {
    new_client_index = client_num;
    socket_list[new_client_index] = s;
    client_num++;
  }
  pthread_mutex_unlock( &g_mutex );

  while(1){
    int i, ret;

    ret = socket_read(s, &size, sizeof(int));
    if ( ret <= 0 ) return NULL;
    if( size > 0 ) {
      pthread_mutex_lock( &g_mutex );
      {
        /*担当するクライアントから受信する*/
        socket_read(s, buf, size);
        printf("Client%d: %s\n",new_client_index,buf);
        memset( messages[new_client_index], '\0',  strlen(buf)+1);
        strcpy( messages[new_client_index], buf);
        /*全てのクライアントに受信データ送信する*/
        for(i = 0; i < client_num; i++) {
          //受信元にはおくらない
        if (socket_list[i] == s) continue;
        if ( socket_write( socket_list[i], &size, sizeof(int) )  < 0 ) {
          return NULL;
        }
        if ( socket_write( socket_list[i], buf, size ) < 0 ) {
          return NULL;
        }
      }
      }
      pthread_mutex_unlock( &g_mutex );
    }
  }

return NULL;
}

//-----------------------------------------------------------------------------------
// thread_idle：アイドル時
//-----------------------------------------------------------------------------------
void* thread_idle(void* arg)
{
  int size = 0;
  CvCapture *capture = (CvCapture *)arg;

  pthread_detach(pthread_self());

  // cvNamedWindow("Server", CV_WINDOW_AUTOSIZE);
  while(1){
    int i;

    IplImage*frame = cvQueryFrame(capture);

    if (frame == NULL) {
      if (cvGetCaptureProperty(capture,CV_CAP_PROP_POS_AVI_RATIO) > 0.9)
        cvSetCaptureProperty(capture,CV_CAP_PROP_POS_AVI_RATIO,0.0);
      continue;
    }

    // cvShowImage("Server", frame);

    //必ずEScキーで終了すること（カメラデバイスが使えなくなる）
    if ( cvWaitKey(10) == 27 ){
      cvReleaseCapture(&capture);
      printf("Relesase success\n");
      // cvDestroyWindow("Server");
      exit(1);
    }


    // すべての接続中のクライアントへバッファを送信
    //---------------------------------------------------
    pthread_mutex_lock( &g_mutex );
    {
      for(i = 0; i < client_num; i++) {
        socket_write( socket_list[i], &size, sizeof(int) );
        socket_write( socket_list[i], frame->imageData, frame->width*frame->height*(frame->depth/8)*frame->nChannels);
      }
    }
    pthread_mutex_unlock( &g_mutex );
  }
  cvDestroyWindow("Server");

  return NULL;
}

//-----------------------------------------------------------------------------------
// main
//-----------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  CvCapture *capture = NULL;
  IplImage *frame;
  int i, s, ns;
  struct sockaddr_in sin;
  pthread_t thread;

  signal( SIGPIPE , SIG_IGN );

  if (argc == 1
      || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
    capture = cvCreateCameraCapture(argc == 2 ? argv[1][0] - '0' : 0);
  else if (argc == 2 )
    capture = cvCreateFileCapture(argv[1]);

  if (capture==NULL) {
    fprintf(stderr, "ERROR : could not create capture object\n");
    exit(1);
  }

  for (i=0;i<3;i++) frame = cvQueryFrame(capture);


  // ソケットの生成
  //---------------------------------------------------
  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("server: socket");
    exit(1);
  }

  memset( (char *)&sin, 0, sizeof(struct sockaddr) );

  sin.sin_family = AF_INET; // アドレスの型の指定
  sin.sin_port = PORT; // ポート番号
  sin.sin_addr.s_addr = INADDR_ANY;
//inet_addr("10.1.1.38"); // 待ち受けのIPアドレスの設定

  /* ソケットにパラメータを与える */
  if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
    perror("server: bind");
    exit(1);
  }

  // クライアントの接続を待つ
  //---------------------------------------------------
  if ((listen(s, 10)) < 0) {
    perror("server: listen");
    exit(1);
  }

  printf("waiting for connection\n");

  // thread_idle（アイドル時）用のスレッドを作成
  pthread_create(&thread, NULL, thread_idle, (void*)capture);

  // クライアントからの接続あれば、スレッドを生成する
  //---------------------------------------------------
  while(1){
    if ((ns = accept(s, NULL, 0)) < 0) {
      perror("server: accept");
      continue;
    }

    printf("Connected : %d\n", ns);

    socket_write( ns, &frame->width, sizeof(int) );
    socket_write( ns, &frame->height, sizeof(int) );
    socket_write( ns, &frame->depth, sizeof(int) );
    socket_write( ns, &frame->nChannels, sizeof(int) );

    //comm用のスレッドを生成する
    pthread_create(&thread, NULL, comm, (void*)&ns);
  }

  cvReleaseCapture(&capture);
  printf("Relesase success\n");
}
