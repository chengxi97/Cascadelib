#ifndef _RECONCILIATION_BOB
#define _RECONCILIATION_BOB

#include <winsock.h>

#define HOSTPORT (8887)       // 服务器端口
#define HOSTIP ("127.0.0.1")  // 服务器IP地址

void netDelayTest();   // 网络延迟测试（交替发送）
void statOutput(int t);     // 数据统计输出
inline void initData(unsigned char* posData, int dataSize, double qber);  // 初始化数据

SOCKET serverSocketInit();  // 服务器端Socket创建
SOCKET clientSocketInit();  // 客户端Socket创建
sockaddr_in clientToServerAddr();  // 客户端获取服务器地址
bool initClientConn(SOCKET* clientSocket);  // 建立客户端连接
bool initServerConn(SOCKET& serverSocket, SOCKET* clientSocket);  // 建立服务器端连接

#endif