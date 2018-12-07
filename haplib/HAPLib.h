// HAPLib.h : HAPLib DLL 的主头文件
//

#pragma once

//#ifndef __AFXWIN_H__
//	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
//#endif

#include "resource.h"		// 主符号
#include "../Algolib/include_global.h"


#define _WINDOWS	1
#define __HAPLIB__	1
#ifdef _WINDOWS
#ifndef __HAPLIB__
#define HAPCLASS	__declspec(dllimport)
#define HIPAPI		__declspec(dllimport)
#define HIPVAR		__declspec(dllimport)
#else
#define HAPCLASS	__declspec(dllexport)
#define HIPAPI		__declspec(dllexport)
#define HIPVAR		__declspec(dllexport)
#endif
#endif

#ifdef _LINUX
#define HAPCLASS
#define HIPAPI
#define HIPVAR
#endif

#pragma warning(disable:4244)

#define snprintf	_snprintf

#define BEGIN_NAMESPACE(name)	namespace name {
#define END_NAMESPACE(name)		};
#define USING_NAMESPACE(name)	using namespace name;

BEGIN_NAMESPACE(HAPLIB)
#undef TRUE
#define TRUE	1
#undef FALSE
#define FALSE	0
#undef NULL	
#define NULL	0

#define VOID	void
#define CONST	const
#define STATIC	static
#define HEADEROFFSET	1024
#define	HAP_MAX_PATH	512
#define	MAXBAND			3 * 1024
#define	HPS_LUT_LOOKUP	0
#define	HPS_LUT_INTER	1
#define	NAMELEN			512
#define	HPSFONTSIZE		32
#define	HPSTEXTSIZE		128
#define	PIE				3.1415926

#define	HPSC_MICROIMAGE	1
#define	HPSC_VIEWIMAGE	2
#define	HPSC_ZOOMIMAGE	3
#define	HPSC_INITFILE	"hipas.ini"

typedef enum
{
	HPS_POINT	= 0,
	HPS_POINTS	= 1,
	HPS_PARTS	= 2,
	HPS_LINE	= 3,
	HPS_RECT	= 4,
	HPS_POLYGON	= 5,
	HPS_ELLIPSE = 6,
	HPS_ANNO = 7

}ShapeType;

//HAP数据格式编号(ENVI)
typedef enum
{
	HAP_Byte		= 1,		//8位无符号整型
	HAP_Int16		= 2,		//16位有符号整型
	HAP_Int32		= 3,		//32位有符号整型
	HAP_Float32		= 4,		//32位有符号浮点型
	HAP_Float64		= 5,		//64位有符号浮点型
	HAP_UInt32		= 6,		//32位无符号整型
	HAP_CInt16		= 8,		//16位复合整型
	HAP_CInt32		= 9,		//32位复合整型
	HAP_CFloat32	= 10,		//32位复合浮点型
	HAP_CFloat64	= 11,		//64位复合浮点型
	HAP_UInt16		= 12		//16位无符号整型
}HAPDataType;

//统计图像信息结构体，存放包括遥感图像的最大值、最小值、均值、标准差，直方图等信息。避免对图像的反复统计，这里一次性记录
struct HAPCLASS StImgSta
{
	float fMax;
	float fMin;

};
//缓存区信息结构，Simple Dims其中的成员不可直接映射到图像上指定位置，
//用途:用于底层缓存信息标记
//SIMDIMS需要联合对应的图像DIMS才能够映射到图像中的信息结构
struct HAPCLASS SimDIMS
{
	long	xStart;	//x起点，可以取到
	long	xEnd;	//x终点，不可到达
	long	yStart;	//y起点，可以取到
	long	yEnd;	//y终止点，不可到达
	long	yHeight;//y方向高度
	long	xWidth;	//x方向宽度
	int		band;	//BSQ->行读取模式:当前波段位置，从0开始计数；光谱读取模式，选中的波段总数，-1表示SIMDIMS没有赋值
	int		bandSize;	//每个波段大小
	int		bandOffset;	//波段偏移量
	
	BYTE *	pData;	//指向指定范围的数据块，SimDIMS不负责管理堆内存
	SimDIMS()
	{
		xStart = -1;
		xEnd = -1;
		yStart = -1;
		yEnd = -1;
		band = -1;
		pData = NULL;
	}
	SimDIMS(int iXstart, int iXend, int iYstart, int iYend, int iBand)
	{
		pData = NULL;
		xStart = iXstart;
		xEnd = iXend;
		yStart = iYstart;
		yEnd = iYend;
		band = iBand;
	}
	~SimDIMS()
	{

	}
	//重新初始化
	void initDIMS();
	//块宽度
	inline void setHeightWidth();
	inline int getHeight();
	inline int getWidth();
	//根据图像行列号，获取缓存块位置
	inline long getCacheLoc(int column, int line);
	//通过在dims内的波段号和行列号，获取缓存块的位置
	inline long wGetCacheLoc(int band, int coloumn, int line);
	inline long getCacheSize();
	inline long wGetCacheSize();
};

//与基结构体作用不一样
//继承band变量，表示图像总波段数
//用途，用于用户波段选择，范围选择功能实现
struct HAPCLASS DIMS : SimDIMS
{
	//波段指针，表明用户选择的波段数，置1表示该波段选中，置0表示没选中
	bool * m_pBand;
	//图像总波段数，也就是m_pBand数组的大小
	int	m_iBandNum;
	DIMS();
	DIMS(int iXStart, int iXEnd, int iYStart, int iYEnd, int iBand = -1);
	//深拷贝
	DIMS(DIMS& DimsCopy);
	~DIMS();
	DIMS& operator= (const DIMS& DimsCopy);
	void InitDIMS();
	//int iBand:图像总波段数，且全部选中
	void InitDIMS(int iXStart, int iXEnd, int iYStart, int iYEnd, int iBand = 0);
	int getDIMSBands();

	//根据DIMS波段编号，获取图像波段编号
	//因为存在波段选择，用户编程时的波段编号可能与图像实际的波段编号不一样
	int getImgBandNum(int iBandNo);
	//知道图像波段编号，获取真实波段数在dims中的NO编号，参数从0开始,返回值从0开始
	int getDIMSBandNum(int iRealBandNo);

};

//这里只是变量的声明，具体在哪里定义会搜索其他源文件
template <class TT>
class  HHpsCMatrix;

//缓存开辟大小，部分算法可自行设置
extern HIPVAR int		g_BlockSize;
//写缓存开辟大小
extern HIPVAR int		g_BlockSizeW;
//缓存开辟大小，部分算法可自行设置
extern HIPVAR DIMS		g_Zdims;
//定义正方形块边长
extern HIPVAR int		g_SquareBorderSize;
END_NAMESPACE(HAPLIB)