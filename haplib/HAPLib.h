// HAPLib.h : HAPLib DLL ����ͷ�ļ�
//

#pragma once

//#ifndef __AFXWIN_H__
//	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
//#endif

#include "resource.h"		// ������
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

//HAP���ݸ�ʽ���(ENVI)
typedef enum
{
	HAP_Byte		= 1,		//8λ�޷�������
	HAP_Int16		= 2,		//16λ�з�������
	HAP_Int32		= 3,		//32λ�з�������
	HAP_Float32		= 4,		//32λ�з��Ÿ�����
	HAP_Float64		= 5,		//64λ�з��Ÿ�����
	HAP_UInt32		= 6,		//32λ�޷�������
	HAP_CInt16		= 8,		//16λ��������
	HAP_CInt32		= 9,		//32λ��������
	HAP_CFloat32	= 10,		//32λ���ϸ�����
	HAP_CFloat64	= 11,		//64λ���ϸ�����
	HAP_UInt16		= 12		//16λ�޷�������
}HAPDataType;

//ͳ��ͼ����Ϣ�ṹ�壬��Ű���ң��ͼ������ֵ����Сֵ����ֵ����׼�ֱ��ͼ����Ϣ�������ͼ��ķ���ͳ�ƣ�����һ���Լ�¼
struct HAPCLASS StImgSta
{
	float fMax;
	float fMin;

};
//��������Ϣ�ṹ��Simple Dims���еĳ�Ա����ֱ��ӳ�䵽ͼ����ָ��λ�ã�
//��;:���ڵײ㻺����Ϣ���
//SIMDIMS��Ҫ���϶�Ӧ��ͼ��DIMS���ܹ�ӳ�䵽ͼ���е���Ϣ�ṹ
struct HAPCLASS SimDIMS
{
	long	xStart;	//x��㣬����ȡ��
	long	xEnd;	//x�յ㣬���ɵ���
	long	yStart;	//y��㣬����ȡ��
	long	yEnd;	//y��ֹ�㣬���ɵ���
	long	yHeight;//y����߶�
	long	xWidth;	//x������
	int		band;	//BSQ->�ж�ȡģʽ:��ǰ����λ�ã���0��ʼ���������׶�ȡģʽ��ѡ�еĲ���������-1��ʾSIMDIMSû�и�ֵ
	int		bandSize;	//ÿ�����δ�С
	int		bandOffset;	//����ƫ����
	
	BYTE *	pData;	//ָ��ָ����Χ�����ݿ飬SimDIMS�����������ڴ�
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
	//���³�ʼ��
	void initDIMS();
	//����
	inline void setHeightWidth();
	inline int getHeight();
	inline int getWidth();
	//����ͼ�����кţ���ȡ�����λ��
	inline long getCacheLoc(int column, int line);
	//ͨ����dims�ڵĲ��κź����кţ���ȡ������λ��
	inline long wGetCacheLoc(int band, int coloumn, int line);
	inline long getCacheSize();
	inline long wGetCacheSize();
};

//����ṹ�����ò�һ��
//�̳�band��������ʾͼ���ܲ�����
//��;�������û�����ѡ�񣬷�Χѡ����ʵ��
struct HAPCLASS DIMS : SimDIMS
{
	//����ָ�룬�����û�ѡ��Ĳ���������1��ʾ�ò���ѡ�У���0��ʾûѡ��
	bool * m_pBand;
	//ͼ���ܲ�������Ҳ����m_pBand����Ĵ�С
	int	m_iBandNum;
	DIMS();
	DIMS(int iXStart, int iXEnd, int iYStart, int iYEnd, int iBand = -1);
	//���
	DIMS(DIMS& DimsCopy);
	~DIMS();
	DIMS& operator= (const DIMS& DimsCopy);
	void InitDIMS();
	//int iBand:ͼ���ܲ���������ȫ��ѡ��
	void InitDIMS(int iXStart, int iXEnd, int iYStart, int iYEnd, int iBand = 0);
	int getDIMSBands();

	//����DIMS���α�ţ���ȡͼ�񲨶α��
	//��Ϊ���ڲ���ѡ���û����ʱ�Ĳ��α�ſ�����ͼ��ʵ�ʵĲ��α�Ų�һ��
	int getImgBandNum(int iBandNo);
	//֪��ͼ�񲨶α�ţ���ȡ��ʵ��������dims�е�NO��ţ�������0��ʼ,����ֵ��0��ʼ
	int getDIMSBandNum(int iRealBandNo);

};

//����ֻ�Ǳ��������������������ﶨ�����������Դ�ļ�
template <class TT>
class  HHpsCMatrix;

//���濪�ٴ�С�������㷨����������
extern HIPVAR int		g_BlockSize;
//д���濪�ٴ�С
extern HIPVAR int		g_BlockSizeW;
//���濪�ٴ�С�������㷨����������
extern HIPVAR DIMS		g_Zdims;
//���������ο�߳�
extern HIPVAR int		g_SquareBorderSize;
END_NAMESPACE(HAPLIB)