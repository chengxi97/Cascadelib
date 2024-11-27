#include <omp.h>
#include <random>

#include "ReconciliationAlice.h"
#include "Reconciliation.h"

#pragma comment(lib, "CascadeLib.lib")
#pragma comment(lib,"ws2_32.lib")
  
//#define TEST_NUM (10)                // ��������
#define CASCADE_FLAG (0)               // Bob:1 , Alice:0 
#define OMP_THREADS_NUM (1)            // ���߳�����
#define CONN_NUM (2)                   // ÿ���߳��䱸CONN_NUM��Socket��1:����, 2:У��)
#define CONN_SUM (OMP_THREADS_NUM * CONN_NUM)   // Socket����

// ȫ�ֱ���
SOCKET conn[CONN_SUM];      // Socket����
//CascadeParallel* cascadeInst[OMP_THREADS_NUM] = { NULL };
void* cascadeInst[OMP_THREADS_NUM];
double qberList[OMP_THREADS_NUM] = { 0.02 };  // QBER�б�
unsigned char* dataList[OMP_THREADS_NUM];

unsigned char* codeList[OMP_THREADS_NUM];
unsigned int codeSize[OMP_THREADS_NUM];  // ÿ��codeSize
unsigned int codeSum[OMP_THREADS_NUM];   // �ۼ�code���

// ��������
std::uniform_int_distribution<int> disInfo(0, 255);
std::default_random_engine rndInfo(3);
std::uniform_int_distribution<int> disQBER(0, RAND_MAX - 1);
std::default_random_engine rndQBER(107);

// �ļ����
FILE* fp_data;
int flagTerminate = 0;

// ͳ�����
double qberStat[OMP_THREADS_NUM];
unsigned int leakedStat[OMP_THREADS_NUM];
unsigned int inputStat[OMP_THREADS_NUM];
unsigned int interactStat[OMP_THREADS_NUM];

// ��ʱ���
LARGE_INTEGER t1, t2, tc;
double durationCascade;

int main()
{
	// 0. ��ʼ��
	initClientConn(conn);
	omp_set_num_threads(OMP_THREADS_NUM);

	for (int i = 0; i < OMP_THREADS_NUM; i++) {
		initCascade(qberList[i], conn + i * CONN_NUM, CASCADE_FLAG, cascadeInst + i);
	}

	fp_data = fopen("../../Data/sifted_key_alice", "rb");
	if (fp_data == NULL){
		printf("�ļ���ȡʧ��!\n");
		exit(1);
	}
	int testNum = 0;

	// �����ӳٲ���
	netDelayTest();

	// 1. Э�̣�OMP��
	while (1) {
		durationCascade = 0;

#pragma omp parallel for
		for (int i = 0; i < OMP_THREADS_NUM; i++) {
			int rtn = 0;
			codeSum[i] = 0;

			do {
				int size = getCascadeBuf(dataList + i, codeList + i, cascadeInst[i]);

				initData(dataList[i], size, qberList[i]);
				if (feof(fp_data)) {
					printf("�ļ���ȡ���!\n");
					flagTerminate = 1;
					break;
				}

				// ��ʱ��ʼ
				QueryPerformanceCounter(&t1);
				QueryPerformanceFrequency(&tc);

				rtn = cascadeEC(codeSize + i, cascadeInst[i]);
				codeSum[i] += codeSize[i];

				// ��ʱ����
				QueryPerformanceCounter(&t2);
				durationCascade = durationCascade + (t2.QuadPart - t1.QuadPart) * 1.0 / tc.QuadPart * 1000;  // ms
			} while (rtn == 0);

			if (!flagTerminate) {
				// ��ȡͳ������
				getCascadeStat(qberStat + i, leakedStat + i, inputStat + i, interactStat + i, cascadeInst[i]);
			}

		}

		if (flagTerminate) {
			break;
		}

		statOutput(testNum);
		testNum++;
	};

	// 2. �ͷ�Socket���ͷ��ڴ�
	for (int i = 0; i < OMP_THREADS_NUM; i++) {
		delete cascadeInst[i];
		cascadeInst[i] = NULL;

		closesocket(conn[i]);
	}
	WSACleanup();

	printf("Э�̽���\n");
	getchar();

	return 0;
}

inline void initData(unsigned char* posData, int dataSize, double qber)
{
	fread(posData, sizeof(unsigned char), dataSize, fp_data);
}

