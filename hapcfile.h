#pragma once
#pragma warning( disable: 4996)
#include "HAPLib.h"
#include <string>

BEGIN_NAMESPACE(HAPLIB)
#define	LEN	128

//����ͶӰ���ƺ�ͶӰ��Ϣ�Ķ�Ӧģʽ
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

//�Ȼ�Բ��ͶӰ
#define	Cylinderical_Equal_Area						101
//������
#define	Cassini_Soldner								102
//������IV������ͶӰ���ߵȻ�αԲ��ͶӰ)
#define	Eckert_IV									103			
//������VI������ͶӰ���ߵȻ�αԲ��ͶӰ)
#define	Eckert_VI									104	
#define	Gall_Stereographic							105
#define	Krovak_Oblique_Conic_Conformal				106
//����������Բ׶ͶӰ(1SP)
#define Lambert_Conformal_Conic_1SP					107
//����������Բ׶ͶӰ(����ʱ)
#define Lambert_Conformal_Conic_Belgium				108
//�Ⱦ�Բ׶ͶӰ
#define New_Zealand_Map_Grid						109
//б������ͶӰ
#define Oblique_Stereographic						110
//��ʿб��Բ��ͶӰ
#define Swiss_Oblique_Cylindrical					111
#define Tunesia_Mining_Grid							112
//����ī����ͶӰ������
#define Transverse_Mercator_South_Oriented			113
#define User_Defined_projection						99

typedef enum
{
	INVALID	= 0,
	READ	= 1,
	WRITE	= 2
}OpenType;

//ENVI��ͼͶӰ�а�������Ҫ��Ϣ

struct HAPCLASS	MapInfo
{
	struct HAPCLASS UTMUnit
	{
		//��ʶ�ڼ�����ͶӰ���ţ�-1��ʾû��Ϣ
		short m_nZone;
		//true-������false-����
		bool m_NS;
		UTMUnit()
		{
			m_nZone = -1;
			m_NS = true;
		}
	};
	//ͶӰ����
	char m_cProName[LEN];
	//���Ͻ����꣬ԭ��ͼ������
	double m_dLeftUpper[2];
	//���½����꣬ԭ��ͼ�����꣬���ͨ��ϵ���������
	double m_dRightDown[2];
	//ͶӰ����ƫ��ϵ������gdal����һ��
	double m_dOffset[2];
	//���ش�С��X������ֵ��Y����ֵ
	double m_dPixelSize[2];
	//����ͶӰ����ΪUTMʱʹ��
	UTMUnit m_UTM;
	//��ػ�׼����WGS84,NAD27
	char	m_cDatum[LEN];
	//��λ
	char	m_cUnit[20];
	MapInfo()
	{
		//����Ĭ��ֵ
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
		//Բ��뾶
		double	m_dR;
		//������
		double	m_dA;
		//�������
		double	m_dB;
		//����ƫ����Eccentricity
		double	m_dEcc;
		//�������ͣ���Ҫ��ָLandsat1-7,�ֱ���1-7���ֱ�ʾ
		int		m_nSatNum;
		//���ǹ����
		int		m_nPathNum;
		//End of Path FLAG,1��ʾ�ǣ�0��ʾ��
		int		m_nPathFlag;
		//ͶӰ�ο���γ��
		double	m_dLat0;
		//���������߾���
		double	m_dLon0;
		//���ߵ�һ��γ��
		double	m_dLat1;
		//���ߵڶ��㾭��
		double	m_dLon1;
		//���ߵ�һ��γ��
		double	m_dLat2;
		//���ߵڶ��㾭��
		double	m_dLon2;
		//��λ��
		double	m_dAzimuth;
		//α��ƫ
		double	m_dX0;
		//α��ƫ
		double	m_dY0;
		//����ϵ��
		double	m_dK0;
		//��һ��׼γȦγ�ȣ���׼γȦγ�ȣ�α��׼γȦγ�ȣ���λ����㾭��)
		double	m_dSp1;
		//�ڶ���׼γȦγ��
		double	m_dSp2;
		//͸�ӵ�߶�
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
	
	//ͶӰ����
	int				m_nProType;
	//ͶӰ����
	ProjectParam	m_ProjectParam;
	//��ػ�׼
	char			m_cDatum[LEN];
	//����
	char			m_cProName[LEN];
	//��λ
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

//ͷ�ļ��ṹ
struct HAPCLASS HAPCFileHeader
{
	HAPCFileHeader();
	~HAPCFileHeader();
	inline int getBytesPerPt();
	void putInfoByDIMS(DIMS dims, HAPCFileHeader & header);
	HAPCFileHeader& operator = (const HAPCFileHeader &header);

	//�ļ�����
	char m_cDescription[512];
	//���(head4)
	int	m_nSamples;
	//����(head5)
	int	m_nLines;
	//������(head6)
	int	m_nBands;
	//�ļ�ͷƫ��(head9)
	int m_nHeaderOffset;
	//�ļ�����
	char m_cFileType[32];
	//��������(head7)
	HAPDataType m_nDataType;
	//��������
	char m_cInterLeave[64];
	//��������
	char m_cSensorType[64];
	//(head8)
	int m_nByteOrder;
	//X����ʼֵ
	double m_dXStart;
	//Y����ʼֵ
	double m_dYStart;
	//Ĭ�ϲ���
	int m_nDefaultBands[3];
	//����Χ
	double m_dPlotRange[2];
	//����ƽ��ֵ
	double m_fPlotAverage;
	//�������
	char *m_cPlotTitle[2];
	//����
	int m_nStretch;
	//����ģʽ
	char m_sStretchType[64];
	//����������
	char * m_cBandNames[MAXBAND];
	//�����β���
	double m_dWaveLength[MAXBAND];
	//���װ�߿�������Ӧ��������Ӧ�ﵽ%��ʱ��Ĳ��׿�ȣ�
	double m_dFwhm[MAXBAND];
	double m_dxOffset;
	double m_dyOffset;
	char ** m_cSpectraName;
	//��ͼ��Ϣ
	MapInfo * m_MapInfo;
	//ͶӰ��Ϣ
	ProjectInfo * m_ProjectInfo;
	char m_sProvince[LEN];
	char m_sCity[LEN];
	char m_sTime[LEN];
	//ͼ�񼶱�
	double m_fRank;
	//��Ҫ�汾��Ϣ(head2)
	short m_nVersionMajor;
	//��Ҫ�汾��Ϣ(head3)
	short m_nVersionMinor;
	//ϵͳ����ID��head1)
	unsigned long m_dwID;

};

END_NAMESPACE(HAPLIB)