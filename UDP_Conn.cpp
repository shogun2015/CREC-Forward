#include "Global.h"
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "UDP_Conn.h"

// #define Msg_HeartBeat_label 0x01
// #define Msg_HeartBeat_Length 6 // 4 + 1 + 1
// #define Msg_Length 405         // 4 + 1 + 400
// #define ContentByte 5          // 4 + 1
// #define HeaderLength 4
using namespace std;

UDP_Conn::UDP_Conn(string IP_Bind, int Port_Bind, string Name)
{
    this->Name = Name;

    // 对套接字初始化
    bzero(&localAddr, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(Port_Bind);
    // localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_addr.s_addr = inet_addr(IP_Bind.c_str());

    // 创建套接口
    if ((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        cout << Name << " ";
        perror("socket create failed\n");
        exit(EXIT_FAILURE);
    }
    // puts("socket create success");
    // 设置端口
    if (bind(udp_fd, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0)
    {
        cout << Name << " ";
        perror("bind fail\n");
        close(udp_fd);
        exit(EXIT_SUCCESS);
    }
    // puts("bind success");
}

void UDP_Conn::SendMessage(char *msg, int size, struct sockaddr_in *addr)
{
    int err = sendto(udp_fd, msg, size, 0, (struct sockaddr *)addr, sizeof(struct sockaddr_in));
    // printf("err: %d\n", err);
}

void UDP_Conn::SendMessage(char *msg, int size, const char *IP, int port)
{
    // cout << IP << ":" << port << endl;
    struct sockaddr_in remoteAddr;

    bzero(&remoteAddr, sizeof(struct sockaddr_in));
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(port);
    remoteAddr.sin_addr.s_addr = inet_addr(IP);

    this->SendMessage(msg, size, &remoteAddr);
}

void UDP_Conn::RecvMessage(char *msg, struct sockaddr_in *addr)
{
    socklen_t addrlen = sizeof(struct sockaddr_in);
    bzero(addr, sizeof(struct sockaddr_in));

    recvfrom(udp_fd, msg, Msg_Length, 0, (struct sockaddr *)&addr, &addrlen);
}

void UDP_Conn::RecvMessage(char *msg, char *source_IP, int &Port)
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    bzero(&addr, sizeof(struct sockaddr_in));

    // msg = new char[Msg_Length];
    recvfrom(udp_fd, msg, Msg_Length, 0, (struct sockaddr *)&addr, &addrlen);

    strcpy(source_IP, inet_ntoa(addr.sin_addr));
    Port = addr.sin_port;
}

UDP_Conn::~UDP_Conn()
{
    close(udp_fd);
}