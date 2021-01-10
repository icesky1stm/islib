//
// Created by suitm on 2020/12/30.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "../src/istcp.h"

#define TEST_PORT 28111
#define TEST_UNIX_PATH "./istest_unix_tmp"

int istcptest_server(){
    char * ip;
    int ret = 0;
    int listen_sock = 0;
    int conn_sock = 0;
    char msg[200];

    listen_sock = istcp_listen(TEST_PORT);
    if( listen_sock < 0 ){
        printf("listen error [%d],[%s]\n", listen_sock, strerror(errno));
        return -5;
    }
    printf("AF_INET - BIND PORT[%d]\n", TEST_PORT);
    while(1){
        conn_sock = istcp_accept_gethost(listen_sock, &ip);
        if( conn_sock < 0) {
            printf("recv error [%d]", ret);
            return -5;
        }
        printf("AF_INET - ACCEPT\n");

        memset( msg, 0x00, sizeof( msg));
        ret = istcp_recv( conn_sock, msg, 10, 10);
        if( ret < 0){
            printf("recv error [%d]\n", ret);
            return -5;
        }
        printf("AF_INET - RECV[%s]\n", msg);
        ret = istcp_send( conn_sock, "12345", 5, 10);
        if( ret < 0){
            printf("recv error [%d]\n", ret);
            return -5;
        }
        sleep(1);
        ret = istcp_send( conn_sock, "67890", 5, 10);
        if( ret < 0){
            printf("recv error [%d]\n", ret);
            return -5;
        }
        istcp_close(conn_sock);

        // 测试，所以连接一次就退出
        break;
    }
    return 0;
}

int istcptest_server_unix(){
    int ret = 0;
    int listen_sock = 0;
    int conn_sock = 0;
    char msg[200];

    listen_sock = istcp_listen_unix(TEST_UNIX_PATH);
    if( listen_sock < 0 ){
        printf("AF_UNIX listen error [%d][%s]\n", listen_sock, strerror(errno));
        return -5;
    }
    printf("AF_UNIX - BIND PATH[%s]\n", TEST_UNIX_PATH);
    while(1){
        conn_sock = istcp_accept(listen_sock);
        if( conn_sock < 0) {
            printf("AF_UNIX recv error [%d]", ret);
            return -5;
        }
        memset( msg, 0x00, sizeof( msg));
        ret = istcp_recv( conn_sock, msg, 10, 10);
        if( ret < 0){
            printf("AF_UNIX recv error [%d]", ret);
            return -5;
        }
        printf("AF_UNIX - RECV[%s]\n", msg);
        ret = istcp_send( conn_sock, "1234567890", 10, 10);
        if( ret < 0){
            printf("AF_UNIX recv error [%d]", ret);
            return -5;
        }
        istcp_close(conn_sock);

        // 测试就连一次
        break;
    }
    return 0;
}

int main(){
    printf("version = [%s]\n", istcp_version());

    pid_t pid;

    pid = fork();
    if( pid == 0){
        /** 子进程作为服务端A  **/
        istcptest_server();
        return 0;
    }else if( pid < 0){
        printf("创建进程失败!!![%s]",strerror(errno));
        return -5;
    }

    int ret = 0;
    char msg[200];
    int conn = 0;
    /** 开始作为客户端的测试 **/
    sleep(1);
    conn = istcp_connect("127.0.0.1", TEST_PORT);
    if( conn < 0){
        printf("连接失败!!![%d]\n", conn);
        return -5;
    }
    printf("测试tcp server，连接成功\n");

    ret = istcp_send( conn, "0123456789", 10, 10);
    if( ret <= 0){
        printf("发送失败!!![%d]\n", ret);
       return -5;
    }
    printf("测试tcp server，发送成功\n");
    ret = istcp_recv_nowait( conn, msg, 10, 10);
    if( ret <= 0){
        printf("接收失败!!![%d]\n", ret);
        return -5;
    }
    printf("测试tcp server，收到报文[%s],少收到了5个\n\n", msg);

    /** unix域socket服务测试 **/
    pid = fork();
    if( pid == 0){
        istcptest_server_unix();
        return 0;
    }else if( pid < 0){
        printf("创建进程失败!!![%s]\n",strerror(errno));
        return -5;
    }
    /** 开始作为客户端的测试 **/
    sleep(1);
    conn = istcp_connect_unix(TEST_UNIX_PATH);
    if( conn < 0){
        printf("连接失败!!![%d]\n", conn);
        return -5;
    }
    printf("测试unix server，连接成功\n");

    ret = istcp_send( conn, "0123456789", 10, 10);
    if( ret <= 0){
        printf("发送失败!!![%d]\n", ret);
        return -5;
    }
    printf("测试unix server，发送成功\n");
    ret = istcp_recv( conn, msg, 10, 10);
    if( ret <= 0){
        printf("接收失败!!![%d]\n", ret);
        return -5;
    }
    printf("测试unix server，收到报文[%s]\n\n", msg);


    return 0;
}