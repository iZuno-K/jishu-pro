/* chat_client.c --チャット クライアント プログラム */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
 
#define PORT      63000  /* 変更 0-65535 */
#define BUF_SIZE  1024   /* バッファのサイズ */
 
#define BLACK      "\x1b[0;30m"
#define RED        "\x1b[0;31m"
#define GREEN      "\x1b[0;32m"
#define YELLOW     "\x1b[0;33m"
#define BLUE       "\x1b[0;34m"
#define VIOLET     "\x1b[0;35m"
#define LIGHT_BLUE "\x1b[0;36m"
 
static int myconnect(char *host, int port);
int        input_name(int fd);  /* 名前入力 */
void       *send_msg(int *fd);  /* スレッド(メッセージ送信) */
 
char hn[64]; /* ハンドルネーム */
 
 
int main(int argc, char *argv[]) {
    int       fd, n, i;
    char      buf[BUF_SIZE], lieve[BUF_SIZE];
    pthread_t pid;
 
    /* 引数チェック */
    if (argc != 2) {
        fprintf(stderr, "useage: %s host\n", argv[0]);
        return 1;
    }
 
    /* サーバと接続 */
    if ((fd = myconnect(argv[1], PORT)) < 0) {
        return 1;
    }
    printf("\x1b[2J");  //画面消去
 
    /* サーバとのデータのやり取り */
    input_name(fd);  /* 名前入力 */
    read(fd, buf, sizeof(buf));
    buf[strlen(buf)-1] = '\0';
    printf("%s\n", buf);
 
    pthread_create(&pid, NULL, (void *)&send_msg, (void *)&fd);  /* スレッド作成 */
    sprintf(lieve, "!-- %s left the room.\a", hn); lieve[strlen(lieve)-1] = '\0';
    /* サーバからのデータ受信 */
    while(1) {
        buf[0] = '\0';  /* 初期化 */
        n = read(fd, buf, sizeof(buf));
        buf[n] = '\0';
        if (strcmp(buf, lieve) == 0) {
            break;
        }
        printf("%s\n", buf);
	pthread_join(pid, NULL);
    }
    /* 終了 */
    close(fd);
    return 0;
}
 
 
/*----------*
 * 名前入力 *
 *----------*/
int input_name(int fd) {
    hn[0] = '\0';  /* 初期化 */
    printf("名前を入力してください: ");
    if (fgets(hn, sizeof(hn), stdin) == NULL) {
        fprintf(stderr, "Input your name!\n");
        close(fd);
        return -1;
    }
    hn[strlen(hn)-1] = '\0';    /* 改行文字を消す */
    printf("Your name is '%s'\n", hn);
    write(fd, hn, strlen(hn));  /* 送信 */
    return 0;
}
 
 
/*----------------*
 * スレッド       *
 * メッセージ送信 *
 *----------------*/
void *send_msg(int *fd) {
    char       snd[BUF_SIZE], buf[BUF_SIZE], color[16];
    int        n, color_no = 0;
    static int i = 0;
 
    sprintf(color, "%s", BLACK);
    /* 情報 */
    usleep(1000*300);
    printf( "!--INFORMATION\n"
            "  To get member list, input 'MEMBER'\n"
            "  To change message color, input 'COLOR'\n"
            "  To logout the room, input'LOGOUT'\n");
 
    while(1) {
        buf[0] = '\0'; snd[0] = '\0';  /* 初期化 */
        usleep(500*1000);
        if (i++ == 0) {
            printf("Input message: ");
        }
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        if (strcmp(buf, "LOGOUT") == 0) {  /* ログアウト */
            write(*fd, buf, sizeof(buf));
            printf("logouted.\nPlease push 'Ctrl+C'.\n");  /* ログアウト確認 */
            break;
        }
        else if (strcmp(buf, "COLOR") == 0) {
            printf( "!--Choose your message color.\n"
                    "  0: black (default)\n"
                    "  1:%s red        %s\n"
                    "  2:%s green      %s\n"
                    "  3:%s yellow     %s\n"
                    "  4:%s blue       %s\n"
                    "  5:%s violet     %s\n"
                    "  6:%s light blue %s\n"
                    "  --Input the number of color:",
                    RED,  BLACK, GREEN,  BLACK, YELLOW,     BLACK,
                    BLUE, BLACK, VIOLET, BLACK, LIGHT_BLUE, BLACK);
            scanf("%d", &color_no);
            if ((color_no < 0) || (color_no > 6)) {
                printf("%sinput from 0 to 6!!%s\n", RED, BLACK);
            }
            else {
                sprintf(color, "\x1b[0;%dm", color_no+30);
                printf("%s color changed.%s\n", color, BLACK);
            }
        }
        else {
            sprintf(snd, "%s%s%s", color, buf, BLACK);
            write(*fd, snd, sizeof(snd));
        }
    }
    pthread_exit(NULL);
}
 
 
/*------------------------------*
 * サーバに接続して記述子を返す *
 *------------------------------*/
static int myconnect(char *host, int port) {
    int                fd;
    struct sockaddr_in addr;
    struct hostent     *hp;
 
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }
    if ((hp = gethostbyname(host)) == NULL) {
        fprintf(stderr, "gethost error %s\n", host);
        close(fd);
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return -1;
    }
    return fd;
}
