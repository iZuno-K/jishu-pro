/* chat_server.c --チャット サーバ プログラム */
 
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
 
#define PORT        63000  /* 変更 0-65535 */
#define MAX_CLIENT  10     /* 接続要求を受け付ける数 */
#define BUF_SIZE    1024   /* バッファのサイズ */
 
/* メンバーの情報を格納する */
typedef struct {
    char name[64];
    int  fd;
    int  no;
} Mem;
 
Mem member[MAX_CLIENT];
 
static int open_server(int port);
void * thr(void * member);
void * send_msg(char *buf);
 
 
int main(void) {
    int       fd, client_fd;
    pthread_t client[MAX_CLIENT];
    int       n, i;
 
    for (i = 0; i < MAX_CLIENT; i++) {
        member[i].fd = 0;
    }
    printf("Press 'Ctrl+C' to end.\n");
    /* サーバ準備 */
    if ((fd = open_server(PORT)) < 0) {
        return 1;
    }
 
    /* クライアントの接続を待つ */
    for (i = 0; i < MAX_CLIENT; i++) {
        printf("waiting for connection of client No.%d\n", i);
        if ((member[i].fd = accept(fd, NULL, NULL)) < 0) {
            perror("accept");
            close(fd);
            return 1;
        }
        printf("client_fd: %d come\n", member[i].fd);
        member[i].no = i;
        pthread_create(&client[i], NULL, (void *)thr, (void *)&member[i]);
	printf("thread created!\n");
    }
    /* 終了 */
    close(fd);
    return 0;
}
 
 
/*----------*
 * スレッド *
 *----------*/
void *thr(void * member)
{
    char buf[BUF_SIZE];
    int  n, i;
    Mem mem = *(Mem *)member;
    
    printf("%d\n",mem.fd);
    n = read(mem.fd, mem.name, strlen(mem.name)-1);
    mem.name[n] = '\0';
    printf("%s come\n", mem.name);
    
    printf("n = %d\n", n);
    printf("imakoko\n");
 
    sprintf(buf, "%s(id:%d) が参加しました.\n", mem.name, mem.fd);
    send_msg(buf);
 
    sprintf(buf, "If you want to exit,please input 'LOGOUT' .\n");
    write(mem.fd, buf, sizeof(buf));

    printf("imakoko2\n");
    printf("%d\n",mem.fd);
    while(1) {
        while ((n = read(mem.fd, buf, strlen(buf))) > 0) {
        }
        if (strcmp(buf, "LOGOUT") == 0) {
            sprintf(buf, "%s が退室しました.\n", mem.name);
            send_msg(buf);
            pthread_exit(0);
        }
        send_msg(buf);
    }
  pthread_exit(0);

}
 
 
/*------------------*
 * メッセージ送信部 *
 *------------------*/
void *send_msg(char *buf) {
    int i;
    for (i = 0; i < MAX_CLIENT; i++) {
        if (member[i].fd != 0) {
            write(member[i].fd, buf, strlen(buf)-1);
        }
    }
}
 
 
/*------------------------------------------*
 * サーバソケットをオープンして記述子を返す *
 *------------------------------------------*/
static int open_server(int port) {
    int                fd;
    int                opt;
    struct sockaddr_in addr;
 
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return -1;
    }
    if (listen(fd, MAX_CLIENT) < 0) {
        perror("listen");
        close(fd);
        return -1;
    }
    opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    return fd;
}
