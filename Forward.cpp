
#include "Global.h"
#include "Forward.h"

#include "UDP_Conn.h"
#include "Double_map.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>

#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 预编译开关
#define LTE_Enable
#define WLAN_Enable
#define VOBC_Enable
#define Redundancy_Removal

using namespace std;

// 网络通信相关变量
UDP_Conn *udp_conn_LTE;
UDP_Conn *udp_conn_WLAN;
UDP_Conn *udp_conn_VOBC;
// struct sockaddr_in addr_LTE;
// struct sockaddr_in addr_WLAN;
// struct sockaddr_in addr_VOBC;

// 双map变量
Double_map *map_NameCode_LTE;
Double_map *map_NameCode_WLAN;

#ifdef Redundancy_Removal
// 面向VOBC的发送队列
deque<char *> VOBC_FIFO;
// 用于FIFO的线程锁
pthread_mutex_t mutex_FIFO_toVOBC;
#endif

int main(int argc, char const *argv[])
{
    // 初始化map变量
    cout << BOLDWHITE;
    map_NameCode_LTE = new Double_map(LTE_File);
    map_NameCode_WLAN = new Double_map(WLAN_File);
    cout << RESET << endl;

    // 创建线程
    pthread_t t_Recv_LTE, t_Recv_WLAN, t_Recv_VOBC;

    // 从CSV文件读取IP地址端口配置
    init_Network_Value(Local_IP_File);

    // cout << local_IP_LTE << ":" << Port_LTE << endl;

// 准备UDP连接 、 远端地址 、 链路状态
#ifdef LTE_Enable
    udp_conn_LTE = new UDP_Conn(local_IP_LTE, Port_LTE, "LTE");
    pthread_create(&t_Recv_LTE, NULL, thread_RecvMsg_LTE, (void *)NULL);
#endif

#ifdef WLAN_Enable
    udp_conn_WLAN = new UDP_Conn(local_IP_WLAN, Port_WLAN, "WLAN");
    pthread_create(&t_Recv_WLAN, NULL, thread_RecvMsg_WLAN, (void *)NULL);
#endif

#ifdef VOBC_Enable
    udp_conn_VOBC = new UDP_Conn(local_IP_VOBC, local_Port_VOBC, "VOBC");
    pthread_create(&t_Recv_VOBC, NULL, thread_RecvMsg_VOBC, (void *)NULL);
#endif

#ifdef Redundancy_Removal
    pthread_t t_Send_VOBC;
    pthread_create(&t_Send_VOBC, NULL, thread_SendMsg_VOBC, (void *)NULL);
    // 初始化互斥锁
    pthread_mutex_init(&mutex_FIFO_toVOBC, NULL);
#endif

    while (1)
    {
        // // 生成数据
        // // cout << "input destination: ";
        // int iDest = 2;
        // // cin >> iDest;
        // cout << "input content: ";
        // string strContent/* = "abcdefg"*/;
        // cin >> strContent;
        // char Dest[4];
        // Dest[0] = 'T';
        // Dest[1] = '0';
        // Dest[2] = '0';

        // switch (iDest)
        // {
        // case 1:
        //     Dest[3] = '1';
        //     break;
        // case 2:
        //     Dest[3] = '2';
        //     break;

        // default:
        //     cout << "wrong destination" << endl;
        //     break;
        // }

        // char *msg = new char[Msg_Length];
        // memcpy(msg, Dest, 4);
        // strcpy(msg + 4, strContent.c_str());

        // udp_conn_VOBC->SendMessage(msg, Msg_Length, "127.0.0.1", local_Port_VOBC);

        // delete[] msg;
        // break; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!记得删掉
    }

    pthread_join(t_Recv_LTE, NULL);
    pthread_join(t_Recv_WLAN, NULL);
    pthread_join(t_Recv_VOBC, NULL);
#ifdef Redundancy_Removal
    pthread_join(t_Send_VOBC, NULL);
    // 释放互斥锁
    pthread_mutex_destroy(&mutex_FIFO_toVOBC);
#endif

    // 释放连接类
    delete udp_conn_LTE;
    delete udp_conn_WLAN;
    delete udp_conn_VOBC;

    // 释放map
    delete map_NameCode_LTE;
    delete map_NameCode_WLAN;

    return 0;
}

