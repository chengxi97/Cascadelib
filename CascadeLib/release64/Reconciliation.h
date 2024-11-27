#ifndef _RECONCILIATION
#define _RECONCILIATION

#include <winsock.h>

void initCascade(double qber, SOCKET* ptrConn, int flag, void** cascadeInst);     // 初始化Cascade相关参数
int getCascadeBuf(unsigned char** data, unsigned char** code, void* cascadeInst);      // 获取Cascade缓存, 返回dataSize
//void cascadeEC(void* cascadeInst, int* codeSize, int* leakedSum);     // Cascade纠错函数
int cascadeEC(unsigned int* codeSize, void* cascadeInst);     // Cascade纠错函数
void getCascadeStat(double* qber, unsigned int* leaked, unsigned int* inputNum, unsigned int* interact, void* cascadeInst);      // 获取Cascade统计数据

#endif