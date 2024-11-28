# 项目概述

本项目基于论文“High performance reconciliation for practical quantum key distribution systems”中提出的Cascade纠错算法。
重要：本程序仅为仿真测试程序，为支撑文章实验结果所用，内部可能存在隐藏BUG，请谨慎用于生产环境。程序具体设计细节，详见论文（https://arxiv.org/abs/2101.12565）。

# 项目使用说明

待续

# Cascade纠错函数使用说明

## 1 函数 initCascade()

说明：初始化 Cascade 相关参数。

原型：
void initCascade(double qber, SOCKET* ptrConn, int flag, void** cascadeInst);

输入参数：
- qber: 估计误码率（测试时可简单取为 QBER 最大值）。
- ptrConn: Socket 连接数组（一个线程需要提供两个 Socket，分别用来实现纠错和校验信息传递）。
- flag: Bob 端取值为 1，Alice 端取值为 0。

输出参数：
- cascadeInst: 保存声明对象。

返回值：无。

---

## 2 函数 getCascadeBuf()

说明：获取 Cascade 缓存，返回数据大小。

原型：
int getCascadeBuf(unsigned char** data, unsigned char** code, void* cascadeInst);

输入参数：
- cascadeInst: 将 initCascade() 获取的对象地址输入。

输出参数：
- data: 输入数据缓存指针（后续将数据输入至该地址即可）。
- code: 输出码字缓存指针（码字和数据采用不同缓存）。

返回值：输入数据大小。

---

## 3 函数 cascadeEC()

说明：Cascade 纠错函数（核心函数）。

原型：
int cascadeEC(unsigned int* codeSize, void* cascadeInst);

输入参数：
- cascadeInst: 将 initCascade() 获取的对象地址输入。

输出参数：
- codeSize: 本组输出码字长度。

返回值：
- 返回值为 1: 数据量达到 PA 预设值。
- 返回值为 0: 未达到，需要继续纠错。

---

## 4 函数 getCascadeStat()

说明：获取 Cascade 统计数据。

原型：
void getCascadeStat(double* qber, unsigned int* leaked, unsigned int* inputNum, unsigned int* interact, void* cascadeInst);

输入参数：
- cascadeInst: 将 initCascade() 获取的对象地址输入。

输出参数：
- qber: 统计得到的信号态误码率。
- leaked: 输出码字暴露信息量。
- inputNum: 输入数据量（不包含仍在流水中的数据）。
- interact: 完成本组纠错的交互次数。

返回值：无。
