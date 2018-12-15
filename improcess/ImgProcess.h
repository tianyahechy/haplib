#pragma once

#include "hpscegsbaseTmpl.h"
#include "HAPBEGThread.h"

#define IMGPROCESS_EXPORTS
#ifdef IMGPROCESS_EXPORTS
#define IMGPROCESS_API	__declspec(dllexport)
#else
#define IMGPROCESS_API	__declspec(dllimport)
#endif

#pragma warning(disable : 4996)
//方便用户调用数据
#define ImgRead *(TT*) m_ImgIO->Read
#define ImgReadL *(TT*) m_ImgIO->ReadL
#define ImgWrite(Value) *(TT*) m_ImgIO->Write(&Value);

//图像处理基类，其他算法继承该类
class IMGPROCESS_API Base_IPAS
{
public:
	Base_IPAS();
	virtual ~Base_IPAS();
	void Set_FileName(const char* fileName);
	//标准处理，即该算法类最常规的处理-输入图像文件，准备处理参数，计算，输出图像文件
	//初始化m_ImgIO,算法参数，依据数据类型调用不同的Process()函数
	bool StandardProcess(DIMS dims, CGDALFileManager * pmgr1, CGDALFileManager* pMgr2 = NULL);
	char * GetErrorMessage();

protected:
	//处理图像并输出数据，只与输入文件、输出文件均相关
	//int cpuNo:cpu序号，从1开始
	template<class TT>
	bool Process(int cpuNo = 0);
	//准备处理参数或同级图形参数函数，只与输入文件读取有关，与输出文件无关
	template<class TT>
	bool PreparePara(int cpuNo = 0);
	//采用动态联编机制，子类可以指定为HAPBEGThread类对象，也可以申请多个，但基类只提供1个
	HAPBEGTBase* m_ImgIO;
	//文件路径
	char m_FileName[512];
	//输入数据类型约定，模板函数和计算部分参数使用
	HAPDataType m_iDataTypeIn;
	//处理出错时返回信息
	char m_pcErrMessage[256];

};
