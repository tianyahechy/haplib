#pragma once

#include "GDALFileManager.h"
USING_NAMESPACE(HAPLIB)

//统一管理图像处理算法使用的缓存
//模板类、模板函数在编写使用的时候一定需要让实例化代码能够直接找到模板类的定义，而不是只找到声明

class HAPCLASS HAPBEGBase
{
public:
	HAPBEGBase();
	virtual ~HAPBEGBase();

	CGDALFileManager * Get_MgrIn();
	CGDALFileManager * Get_MgrOut();

	//设置读文件指针
	bool Set_MgrIn(CGDALFileManager * mgr, DIMS dims);
	//设置写文件指针
	bool Set_MgrOut(CGDALFileManager * mgr, const char * fileName);
	//关闭写指针保护
	void Close_MgrOut();
	//关闭读指针保护
	void Close_MgrIn();

	//通过图像行、列波段号直接确定该行所在的块的相关信息
	//按行分块
	void RGetBlockInfo(int col, int row, int band, SimDIMS * psBlockInfo);
	//按列分块
	void CGetBlockInfo(int col, int row, int band, SimDIMS * psBlockInfo);
	//按正方形分块
	void BGetBlockInfo(int col, int row, int band, SimDIMS * psBlockInfo);
	//按波段分块
	void WGetBlockInfo(int col, int row, int band, SimDIMS * psBlockInfo);

	//读像素指针，这里按照考虑BSQ方式读入
	//默认按行读取
	virtual void * RRead(long column, long line, long band);
	//按列读取
	void * CRead(long column, long line, long band);
	//按块读取Block Read
	void * BRead(long column, long line, long band);
	//按波段读取
	void * WRead(long column, long line, long band);

	//写像素值函数，按照BSQ、bip或者BIL方式顺序，不可随机写入
	//写数据
	virtual bool Write(void * pValue);
	//根据波段号，写入数据
	virtual bool WWrite(void * pValue, int band);
	//输入DIMS信息
	DIMS m_DimsIn;
	//输出DIMS信息
	DIMS m_DimsOut;
	//保存错误提示信息
	char pcErrMessage[64];

	//请求分配缓存空间，并且返回缓存指针。每一次请求必须紧跟着为缓存赋值，否则会冲掉最常用的数据
	virtual BYTE * RequestCache(SimDIMS * &pSBlockInf);
	//输入文件管理指针
	CGDALFileManager * m_MgrIn;
	//输出文件管理指针
	CGDALFileManager * m_MgrOut;
	bool m_bIsReadSample;
	bool m_OutInMem;
	//数据字节长度，pixel tO length
	int m_nPtLength;
	//输出数据长度
	int m_nPtLengthOut;

	int m_nOffsetY;
	int m_nOffsetX;

	//输入缓冲区大小
	int m_InBufferSize;
	//输出缓冲区大小
	int m_OutBufferSize;

	//输入一级缓冲区信息
	SimDIMS m_SBlockInInf;
	//输入二级缓冲区信息
	SimDIMS m_SBlockInInf2;

	//输入一级缓冲区
	SimDIMS m_SBlockIn;
	//输入二级缓冲区
	SimDIMS m_SBlockIn2;

	//输入一级缓冲区
	BYTE * m_bBlockIn;
	//输入二级缓冲区
	BYTE * m_bBlockIn2;
	//写出内存缓冲区，一级缓冲区
	BYTE * m_bBlockOut;
	//写出内存缓冲区，二级缓冲区
	BYTE * m_bBlockOut2;

	//已输出的像元数,一级写缓冲区状态标记，一级写缓存区状态标记；-1表示缓冲区进入无效状态
	int m_WriteDoneCount;
	//已输出的像元数,二级写缓冲区状态标记，一级写缓存区状态标记；-1表示缓冲区进入无效状态
	int m_WriteDoneCount2;

	//标记哪号缓冲区即将丢弃
	int m_iRejectCacheFlag;
	//0为write,1为wwrite
	int m_writeFlag;
	//计算地址的相对位置
	int m_writePos;


};