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

//ͳ��ͼ����Ϣ�ṹ�壬��Ű���ң��ͼ������ֵ����Сֵ����ֵ����׼�ֱ��ͼ����Ϣ�������ͼ��ķ���ͳ�ƣ�����һ���Լ�¼
struct HAPCLASS StImgSta
{
	float fMax;
	float fMin;
};

//��������Ϣ�ṹ,simpledims���еĳ�Ա����ֱ��ӳ�䵽ͼ����ָ��λ��
//��;�����ڵײ㻺����Ϣ���
//ע��SimDIMS��Ҫ���϶�Ӧ��ͼ��DIMS���ܹ�ӳ�䵽ͼ���е���Ϣ�ṹ
struct HAPCLASS SimDIMS
{
	//x��ʼ�㣬����ȡ��
	long xStart;
	//x��ֹ�㣬���ɴﵽ
	long xEnd;
	//y��ʼ�㣬����ȡ��
	long yStart;
	//y��ֹ�㣬���ɴﵽ
	long yEnd;
	//y����߶�
	long height;
	//x������
	long width;
	//BSQ->�ж�ȡģʽ����ǰ����λ�ã���0��ʼ����
	//���׶�ȡģʽ��ѡ�еĲ�������
	//-1��ʾSimdimsû�и�ֵ
	int band;
	//ÿ�����δ�С
	int bandSize;
	//����ƫ����
	int bandOffset;
	//ָ��ָ����Χ�����ݿ飬SimDIMS�����������ڴ�
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
	//���³�ʼ��
	void InitDIMS();
	//����
	inline void SetHeightWidth();
	inline int GetHeight();
	inline int GetWidth();
	//����ͼ�����кţ���ȡ�����λ��
	inline long GetCacheLoc(int column, int line);
	//ͨ����dims�ڵĲ��κź����кţ���ȡ������λ��
	inline long WGetCacheLoc(int iBand, int column, int line);
	inline long GetCacheSize();
	inline long WGetCacheSize();
};

//�����û�����ѡ�񣬷�Χѡ����ʵ��
struct HAPCLASS DIMS: SimDIMS
{
	//����ָ�룬�����û�ѡ��Ĳ���������1��ʾ�ò���ѡ�У� ��0��ʾûѡ��
	bool * m_pBand;
	//ͼ���ܲ�������Ҳ��m_pBand����Ĵ�С
	int m_iBandNum;
	DIMS();
	//int iBand:ͼ�����ĸ��ܲ���������ȫ��ѡ��
	DIMS(int iXStart, int iXend, int iYstart, int iYend, int iBand = -1);
	//���
	DIMS(DIMS& DimsCopy);
	~DIMS();
	DIMS& operator= (const DIMS& DimsCopy);
	void InitDIMS();
	//int iBand:ͼ���ܲ���������ȫ��ѡ��
	void InitDIMS(int iXStart, int iXend, int iYstart, int iYend, int iBand = 1);
	int getDIMSBands();
	//��Ϊ���ڲ���ѡ���û����ʱ�Ĳ��α�ſ�����ͼ��ʵ�ʵĲ��α�Ų�һ�¡�
	//֪��DIMS���α�ţ���ȡͼ�񲨶α�ţ���ȡͼ�񲨶α�ţ�Ϊ�˷����ϲ��û���̣�����ibandno��0��ʼ
	int GetImgBandNum(int iBandNo);
	//֪��ͼ�񲨶α�ţ���ȡ��ʵ��������dims�е�no��ţ�������0��ʼ������ֵ��0��ʼ
	int getDimsBandNo(int iRealBandNo);

};

//����ֻ�Ǳ��������������������ﶨ�����������������Դ�ļ�
template <class T>
class HHpsCMatrix;
//���濪�ٴ�С�������㷨����������
extern HIPVAR int	g_BlockSize;
//д���濪�ٴ�С
extern HIPVAR int	g_BlockSizeW;
//���濪�ٴ�С�������㷨����������
extern HIPVAR DIMS	g_Zdims;
//square���������ο�߳�
extern HIPVAR int	g_SquareBorderSize;
END_NAMESPACE(HAPLIB)