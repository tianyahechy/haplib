#pragma once
#pragma warning( disable: 4996)
#include "HAPLib.h"
#include <string>

BEGIN_NAMESPACE(HAPLIB)
#define	LEN	128

//定义投影名称和投影信息的对应模式
#define Arbitrary									0
#define	Geographic									1
#define	UTM											2
#define	Transverse_Mercator							3
#define	Lambert_Conformal_Conic						4
#define Hotin_Oblique_Mercator_A					5
#define	Hotin_Oblique_Mercator						6
#define Stereographic_Ellipsoid						7
#define	State_Plane									8
#define	Albers_Conical_Equal_Area					9
#define	Polyconic									10
#define	Lambert_Azimuthal_Equal_Area				11
#define	Azimuthal_Equidistant						12
#define	Gnomonic									13
#define	Orthographic								14
#define	General_Vertical_Nearside_Perspective		15
#define	Sinusoidal									16
#define	Equirectangular								17
#define	Miller_Cylindrical							18
#define	Van_der_Griten								19
#define	Mercator									20
#define	Robinson									21
#define	Space_Oblique_Mercator_A					22
#define Alaska_Conformal							23
#define	Interrupted_Goode							24
#define Mollweide									25
#define	Interrupted_Mollweide						26
#define	Hammer										27
#define	Wagner_IV									28
#define	Wagner_VII									29
#define	Oblated_Equal_Area							30
#define Polar_Stereographic							31
#define	Space_Oblique_Mercator_B					32
#define Equidistant_Conic_A							33
#define Equidistant_Conic_B							34
#define Stereographic_Sphere						35
#define Lambert_Azimuthal_Equal_Area_Sphere			36

//等积圆柱投影
#define	Cylinderical_Equal_Area						101
//卡西尼
#define	Cassini_Soldner								102
//爱凯特IV（极点投影成线等积伪圆柱投影)
#define	Eckert_IV									103			
//爱凯特VI（极点投影成线等积伪圆柱投影)
#define	Eckert_VI									104	
#define	Gall_Stereographic							105
#define	Krovak_Oblique_Conic_Conformal				106
//兰伯特正形圆锥投影(1SP)
#define Lambert_Conformal_Conic_1SP					107
//兰伯特正形圆锥投影(比利时)
#define Lambert_Conformal_Conic_Belgium				108
//等距圆锥投影
#define New_Zealand_Map_Grid						109
//斜轴立体投影
#define Oblique_Stereographic						110
//瑞士斜轴圆柱投影
#define Swiss_Oblique_Cylindrical					111
#define Tunesia_Mining_Grid							112
//横轴墨卡托投影（南向）
#define Transverse_Mercator_South_Oriented			113
#define User_Defined_projection						99

typedef enum
{
	INVALID	= 0,
	READ	= 1,
	WRITE	= 2
}OpenType;

//ENVI地图投影中包含的主要信息

struct HAPCLASS	MapInfo
{
	struct HAPCLASS UTMUnit
	{
		//标识第几区，投影带号，-1表示没信息
		short m_nZone;
		//true-北区，false-南区
		bool m_NS;
		UTMUnit()
		{
			m_nZone = -1;
			m_NS = true;
		}
	};
	//投影名称
	char m_cProName[LEN];
	//左上角坐标，原点图像坐标
	double m_dLeftUpper[2];
	//右下角坐标，原点图像坐标，最好通过系数计算出来
	double m_dRightDown[2];
	//投影坐标偏移系数，与gdal定义一致
	double m_dOffset[2];
	//像素大小，X方向正值，Y方向负值
	double m_dPixelSize[2];
	//仅在投影名称为UTM时使用
	UTMUnit m_UTM;
	//大地基准，如WGS84,NAD27
	char	m_cDatum[LEN];
	//单位
	char	m_cUnit[20];
	MapInfo()
	{
		//设置默认值
		strcpy(m_cProName, "Arbitrary");
		strcpy(m_cDatum, "Unknown");
		strcpy(m_cUnit, "Meters");
		for (size_t i = 0; i < 2; i++)
		{
			m_dLeftUpper[i] = 0;
			m_dRightDown[i] = 100;
			m_dOffset[i] = 0;
			m_dPixelSize[i] = 1;

		}

	}

	~MapInfo()
	{
	}

	MapInfo& operator = (const MapInfo &mpinfo)
	{
		if (strcmp(mpinfo.m_cProName, "") == 0 )
		{
			return *this;
		}
		strcpy(m_cProName, mpinfo.m_cProName);
		m_dLeftUpper[0] = mpinfo.m_dLeftUpper[0];
		m_dLeftUpper[1] = mpinfo.m_dLeftUpper[1];
		m_dOffset[0] = mpinfo.m_dOffset[0];
		m_dOffset[1] = mpinfo.m_dOffset[1];
		m_dPixelSize[0] = mpinfo.m_dPixelSize[0];
		m_dPixelSize[1] = mpinfo.m_dPixelSize[1];
		m_dRightDown[0] = mpinfo.m_dRightDown[0];
		m_dRightDown[1] = mpinfo.m_dRightDown[1];
		
		strcpy(m_cUnit, mpinfo.m_cUnit);
		m_UTM.m_nZone = mpinfo.m_UTM.m_nZone;
		m_UTM.m_NS = mpinfo.m_UTM.m_NS;
		strcpy(m_cDatum, mpinfo.m_cDatum);
		return *this;
	}


};

