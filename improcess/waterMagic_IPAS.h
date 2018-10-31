
#include "ImProcess.h"
#include "HAPBEGThreads.h"
#include <vector>

#pragma once
struct st_PointV
{
	int x;
	int y;
	float valueThreshold;

};

class IMGPROCESS_API WaterMagic_IPAS : public BASE_IPAS
{
public:
	WaterMagic_IPAS();
	~WaterMagic_IPAS();
	void Set_FileName(const char * fileName);

	//标准处理，即该算法类最常规的处理---输入图像文件、计算、输出图像文件
	//初始化m_ImgIO,算法参数，依据数据类型调用不同的process()函数
	bool standardProcess(DIMS dims, CGDALFileManager * pmgr1, CGDALFileManager* pmgr2 = NULL);
	bool NDWICutStdProcess(DIMS dims, CGDALFileManager * pmgr1, CGDALFileManager* pmgr2 = NULL);
	//准备处理参数函数，在中间结果存在时调用，用于获取种子点植被指数值
	bool PreparePara();
	template <class TT> bool NDWIProcess();
	//实现对NDWI图像的自动二值化分割
	bool NDWICut(GDALFileManager* pmgr2 = NULL);
	//seedXY[2]会分配内存，类中会释放
	bool ReadWaterMagicINI(
		int& iNirBand,
		int& iBlueBand,
		float& ImageThold,
		float& fOffset,
		int& smallSpot,
		int* seedXY[2],
		int& seedNum
		);
	bool SetWaterMagicINI(
		int iNirBand,
		int iBlueBand,
		float ImageThold,
		float fOffset,
		int smallSpot,
		int* seedXY[2],
		int seedNum
		);
	//获取结果数据
	BYTE* GetProcessedImgMem(
		int& iRegSamples,
		int& iRegLines,
		int& Xstart,
		int& Ystart
		);
	bool SaveStaticFile(float fMax, float fMin);
	bool GetStaticFile(float& fMax, float& fMin);
	bool TrackSpotBySeed(
			long lCurPos,
			HAPBEGThreadS * m_ImgIO,
			//标识该像元是否已经被搜索
			//0:初始非目标存储位
			//1:已经搜索，非目标
			//2:已经搜索，目标
			//255:初始目标存储位
			BYTE** ppBOutImage,
			//记录生长的外接矩形区域
			SimDIMS& stSeedRegion,
			//进度条使用
			int iSeedNumber = 0,
			BOOL bCheck8 = TRUE
		);
	//输出文件路径
	char m_OutFileName[512];
	//中间临时文件路径
	char m_TempFileName[512];
	//取值范围的最大值，适用于无符号数据
	int m_iMaxValue;
	//输入数据类型约定，模板函数使用
	int m_iDataTypeIn;
	//种子点个数
	int m_SeedNum;
	//存放种子点X,Y坐标
	int * m_SeedCoor[2];
	//水体提取门限值，全局一个值
	float m_ImageThold;
	//水体提取门限值上确界在需要的时候使用
	float m_ImageTHoldUp;
	//门限值偏移量
	float m_fOffset;
	//小斑门限
	int m_smallSpot;
	//近红外，吸收波段
	int m_iNirBand;
	//透射波段
	int m_iBlueBand;
	//输出结果参数
	//目标识别二值图像内存块，只能取值0，1
	BYTE* m_pBSeedReg;
	//图像宽
	int m_iRegSamples;
	//图像高
	int m_iRegLine;
	//图像X方向偏移量
	int m_XStart;
	//图像Y方向偏移量
	int m_YStart;

};

BOOL TrackSpotBySeedByV3(
	long lCurPos,
	//标识该像元是否已经被搜索，
	//0:初始非目标存储位
	//1:已经搜索，非目标
	//2:已经搜索，目标
	//255:初始目标存储位
	BYTE** ppBOutImage,
	long lFileWidth,
	long lFileHeight,
	//当前生长区域的值（即种子点的值），
	//fregionvalue不能为0，1，2，3
	BYTE fRegionValue,
	//记录生长的外接矩形区域
	SIMDIMS& stSeedRegion,
	BOOL bCheck8 = TRUE
);

BOOL TrackSpotBySeedByV2(
	long lCurPos,
	//标识该像元是否已经被搜索，
	//1:已经搜索，不论小斑、非小斑区域
	//fRegion:
	//255:初始目标存储位
	BYTE** ppBOutImage,
	long lFileWidth,
	long lFileHeight,
	//记录区域包含的像素序号
	std::vector<int>& pixels,
	//标记为边缘区域，即非小斑（小斑在边缘，说明不是目标内部的小斑，不予关注）
	bool& bEdgeReg,
	//当前生长区域的值（即种子点的值），一般为0
	BYTE fRegionValue,
	BOOL bCheck8 = FALSE
	);

