#pragma once
#include "resource.h"
#include "..\Algolib\include_global.h"

#define _WINDOWS
#define __HAPLIB__
#ifdef _WINDOWS
#ifndef __HAPLIB__
#define HAPCLASS		__declspec(dllimport)
#define	HIPAPI			__declspec(dllimport)
#define HIPVAR			__declspec(dllimport)
#else
#define HAPCLASS		__declspec(dllexport)
#define	HIPAPI			__declspec(dllexport)
#define HIPVAR			__declspec(dllexport)
#endif
#endif

#ifdef _LINUX
#define HAPCLASS	
#define HIPAPI
#define HIPVAR	
#endif

#pragma warning(disable:4172)
#define snprintf _snprintf
#define BEGIN_NAMESPACE(name)	namespace name {
#define END_NAMESPACE(name)			};
#define USING_NAMESPACE(name)	using namespace name;

BEGIN_NAMESPACE(HAPLIB)

#undef TRUE
#define TRUE	1
#undef FALSE
#define	FALSE	0
#undef NULL	
#define NULL	0

#define VOID	void
#define	CONST	const
#define	STATIC	static
#define	HEADEROFFSET	1024
#define	HAP_MAX_PATH	1024
#define	MAXBAND			3*1024
#define HPS_LUT_LOOKUP	1
#define	NAMELEN			512
#define HPSFONTSIZE		32
#define	HPSTEXTSIZE		128
#define	PIE				3.1415926

#define	HPSC_MICROIMAGE	1
#define HPSC_VIEWIMAGE	2
#define	HPSC_ZOOMIMAGE	3
#define	HPSC_INITFILE	"hipas.ini"

typedef enum
{
	HPS_POINT = 0,
	HPS_POINTS = 1,
	HPS_PARTS = 2,
	HPS_LINE = 3,
	HPS_RECT = 4,
	HPS_POLYGON = 5,
	HPS_ELLIPSE = 6,
	HPS_ANNO = 7
}ShapeType;

typedef enum
{
	HAP_Unkown = 0,
	HAP_Byte = 1,
	HAP_Int16 = 2,
	HAP_Int32 = 3,
	HAP_Float32 = 4,
	HAP_Float64 = 5,
	HAP_UInt32 = 6,
	HAP_Int8 = 7,
	HAP_CInt16 = 8,
	HAP_CInt32 = 9,
	HAP_CFloat32 = 10,
	HAP_CFloat64 = 11,
	HAP_UInt16 = 12
}HAPDataType;

//统计图像信息结构体，存放包括遥感图像的最大值，最小值，均值，标准差，直方图等信息，避免对图像的反复统计，这里一次性记录
struct HAPCLASS StImgSta
{
	float fMax;
	float fMin;
};

//缓存区信息结构,simpledims其中的成员不可直接映射到图像上指定位置
//用途，用于底层缓存信息标记
//注：SimDIMS需要联合对应的图像DIMS才能够映射到图像中的信息结构
struct HAPCLASS SimDIMS
{
	//x起始点，可以取到
	long xStart;
	//x终止点，不可达到
	long xEnd;
	//y起始点，可以取到
	long yStart;
	//y终止点，不可达到
	long yEnd;
	//y方向高度
	long height;
	//x方向宽度
	long width;
	//BSQ->行读取模式：当前波段位置，从0开始计数
	//光谱读取模式：选中的波段总数
	//-1表示Simdims没有赋值
	int band;
	//每个波段大小
	int bandSize;
	//波段偏移量
	int bandOffset;
	//指向指定范围的数据块，SimDIMS不负责管理堆内存
	BYTE * pData;
	SimDIMS() : xStart(-1), xEnd(-1), yStart(-1), yEnd(-1), band(-1)
	{
		pData = NULL;
	}
	SimDIMS(int iXStart, int iXEnd, int iYstart, int iYEnd, int iBand = 1)
	{
		pData = NULL;
		xStart = iXStart;
		xEnd = iXEnd;
		yStart = iYstart;
		yEnd = iYEnd;
		band = iBand;
	}
	~SimDIMS()
	{

	}
	//重新初始化
	void InitDIMS();
	//块宽度
	inline void SetHeightWidth();
	inline int GetHeight();
	inline int GetWidth();
	//依据图像行列号，获取缓存块位置
	inline long GetCacheLoc(int column, int line);
	//通过在dims内的波段号和行列号，获取缓存块的位置
	inline long WGetCacheLoc(int iBand, int column, int line);
	inline long GetCacheSize();
	inline long WGetCacheSize();
};

//用于用户波段选择，范围选择功能实现
struct HAPCLASS DIMS: SimDIMS
{
	//波段指针，表明用户选择的波段数，置1表示该波段选中， 置0表示没选中
	bool * m_pBand;
	//图像总波段数，也是m_pBand数组的大小
	int m_iBandNum;
	DIMS();
	//int iBand:图像是哪个总波段数，且全部选中
	DIMS(int iXStart, int iXend, int iYstart, int iYend, int iBand = -1);
	//深拷贝
	DIMS(DIMS& DimsCopy);
	~DIMS();
	DIMS& operator= (const DIMS& DimsCopy);
	void InitDIMS();
	//int iBand:图像总波段数，且全部选中
	void InitDIMS(int iXStart, int iXend, int iYstart, int iYend, int iBand = 1);
	int getDIMSBands();
	//因为存在波段选择，用户编程时的波段编号可能与图像实际的波段编号不一致。
	//知道DIMS波段编号，获取图像波段编号，获取图像波段编号，为了方便上层用户编程，这里ibandno从0开始
	int GetImgBandNum(int iBandNo);
	//知道图像波段编号，获取真实波段数在dims中的no编号，参数从0开始，返回值从0开始
	int getDimsBandNo(int iRealBandNo);

};

//这里只是变量的声明，具体在哪里定义编译器会搜索其他源文件
template <class T>
class HHpsCMatrix;
//缓存开辟大小，部分算法可自行设置
extern HIPVAR int	g_BlockSize;
//写缓存开辟大小
extern HIPVAR int	g_BlockSizeW;
//缓存开辟大小，部分算法可自行设置
extern HIPVAR DIMS	g_Zdims;
//square定义正方形块边长
extern HIPVAR int	g_SquareBorderSize;
END_NAMESPACE(HAPLIB)