void netDelayTest()
{
	char sendData[100];

#if CASCADE_FLAG == 1
	LARGE_INTEGER t1, t2, tc;
	QueryPerformanceFrequency(&tc);

	for (int i = 0; i < 5; i++) {
		QueryPerformanceCounter(&t1);
		memcpy(sendData, &t1, sizeof(LARGE_INTEGER));
		send(conn[0], sendData, sizeof(LARGE_INTEGER), 0);
		recv(conn[0], sendData, sizeof(LARGE_INTEGER), 0);
		memcpy(&t2, sendData, sizeof(LARGE_INTEGER));

		QueryPerformanceCounter(&t1);

		double duration = (t1.QuadPart - t2.QuadPart) * 1.0 / tc.QuadPart * 1000 / 2;  // ms
		printf("����: %d , ���絥����ʱ: %.2f ms\n", i, duration);

		Sleep(100); // ��Ϣ100ms�ٴβ���
	}
#else
	for (int i = 0; i < 5; i++) {
		recv(conn[0], sendData, sizeof(LARGE_INTEGER), 0);
		send(conn[0], sendData, sizeof(LARGE_INTEGER), 0);
	}
#endif
}

inline void statOutput(int t)
{
	unsigned int interactSum = 0;
	unsigned int leakedSum = 0;

	double duration = (t2.QuadPart - t1.QuadPart) * 1000.0 / tc.QuadPart;

	for (int index = 0; index < OMP_THREADS_NUM; index++) {
		double qber = qberStat[index];
		double shannonRate = -log(qber) * qber / log(2.0) - log(1 - qber) * (1 - qber) / log(2.0);
		double retainStat = (double)(codeSum[index] * 8 - leakedStat[index]) / inputStat[index];
		double f = (1 - retainStat) / shannonRate;
		double speed = inputStat[index] / (durationCascade * 1000);  // Mbps

		printf("��%d��, �߳�: %d , �ܺ�ʱ: %.2f ms, ��������: %.2f Mbps\n", t, index, durationCascade, speed);
		printf("������:%lf, ����֡��: %d, �ܽ�������: %d, Э��Ч��: %.3f\n\n", qber, inputStat[index] / (64 * 1024), interactStat[0], f);
	}
}

SOCKET serverSocketInit()
{
	// 0. ��ʼ��WSA    
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0) {
		return 0;
	}

	// 1. �����׽���    
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET) {
		printf("Socket Error !");
		return 0;
	}

	// 2. ��IP�Ͷ˿�    
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(HOSTPORT);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR) {
		printf("Bind Error !");
		exit(-1);
	}

	return slisten;
}

SOCKET clientSocketInit()
{
	// 0. ��ʼ��WSA    
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;

	if (WSAStartup(sockVersion, &wsaData) != 0) {
		return 0;
	}

	// 1. �����׽���
	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sclient == INVALID_SOCKET) {
		printf("Invalid Socket!");
		return 0;
	}

	return sclient;
}

sockaddr_in clientToServerAddr()
{
	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(HOSTPORT);
	serAddr.sin_addr.S_un.S_addr = inet_addr(HOSTIP);

	return serAddr;
}

bool initClientConn(SOCKET* clientSocket)
{
	bool flagClient = false;

	//��������Socket
	sockaddr_in clientConnectAddr;

	while (!flagClient) {
		for (int i = 0; i < CONN_SUM; i++) {
			clientSocket[i] = clientSocketInit();
			clientConnectAddr = clientToServerAddr();

			if (connect(clientSocket[i], (sockaddr *)&clientConnectAddr, sizeof(clientConnectAddr)) == SOCKET_ERROR) {   //����ʧ��   
				printf("connect error !");
				closesocket(clientSocket[i]);
				Sleep(1000);
				continue;
			}
			else {
				printf("Recieve:\n");
				char recData[255];
				int ret = recv(clientSocket[i], recData, 255, 0);
				if (ret > 0) {
					recData[ret] = 0x00;
					printf(recData);
				}
				printf("Sokcet = %d\n", clientSocket[i]);
			}
		}

		flagClient = true;
	}

	if (flagClient)
		printf("Connection has been established\n");
	else
		printf("Connect Error\n");

	return flagClient;
}

bool initServerConn(SOCKET& serverSocket, SOCKET* clientSocket)
{
	bool flagServer = false;
	serverSocket = serverSocketInit();

	//��������Socket
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);

	while (!flagServer) {
		//��ʼ����    
		if (listen(serverSocket, 5) == SOCKET_ERROR) {
			printf("listen error !");
			return 0;
		}

		for (int i = 0; i < CONN_SUM; i++) {
			printf("�ȴ�����...\n");
			clientSocket[i] = accept(serverSocket, (SOCKADDR *)&remoteAddr, &nAddrlen);

			if (clientSocket[i] == INVALID_SOCKET) {
				printf("accept error !");
				continue;
			}
			else {
				printf("���ܵ�Socket���ӣ�%s \r\n", inet_ntoa(remoteAddr.sin_addr));
				//��������    
				const char * sendData = "Hello, Alice!\n";
				send(clientSocket[i], sendData, strlen(sendData), 0);

				printf("Sokcet = %d\n", clientSocket[i]);
			}
		}

		flagServer = true;
	}

	if (flagServer)
		printf("Connection has been established\n");
	else
		printf("Connect Error\n");

	return flagServer;
}