#pragma once
#include "HAPBEGTBase.h"
USING_NAMESPACE(HAPLIB)
//影像自动分块读写接口模块，采用多线程方式实现IO，与计算线程同步，
//读：正方形分块，缓存队列，不适用多线程，依据算法访问需求自由读取缓存块数据
//写：独立线程，双缓存，满行分块
class HAPCLASS HAPBEGThreadS : public HAPBEGTBase
{
public:
	HAPBEGThreadS();
	virtual ~HAPBEGThreadS();
	//设置读文件指针
	bool Set_MgrIn(CGDALFileManager * mgr, DIMS dims = g_Zdims, int iBlockOverlap = 0);
	//设置写文件指针
	bool Set_MgrOut(CGDALFileManager * mgr, const char* fileName);
	//关闭写指针
	void Close_MgrOut();
	//关闭读指针
	void Close_MgrIn();
	//使用独立IO线程优化算法的前提：
	//用户必须知道算法读取图像数据的顺序，因为这里仅支持顺序读取
	//默认按行读取
	void * Read(long column, long line, long band) ;
	//顺序写出数据
	bool Write(void * pValue);
protected:
	//通过图像块索引号获取缓存块信息
	void GetBlockInfoByIndex(
		int iIndex,				//索引块号，从0开始计数
		SimDIMS* pSBlockInfo,	//返回的缓存块信息，事先创建对象
		bool InOrOut = true);
	//获取图像块总数
	void setBlockNumber(bool InOrOut = false);
	//通过行列波段号获取图像块编号
	int GetBlockIndex(long column, long line, long band, SimDIMS * pSBlockInfo);
	//输入DIMS信息
	DIMS m_DimsIn;
	//保存错误提示信息，供上层调用
	char pcErrMessage[64];
private:
	//输入文件管理指针
	CGDALFileManager * m_MgrIn;
	//输出文件管理指针
	CGDALFileManager * m_MgrOut;
	//数据字节长度
	int m_nPtLength;
	//输出数据长度
	int m_nPtLengthOut;
	//0缓冲区重叠不存在，1-3，缓冲区存在1-3行数据重叠，例如n*n（奇数）模板中值滤波，则需要n-1行数据重叠，保证缓存块交换不会缺少数据。
	//若n为偶数，则需要n行。重叠区设为偶数，方便输出缓存区位置计算，可以对半分
	//用于设置缓存区的重叠，部分图像处理算法需要用到，如滤波、插值算法等
	int m_iBlockOverlap;
	//警告信息，仅提示用户一次
	bool m_bAlarmInfo;
	//线程异常，强迫各个线程退出
	bool m_bThreadAbnormal;
	//标记图像IO线程正在工作，告知其他线程不要进行磁盘IO操作
	static bool m_bIOThreadRun;
	//输出调试信息
	FILE * ptf;
private:


};
