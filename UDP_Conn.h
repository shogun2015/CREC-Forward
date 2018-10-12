#ifndef UDP_CONN_H

#define UDP_CONN_H

#include <string>
#include <netinet/in.h>

using namespace std;

class UDP_Conn
{
private:
  int udp_fd;
  struct sockaddr_in localAddr;
  string Name;

public:
  UDP_Conn(string IP_Bind, int Port_Bind, string Name);
  ~UDP_Conn();

  void SendMessage(char *msg, int size, struct sockaddr_in *addr);
  void SendMessage(char *msg, int size, const char *IP, int port);

  void RecvMessage(char *msg, struct sockaddr_in *addr);
  void RecvMessage(char *msg, char *source_IP, int &Port);
};

#endif