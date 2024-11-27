#ifndef _RECONCILIATION
#define _RECONCILIATION

#include <winsock.h>

void initCascade(double qber, SOCKET* ptrConn, int flag, void** cascadeInst);     // ��ʼ��Cascade��ز���
int getCascadeBuf(unsigned char** data, unsigned char** code, void* cascadeInst);      // ��ȡCascade����, ����dataSize
//void cascadeEC(void* cascadeInst, int* codeSize, int* leakedSum);     // Cascade������
int cascadeEC(unsigned int* codeSize, void* cascadeInst);     // Cascade������
void getCascadeStat(double* qber, unsigned int* leaked, unsigned int* inputNum, unsigned int* interact, void* cascadeInst);      // ��ȡCascadeͳ������

#endif