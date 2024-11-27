#ifndef _RECONCILIATION_BOB
#define _RECONCILIATION_BOB

#include <winsock.h>

#define HOSTPORT (8887)       // �������˿�
#define HOSTIP ("127.0.0.1")  // ������IP��ַ

void netDelayTest();   // �����ӳٲ��ԣ����淢�ͣ�
void statOutput(int t);     // ����ͳ�����
inline void initData(unsigned char* posData, int dataSize, double qber);  // ��ʼ������

SOCKET serverSocketInit();  // ��������Socket����
SOCKET clientSocketInit();  // �ͻ���Socket����
sockaddr_in clientToServerAddr();  // �ͻ��˻�ȡ��������ַ
bool initClientConn(SOCKET* clientSocket);  // �����ͻ�������
bool initServerConn(SOCKET& serverSocket, SOCKET* clientSocket);  // ����������������

#endif