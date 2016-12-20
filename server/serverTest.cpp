/**
* サーバ側。
* ソケット通信にて、サーバで入力した文字をクライアントに送信するプログラム
*/
#include <iostream>
#include <cstdio>
#include <cstring>

#include <unistd.h>      /* read(),write(),close()               */
#include <sys/socket.h>  /* socket(), bind(), listen(), accept() */
#include <netinet/in.h>  /* struct sockaddr_in                   */

#define PORT  (8080)
using namespace std;

int main(int argc, char *argv[]) {
        int s1, s2;                                  // ソケットのハンドル
        struct sockaddr_in server_addr, client_addr; // ソケットアドレスのデータ構造体
        char buf[BUFSIZ];                            // 送信データ

        // ソケットの作成
        if ((s1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                cout << "ソケット作成失敗" << endl;
                return -1;
        } else {
                cout<< "ソケット作成成功" << endl;
        }

        // ソケットアドレスの初期化と設定
        memset((char*) &server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        // 作成したソケットと、アドレスを関連つける（バインド）
        if (bind(s1, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
                cout << "バインド失敗" << endl;
                return -1;
        } else {
                cout << "バインド成功" << endl;
        }

        // クライアントからの接続の受付準備をする
        if (listen(s1, 1) < 0) {
                cout << "受付準備失敗" << endl;
                return -1;
        } else {
                cout << "受付準備成功" << endl;
        }

        // クライアントからコネクション受付を待つ
        cout << "受付開始..." << endl;
        unsigned int len = sizeof(client_addr);
        if ((s2 = accept(s1, (struct sockaddr *) &client_addr, &len)) < 0) {
                cout << "コネクト失敗" << endl;
                return -1;
        } else {
                cout << "コネクト成功" << endl;
        }

        // データ送信
        cout << "送信データを入力してください.-> ";
        cin.getline(buf,BUFSIZ);
        write(s2, buf, sizeof(buf));

        close(s1);
        close(s2);

        return 0;
}
