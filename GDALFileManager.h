// GDALFileManager.h: interface for the CGDALFileManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GDALFILEMANAGER_H__5F0E867D_59AD_4884_A337_BD382A4FD558__INCLUDED_)
#define AFX_GDALFILEMANAGER_H__5F0E867D_59AD_4884_A337_BD382A4FD558__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "hapcfile.h"
#include <string>


using namespace std;
USING_NAMESPACE(HAPLIB)


class HAPCLASS CGDALFileManager   // : public HpsCFileManager  
{
public:
	CGDALFileManager();
	virtual ~CGDALFileManager();

	//文件、图象属性操作
	void GetFileName(char filename[HAP_MAX_PATH]);
	bool LoadFrom(const char* Imfilename);

	void GetDIMSDataByBlock(const DIMS &dims,int i, int NumOfBlock,BYTE **data,int &datasize);
	/************************************************************************/
	/* 通过缓存数据块大小，并获取图像数据指针       
	DIMS& SBlockInfo: 缓存块信息
	BYTE *data：返回数据内容，内存不在此分配 
	*/
	/************************************************************************/
	void GetDIMSDataByBlockInfo(SimDIMS* pSBlockInfo, BYTE *data);
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	bool GetDIMSDataByBlockInfo(SimDIMS* pSBlockInfo, DIMS* pDims, BYTE *data);
	//[11/18/2011 liuxiang]由于SimDims不具有记录波段功能，这里添加DIMS参数。
	bool WGetDIMSDataByBlockInfo(SimDIMS* pSBlockInfo, DIMS* pDims, BYTE *data);

	//获取GDAL定义的数据类型
//	inline GDALDataType GetGDALDataType();
	///************************************************************************/
	///* 依据内存块大小，获取图像总的分块个数，这里按波段分块取整                                                                     */
	///************************************************************************/
	//int InitBlocks(DIMS dims); 
	////	bool BlocksDone(void);
	////	int GetBlockInBand(const int i, const int NumOfBlock,int &StrRow,int &NumofRow);

	////块属性
	////依据图像行号获取起块索引编号，在块中的始行位置，高度等信息。
	//int GetBlockSize(int row,int &StRow,int &NumRow,int &NOBlock);
	//int GetBlockSize(int index, int numofblock);

	/************************************************************************/
	/* 通过图像行、列、波段号直接确定该行所在的块的相关信息。                                                                     */
	/************************************************************************/
	void GetBlockInfo(int col, int row, int band, DIMS* pSBlockInfo);

	// 	保存类方法
	bool HpsSaveImageAs(const char *szImageName);  //非内存模式
	bool WriteBlock(BYTE *pdata, int buffersize);
	bool SaveHeader(string sFileHeader);	

	//多波段写入数据，buffersize为buffer真实大小，包括空区域
	//因为buffer设置寻址的原因，writedonebuffer为在buffer内存储的数据量大小
	//writedownbuffer = writecount*ptLength
	bool WWriteBlockB(BYTE *pdata, int buffersize);	
	bool WWriteBlock2(BYTE *pdata, int buffersize, int writedonebuffer);//yy编写
	bool WWriteBlock(BYTE *pdata, int buffersize);
	//依据缓存区信息写出图像数据
	bool WriteBlockByInfo(BYTE *pdata, SimDIMS* pSBlockInfo);
	// 	//模式管理
	bool BoolIsImgInMem();

	//初始化，结束
	void Close();
	bool Open();

	int m_HAPFlag;				//文件类型标志——0: GDAL支持类型；1: hap平台定义类型；2: 外部扩展类型。
	HAPCFileHeader m_header;    //图像信息头结构对象
	char pcErrMessage[256];		// 保存错误提示信息。


protected:
	void SetFileName(const char *filename); //设置图像文件路径

	void *m_poDataset; //GDALDataset对象，在用的时候再转换。
//	int m_iCurBand;//当前波段号标志
	FILE *m_OutFile;
//	char* m_pcFileName; //文件路径指针
	char m_szFileName[HAP_MAX_PATH]; //文件名称
	char m_szHeaderFileName[HAP_MAX_PATH]; //头文件名称
	int iCutBlockMode; //=0, 默认按行分块读取；
};

#endif // !defined(AFX_GDALFILEMANAGER_H__5F0E867D_59AD_4884_A337_BD382A4FD558__INCLUDED_)