struct HAPCLASS ProjectInfo
{
	struct HAPCLASS ProjectParam
	{
		//圆球半径
		double	m_dR;
		//椭球长轴
		double	m_dA;
		//椭球短轴
		double	m_dB;
		//椭球偏心率Eccentricity
		double	m_dEcc;
		//卫星类型，主要是指Landsat1-7,分别用1-7数字表示
		int		m_nSatNum;
		//卫星轨道号
		int		m_nPathNum;
		//End of Path FLAG,1表示是，0表示否
		int		m_nPathFlag;
		//投影参考点纬度
		double	m_dLat0;
		//中央子午线经度
		double	m_dLon0;
		//中线第一点纬度
		double	m_dLat1;
		//中线第二点经度
		double	m_dLon1;
		//中线第一点纬度
		double	m_dLat2;
		//中线第二点经度
		double	m_dLon2;
		//方位角
		double	m_dAzimuth;
		//伪东偏
		double	m_dX0;
		//伪北偏
		double	m_dY0;
		//比例系数
		double	m_dK0;
		//第一标准纬圈纬度（标准纬圈纬度，伪标准纬圈纬度，方位角轴点经度)
		double	m_dSp1;
		//第二标准纬圈纬度
		double	m_dSp2;
		//透视点高度
		double	m_dHeight;

		ProjectParam()
		{
			m_dR			= -99999;
			m_dA			= -99999;
			m_dB			= -99999;
			m_dEcc			= -99999;
			m_nSatNum		= -99999;
			m_nPathNum		= -99999;
			m_nPathFlag		= -99999;
			m_dLat0			= -99999;
			m_dLat1			= -99999;
			m_dLon1			= -99999;
			m_dLat2			= -99999;
			m_dLon2			= -99999;
			m_dAzimuth		= -99999;
			m_dX0			= -99999;
			m_dY0			= -99999;
			m_dK0			= -99999;
			m_dSp1			= -99999;
			m_dSp2			= -99999;
			m_dHeight		= -99999;

		}




	};
	
	//投影代号
	int				m_nProType;
	//投影参数
	ProjectParam	m_ProjectParam;
	//大地基准
	char			m_cDatum[LEN];
	//名称
	char			m_cProName[LEN];
	//单位
	char			m_cUnit[20];

	ProjectInfo()
	{
		m_nProType = -1;
		strcpy(m_cProName, "");
		strcpy(m_cDatum, "");
		strcpy(m_cUnit, "");
	}
	ProjectInfo& operator = (const ProjectInfo& prjinfo)
	{
		if ( strcmp(prjinfo.m_cProName,"") == 0 )
		{
			return *this;
		}
		m_nProType = prjinfo.m_nProType;
		strcpy(m_cProName, prjinfo.m_cProName);
		strcpy(m_cDatum, prjinfo.m_cDatum);
		strcpy(m_cUnit, prjinfo.m_cUnit);
		m_ProjectParam = prjinfo.m_ProjectParam;

		return *this;
	}
};

//头文件结构
struct HAPCLASS HAPCFileHeader
{
	HAPCFileHeader();
	~HAPCFileHeader();
	inline int getBytesPerPt();
	void putInfoByDIMS(DIMS dims, HAPCFileHeader & header);
	HAPCFileHeader& operator = (const HAPCFileHeader &header);

	//文件描述
	char m_cDescription[512];
	//宽度(head4)
	int	m_nSamples;
	//长度(head5)
	int	m_nLines;
	//波段数(head6)
	int	m_nBands;
	//文件头偏移(head9)
	int m_nHeaderOffset;
	//文件类型
	char m_cFileType[32];
	//数据类型(head7)
	HAPDataType m_nDataType;
	//插入类型
	char m_cInterLeave[64];
	//传感类型
	char m_cSensorType[64];
	//(head8)
	int m_nByteOrder;
	//X轴起始值
	double m_dXStart;
	//Y轴起始值
	double m_dYStart;
	//默认波段
	int m_nDefaultBands[3];
	//区域范围
	double m_dPlotRange[2];
	//区域平均值
	double m_fPlotAverage;
	//区域标题
	char *m_cPlotTitle[2];
	//拉伸
	int m_nStretch;
	//拉伸模式
	char m_sStretchType[64];
	//各波段名称
	char * m_cBandNames[MAXBAND];
	//各波段波长
	double m_dWaveLength[MAXBAND];
	//波谱半高宽（光谱响应函数中相应达到%的时候的波谱宽度）
	double m_dFwhm[MAXBAND];
	double m_dxOffset;
	double m_dyOffset;
	char ** m_cSpectraName;
	//地图信息
	MapInfo * m_MapInfo;
	//投影信息
	ProjectInfo * m_ProjectInfo;
	char m_sProvince[LEN];
	char m_sCity[LEN];
	char m_sTime[LEN];
	//图像级别
	double m_fRank;
	//主要版本信息(head2)
	short m_nVersionMajor;
	//次要版本信息(head3)
	short m_nVersionMinor;
	//系统类型ID（head1)
	unsigned long m_dwID;

};

END_NAMESPACE(HAPLIB)