#pragma once
#include "HAPBEGTBase.h"
USING_NAMESPACE(HAPLIB)
//影像自动分块读写接口模块，采用多线程方式实现IO，按波段读取，与计算线程同步，W：wave
//读：满波段满行分块，独立线程，双缓存
//写：独立线程，双缓存，满行满波段分块输出.写出时，先for波段，再for行列
class HAPCLASS HAPBEGThreadW : public HAPBEGTBase
{
public:
	HAPBEGThreadW();
	virtual ~HAPBEGThreadW();
	//运算符重载只能写到.h文件中
	HAPBEGThreadW& operator= (const HAPBEGThreadW& otherImgIO)
	{
		if (this == &otherImgIO)
		{
			return *this;
		}
		memcpy(this, &otherImgIO, sizeof(HAPBEGThreadW));
		//只深拷贝读缓存
		m_bBlockIn = new BYTE[g_BlockSize];
		m_bBlockIn2 = new BYTE[g_BlockSize];
		//双缓存输出暂时不用，形式
		m_bBlockOut = new BYTE[10];
		m_bBlockOut2 = new BYTE[10];
		return *this;
	}
	//设置读文件指针
	bool Set_MgrIn(CGDALFileManager * mgr, DIMS dims, int iBlockOverlap = 0);
	//设置写文件指针
	bool Set_MgrOut(CGDALFileManager * mgr, const char* fileName);
	//关闭写指针
	void Close_MgrOut();
	//关闭读指针
	void Close_MgrIn();
	//改为公有成员，方便多核计算调用
	//通过行列波段号获取图像块编号
	int GetBlockIndex(long column, long line, long band);
	//获取图像块总数
	int getBlockNumber(bool InOrOut);
	int getBlockLinesByIndex(int iIndex, bool InOrOut);
	//使用独立IO线程优化算法的前提：
	//用户必须知道算法读取图像数据的顺序，因为这里仅支持顺序读取
	//默认按行读取
	void * Read(long column, long line, long band) ;
	//BSQ按波段顺序写出数据
	bool Write(void * pValue);
protected:
	//通过图像块索引号获取缓存块信息
	void GetBlockInfoByIndex(
		int iIndex,				//索引块号，从0开始计数
		SimDIMS* pSBlockInfo,	//返回的缓存块信息，事先创建对象
		bool InOrOut = true);
	//获取图像块总数
	void setBlockNumber(bool InOrOut = false);
	//独立写入线程，执行过程中与输出线程互斥
	//在读线程中负责读缓存区赋值，受保护成员函数，外部无法调用
	static void independentInThread(void * dummy);
	int DiskDataToBlockIn();
	//独立输出线程，执行过程中与写入线程互斥
	//在写线程中负责写缓存区输出，受保护成员函数，外部无法调用
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
	//0:缓冲区重叠不存在；1-3：缓存区存在1-3行数据重叠，例如n*n（奇数）模板中值滤波，则需要第n-1行数据重叠。保证缓存块交换不会缺少数据。
	//若n为偶数，则需要n行
	//重叠区设为偶数，方便输出缓存区位置计算，可以对半分
	//标记前m_QueueLength图像块编号
	//目前IO读线程读取的图像块，也是读线程执行的标记，从0开始计数，-1表示执行完毕
	int m_iCurrentBlockNo;
	//目前IO写线程读取的图像块，也是读线程执行的标记，从0开始计数，-1表示执行完毕
	int m_iCurrentOutBlockNo;
	//图像块总数，用于控制线程结束
	int m_iBlockNum;
	int m_iBlockNumOut;
	//唤醒图像IO读取线程目前可以读数据
	bool m_bNeedsDataNow;

};
