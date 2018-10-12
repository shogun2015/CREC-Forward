#ifndef FORWARD_H

#define FORWARD_H

#include "UDP_Conn.h"
#include <string>

using namespace std;

// 定义本地网卡
string local_IP_LTE;
int Port_LTE;
string local_IP_WLAN;
int Port_WLAN;
string local_IP_VOBC;
int local_Port_VOBC;

string IP_VOBC;
int Port_VOBC;

void *thread_RecvMsg_LTE(void *arg);
void *thread_RecvMsg_WLAN(void *arg);
void *thread_RecvMsg_VOBC(void *arg);
void *thread_SendMsg_VOBC(void *arg);

// 初始化本地网卡变量，将文件内容读入内存
void init_Network_Value(string FIleName);

// // 分解IP地址
// unsigned char *separateIP(string strIP);
// // 合成IP地址
// string mergeIP(unsigned char *n);

#endif