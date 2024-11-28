# 1.项目概述

本项目基于论文 *"High performance reconciliation for practical quantum key distribution systems"* 中提出的 Cascade 纠错算法。文章实现了两种模式的Cascade算法：“High Throughput”和“High Efficiency”。本程序采用“High Efficiency”（高协商效率）模式，因其处理速率已能满足大多数 QKD 系统的需求。

**重要提示**：本程序为仿真测试程序，用于支撑论文实验结果，内部可能存在隐藏 BUG，请谨慎用于生产环境。更多设计细节可参考论文：https://arxiv.org/abs/2101.12565 。

---

# 2.项目说明

## 2.1 项目目录结构

CascadeProject 
├── CascadeLib
   │ ├── x86
   │ ├── x64
├── Example
   │ ├── CascadeAlice
   │ ├── CascadeBob
   
- **CascadeLib**：包含 x86 和 x64 版本的 `.h` 文件、`.dll` 文件和 `.lib` 文件。
- **Example**：包含测试例程 `CascadeAlice` 和 `CascadeBob`，具体使用方式可参考该例程。

## 2.2 项目使用说明

本项目在 Windows 操作系统下使用 Visual Studio 2019 编译和测试通过。库文件的使用方式与普通 `.dll` 文件一致。

### 测试例程使用方法

1. 本项目采用离线文件方式作为筛选码的输入，文件路径可通过修改测试例程代码进行设置。
2. 按以下顺序运行测试程序：
   - 先运行 `CascadeBob`。
   - 再运行 `CascadeAlice`。
3. 程序将自动读取离线文件，完成纠错操作。
4. 调用 `CascadeEC()` 函数后，`codeList` 即为输出协商码，可将其保存到文件中以校验结果。

### 性能优化建议

- 本程序采用流水处理模式，流水的创建和销毁会带来性能损耗。为获得最佳性能，建议连续不断地输入数据。
- 程序内部会对数据进行处理（包括置乱），因此输出数据序列的比特顺序与输入序列不完全一致。测试人员可通过监测网络数据量估算泄露信息量。

## 2.3 Cascade纠错函数使用说明

### 函数 initCascade()

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

### 函数 getCascadeBuf()

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

### 函数 cascadeEC()

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

### 函数 getCascadeStat()

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

# 3. FAQ

Q: 纠错后密钥输出在哪里？
A: 测试例程中，CodeList即为输出码字，其长度为codeSize，将其输出到文件中即可实现比较。

Q: 纠错失败的帧是如何处理的？
A: 误帧率（Frame Error Rate，FER）约为6.7*10^(-4)左右，若译码失败则该子分组会直接丢弃。

Q: 为什么输出数据帧数不是定值？
A: 因此该程序通过流水方式进行处理，在缓冲池中会存在一些未成完整帧的数据，有时这部分数据量还会较大（可能达到几个数据帧）。此外由于采用流水处理，退出时流水中可能还存在一些未处理的数据，上述数据均需要等下次调用时和后续数据一同处理。

Q: 为什么输出数据帧和输入数据帧长度不一样？
A: 产生该现象的原因是程序对块长为1的数据块的处理。目前程序采用的方式是块长为1的数据块直接丢弃（与FER的处理类似）。