#ifdef LTE_Enable
void *thread_RecvMsg_LTE(void *arg)
{
    cout << BOLDBLUE;
    cout << "Begin thread_RecvMsg_LTE" << endl;
    cout << RESET;

    struct sockaddr_in RemoteAddr;
    socklen_t addrlen = sizeof(RemoteAddr);
    int Port;

    while (1)
    {
        char *msg_Recv = new char[Msg_Length]; // 分配数据包内容空间
        char *source_IP = new char[16];        // 分配IP地址空间
        bzero(&RemoteAddr, sizeof(RemoteAddr));
        udp_conn_LTE->RecvMessage(msg_Recv, source_IP, Port);

        //************** 收包处理 **************
        // 将源IP的最后一位翻译城列车名称填入数据包原来代表目的名的位置
        // IP = separateIP(source_IP);
        string strName = map_NameCode_LTE->findName(source_IP);
        memcpy(msg_Recv, strName.c_str(), 4);

#ifdef Redundancy_Removal
        pthread_mutex_lock(&mutex_FIFO_toVOBC);
        VOBC_FIFO.push_back(msg_Recv);
        pthread_mutex_unlock(&mutex_FIFO_toVOBC);
#else
        // 转发给车载控制器VOBC
        udp_conn_VOBC->SendMessage(msg_Recv, Msg_Length, IP_VOBC.c_str(), Port_VOBC);

        delete[] msg_Recv;
        msg_Recv = 0;
#endif
    }
}
#endif

#ifdef WLAN_Enable
void *thread_RecvMsg_WLAN(void *arg)
{
    cout << BOLDBLUE;
    cout << "Begin thread_RecvMsg_WLAN" << endl;
    cout << RESET;

    struct sockaddr_in RemoteAddr;
    socklen_t addrlen = sizeof(RemoteAddr);
    int Port;

    while (1)
    {
        char *msg_Recv = new char[Msg_Length]; // 分配数据包内容空间
        char *source_IP = new char[16];        // 分配IP地址空间
        bzero(&RemoteAddr, sizeof(RemoteAddr));
        udp_conn_WLAN->RecvMessage(msg_Recv, source_IP, Port);

        //************** 收包处理 **************
        // 将源IP的最后一位翻译城列车名称填入数据包原来代表目的名的位置
        // IP = separateIP(source_IP);
        string strName = map_NameCode_WLAN->findName(source_IP);
        memcpy(msg_Recv, strName.c_str(), 4);

#ifdef Redundancy_Removal
        pthread_mutex_lock(&mutex_FIFO_toVOBC);
        VOBC_FIFO.push_back(msg_Recv);
        pthread_mutex_unlock(&mutex_FIFO_toVOBC);

#else
        // 转发给车载控制器VOBC
        udp_conn_VOBC->SendMessage(msg_Recv, Msg_Length, IP_VOBC.c_str(), Port_VOBC);

        delete[] msg_Recv;
        msg_Recv = 0;
#endif
    }
}
#endif

#ifdef VOBC_Enable
void *thread_RecvMsg_VOBC(void *arg)
{
    cout << BOLDBLUE;
    cout << "Begin thread_RecvMsg_VOBC" << endl;
    cout << RESET;

    // unsigned char *IP_template_LTE = separateIP(local_IP_LTE);
    // unsigned char *IP_template_WLAN = separateIP(local_IP_WLAN);

    struct sockaddr_in RemoteAddr;
    socklen_t addrlen = sizeof(RemoteAddr);

    while (1)
    {
        char *msg_Recv = new char[Msg_Length]; // 分配数据包内容空间
        // char *source_IP = new char[15];        // 分配IP地址空间
        bzero(&RemoteAddr, sizeof(RemoteAddr));
        udp_conn_VOBC->RecvMessage(msg_Recv, &RemoteAddr);

        //************** 收包处理 **************
        char TrainName[5];
        memset(TrainName, 0, 5);
        memcpy(TrainName, msg_Recv, 4); // 取前4位做目的地判断
        // unsigned char keyIP = map_NameCode->findCode(TrainName); // 获取目的IP关键值
        string strIP_LTE = map_NameCode_LTE->findIP(TrainName);
        string strIP_WLAN = map_NameCode_WLAN->findIP(TrainName);

#ifdef Redundancy_Removal
        // 向数据包里写入 包序号
        static unsigned int Packet_Seq = 0;
        memcpy(msg_Recv + 4, &Packet_Seq, 4);
        ++Packet_Seq;
#endif

        // // 仅修改两路IP的最后一位
        // IP_template_LTE[3] = keyIP;
        // IP_template_WLAN[3] = keyIP;

        // string strIP_LTE = mergeIP(IP_template_LTE);
        // string strIP_WLAN = mergeIP(IP_template_WLAN);

        udp_conn_LTE->SendMessage(msg_Recv, Msg_Length, strIP_LTE.c_str(), Port_LTE);
        udp_conn_WLAN->SendMessage(msg_Recv, Msg_Length, strIP_WLAN.c_str(), Port_WLAN);

        delete[] msg_Recv;
        msg_Recv = 0;
    }
}
#endif

