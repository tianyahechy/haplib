//影像自动满行分块读写接口模块，采用多线程方式实现IO，与计算线程同步
//读，满行分块，独立线程，双缓存
//写，独立线程，双缓存
#pragma once 
#include "HAPBEGTBase.h"

USING_NAMESPACE(HAPLIB)

class HAPCLASS HAPBEGThread : public HAPBEGTBase
{
public:
	HAPBEGThread();
	virtual ~HAPBEGThread();

private:
	//设置读文件指针
	bool Set_MgrIn(CGDALFileManager * mgr, DIMS dims, int iBlockOverlap = 0);
	//设置写文件指针
	bool Set_MgrOut(CGDALFileManager * mgr, const char* fileName) = 0;
	//关闭写指针
	void Close_MgrOut();
	//关闭读指针
	void Close_MgrIn();
	//使用独立IO线程优化算法的前提
	//用户必须知道算法读取图像数据的顺序，这里仅支持按行读取
	void * Read(long column, long line, long band);
	//顺序写出数据
	bool Write(void * pValue);

protected:
	//通过图像块索引号获取缓存块信息
	//输入索引块号，获取对应缓存块信息
	void getBlockInfoByIndex(
		int iIndex,				//索引块号，从0开始计数
		SimDIMS * pSBlockInfo,	//返回的缓存信息，事先创建对象
		bool InOrOut = true
		);
	//获取输入图像或输出图像分块总数
	void setBlockNumber(bool inOrOut = true);
	//通过行列波段号获取图像块编号
	int getBlockIndex(long column, long line, long band);
	//独立写入线程，执行过程中与输出线程互斥
	//在读线程中负责读缓存区赋值，受保护成员函数，外部无法调用
	static void independentInThread(void * dummy);
	int DiskDataToBlockIn();
	//独立输出线程，执行过程中与写入线程互斥
	static void IndependentOutThread(void * dummy);
	int blockOutToDiskData();
	//输入一级缓冲区信息
	SimDIMS m_SBlockInInf;
	//输入二级缓冲区信息
	SimDIMS m_SBlockInInf2;
	//输入一级缓冲区，用于计算
	BYTE * m_bBlockIn;
	//输入二级缓冲区，用于磁盘内存交换
	BYTE * m_bBlockIn2;
	//写出内存缓冲区，一级缓冲区
	BYTE * m_bBlockOut;
	//输出磁盘缓冲区，二级缓冲区
	BYTE * m_bBlockOut2;
	//输入缓冲区大小，暂时没用
	int m_InBufferSize;
	//输出一级缓冲区大小，也是缓冲区状态标记
	int m_outBufferSize;
	//输出二级缓冲区大小，也是缓冲区状态标记
	int m_outBufferSize2;
	//已经输出的像元数，一级写缓冲区状态标记，-1表示缓冲区进入无效状态
	int m_WriteDoneCount;
	//输出到磁盘的缓冲区信息
	SimDIMS m_SBlockOutInf;
	//目前IO读取的图像块，也是读线程执行的标记，从0开始计数，-1表示执行
	int m_iCurrentBlockNo;
	//目前IO写线程读取的图像块，也是读线程执行的标记，从0开始计数，-1表示执行完毕
	int m_iCurrentOutBlockNo;
	//图像块总数，用于控制线程结束
	int m_iBlockNum;
	int m_iBlockNumOut;
	//唤醒图像IO读取线程目前需要读数据
	bool m_bNeedsDataNow;
};
