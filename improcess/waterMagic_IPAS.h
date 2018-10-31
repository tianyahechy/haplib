
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

	//��׼���������㷨�����Ĵ���---����ͼ���ļ������㡢���ͼ���ļ�
	//��ʼ��m_ImgIO,�㷨�����������������͵��ò�ͬ��process()����
	bool standardProcess(DIMS dims, CGDALFileManager * pmgr1, CGDALFileManager* pmgr2 = NULL);
	bool NDWICutStdProcess(DIMS dims, CGDALFileManager * pmgr1, CGDALFileManager* pmgr2 = NULL);
	//׼������������������м�������ʱ���ã����ڻ�ȡ���ӵ�ֲ��ָ��ֵ
	bool PreparePara();
	template <class TT> bool NDWIProcess();
	//ʵ�ֶ�NDWIͼ����Զ���ֵ���ָ�
	bool NDWICut(GDALFileManager* pmgr2 = NULL);
	//seedXY[2]������ڴ棬���л��ͷ�
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
	//��ȡ�������
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
			//��ʶ����Ԫ�Ƿ��Ѿ�������
			//0:��ʼ��Ŀ��洢λ
			//1:�Ѿ���������Ŀ��
			//2:�Ѿ�������Ŀ��
			//255:��ʼĿ��洢λ
			BYTE** ppBOutImage,
			//��¼��������Ӿ�������
			SimDIMS& stSeedRegion,
			//������ʹ��
			int iSeedNumber = 0,
			BOOL bCheck8 = TRUE
		);
	//����ļ�·��
	char m_OutFileName[512];
	//�м���ʱ�ļ�·��
	char m_TempFileName[512];
	//ȡֵ��Χ�����ֵ���������޷�������
	int m_iMaxValue;
	//������������Լ����ģ�庯��ʹ��
	int m_iDataTypeIn;
	//���ӵ����
	int m_SeedNum;
	//������ӵ�X,Y����
	int * m_SeedCoor[2];
	//ˮ����ȡ����ֵ��ȫ��һ��ֵ
	float m_ImageThold;
	//ˮ����ȡ����ֵ��ȷ������Ҫ��ʱ��ʹ��
	float m_ImageTHoldUp;
	//����ֵƫ����
	float m_fOffset;
	//С������
	int m_smallSpot;
	//�����⣬���ղ���
	int m_iNirBand;
	//͸�䲨��
	int m_iBlueBand;
	//����������
	//Ŀ��ʶ���ֵͼ���ڴ�飬ֻ��ȡֵ0��1
	BYTE* m_pBSeedReg;
	//ͼ���
	int m_iRegSamples;
	//ͼ���
	int m_iRegLine;
	//ͼ��X����ƫ����
	int m_XStart;
	//ͼ��Y����ƫ����
	int m_YStart;

};

BOOL TrackSpotBySeedByV3(
	long lCurPos,
	//��ʶ����Ԫ�Ƿ��Ѿ���������
	//0:��ʼ��Ŀ��洢λ
	//1:�Ѿ���������Ŀ��
	//2:�Ѿ�������Ŀ��
	//255:��ʼĿ��洢λ
	BYTE** ppBOutImage,
	long lFileWidth,
	long lFileHeight,
	//��ǰ���������ֵ�������ӵ��ֵ����
	//fregionvalue����Ϊ0��1��2��3
	BYTE fRegionValue,
	//��¼��������Ӿ�������
	SIMDIMS& stSeedRegion,
	BOOL bCheck8 = TRUE
);

BOOL TrackSpotBySeedByV2(
	long lCurPos,
	//��ʶ����Ԫ�Ƿ��Ѿ���������
	//1:�Ѿ�����������С�ߡ���С������
	//fRegion:
	//255:��ʼĿ��洢λ
	BYTE** ppBOutImage,
	long lFileWidth,
	long lFileHeight,
	//��¼����������������
	std::vector<int>& pixels,
	//���Ϊ��Ե���򣬼���С�ߣ�С���ڱ�Ե��˵������Ŀ���ڲ���С�ߣ������ע��
	bool& bEdgeReg,
	//��ǰ���������ֵ�������ӵ��ֵ����һ��Ϊ0
	BYTE fRegionValue,
	BOOL bCheck8 = FALSE
	);