#ifdef Redundancy_Removal
void *thread_SendMsg_VOBC(void *arg)
{
    cout << BOLDBLUE;
    cout << "Begin thread_SendMsg_VOBC" << endl;
    cout << RESET;

    unsigned int current_Index = -1;
    while (1)
    {

        pthread_mutex_lock(&mutex_FIFO_toVOBC);
        if (!VOBC_FIFO.empty())
        {
            char *Msg = VOBC_FIFO.front();
            VOBC_FIFO.pop_front();
            pthread_mutex_unlock(&mutex_FIFO_toVOBC);
            // 提取包序号
            unsigned int *packet_seq = (unsigned int *)(Msg + 4); // 跳过目的地名称

            // 序号比较, 参考TCP包序号绕回
            if ((signed int)(current_Index - *packet_seq) < 0)
            {
                udp_conn_VOBC->SendMessage(Msg, Msg_Length, IP_VOBC.c_str(), Port_VOBC);
                // 更新至最新序号
                current_Index = *(Msg + 4);
            }

            delete[] Msg;
        }
        else
        {
            pthread_mutex_unlock(&mutex_FIFO_toVOBC);
            usleep(10000); // 10ms
        }
    }
}
#endif

void init_Network_Value(string FileName)
{
    ifstream inFile(FileName.c_str(), ios::in);
    string lineStr;
    char *end; // 仅作填充参数,无进一步处理
    if (inFile.fail())
    {
        cout << "读取 本地IP 文件失败" << endl;
    }

    getline(inFile, lineStr); // 跳过第一行

    while (getline(inFile, lineStr))
    {
        stringstream ss(lineStr);
        string strType;
        string strIP;
        string strPort;
        int iPort;

        // 读取网络类型名称
        getline(ss, strType, ',');

        // 读取本地IP地址
        getline(ss, strIP, ',');

        // 读取本地端口号
        getline(ss, strPort, '\0');
        iPort = static_cast<int>(strtol(strPort.c_str(), &end, 10)); // string -> int

        if ("To_LTE" == strType)
        {
            local_IP_LTE = strIP;
            Port_LTE = iPort;
            continue;
        }

        if ("To_WLAN" == strType)
        {
            local_IP_WLAN = strIP;
            Port_WLAN = iPort;
            continue;
        }

        if ("To_VOBC" == strType)
        {
            local_IP_VOBC = strIP;
            local_Port_VOBC = iPort;
            continue;
        }

        if ("VOBC" == strType)
        {
            IP_VOBC = strIP;
            Port_VOBC = iPort;
            continue;
        }
    }

    cout << BOLDYELLOW;
    cout << "--------- Init Local Network Value ---------" << endl;

    cout << "To LTE\t- " << local_IP_LTE << "\t:" << Port_LTE << endl;
    cout << "To WLAN\t- " << local_IP_WLAN << "\t:" << Port_WLAN << endl;
    cout << "To VOBC\t- " << local_IP_VOBC << "\t:" << local_Port_VOBC << endl;
    cout << endl;
    cout << "VOBC\t- " << IP_VOBC << "\t:" << Port_VOBC << endl;

    cout << "---------------- Init - END ----------------" << endl;
    cout << RESET << endl;
}

// string mergeIP(unsigned char *n)
// {
//     stringstream ss;
//     ss << (int)n[0] << "." << (int)n[1] << "." << (int)n[2] << "." << (int)n[3];

//     return ss.str();
// }

// unsigned char *separateIP(string strIP)
// {
//     unsigned char *IP = new unsigned char[4];
//     unsigned int itemp = inet_addr(strIP.c_str());
//     // inet_addr变换后,为网络字节序,赋值到数组时要逆序
//     IP[3] = (itemp & 0xff000000) >> 24;
//     IP[2] = (itemp & 0x00ff0000) >> 16;
//     IP[1] = (itemp & 0x0000ff00) >> 8;
//     IP[0] = (itemp & 0x000000ff);

//     return IP;
// }