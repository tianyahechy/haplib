// HAPLib.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "HAPLib.h"

USING_NAMESPACE(HAPLIB)

DIMS HAPLIB::g_Zdims(0, 1, 0, 1);

//���³�ʼ��
void SimDIMS::initDIMS()
{
	xStart = -1;
	xEnd = -1;
	yStart = -1;
	yEnd = -1;
	band = -1;
	xWidth = 0;
	yHeight = 0;
	bandSize = 0;
	bandOffset = 0;
	pData = NULL;
}

//���ÿ�߶ȡ����
inline void SimDIMS::setHeightWidth()
{
	yHeight = yEnd - yStart;
	xWidth = xEnd - xStart;
	bandSize = yHeight * xWidth;
	bandOffset = yStart * xWidth + xStart;
}

//�õ���ĸ߶ȺͿ��
inline int SimDIMS::getHeight()
{
	return yEnd - yStart;
}

inline int SimDIMS::getWidth()
{
	return xEnd - xStart;
}

//����ͼ�����кţ���ȡ�����λ��
inline long SimDIMS::getCacheLoc(int column, int line)
{
	return (line - yStart) * xWidth + (column - xStart);
}

//ͨ����dims�ڵĲ��κź����кţ���ȡ������λ�ã����׶�ȡģʽ
inline long SimDIMS::wGetCacheLoc(int iBand, int column, int line)
{
	return iBand * xWidth * yHeight + (line - yStart) * xWidth + (column - xStart);
}

inline long SimDIMS::getCacheSize()
{
	return xWidth * yHeight;
}

inline long SimDIMS::wGetCacheSize()
{
	return band * xWidth * yHeight;
}

//DIMS��������
DIMS::DIMS()
{
	m_pBand = NULL;
	m_iBandNum = -1;
}

DIMS::DIMS(int iXStart, int iXEnd, int iYStart, int iYEnd, int iBand)
{
	xStart = iXStart;
	xEnd = iXEnd;
	yStart = iYStart;
	yEnd = iYEnd;
	setHeightWidth();

	m_pBand = NULL;
	m_iBandNum = -1;
	if ( iBand <= 0 )
	{
		return;
	}
	band = m_iBandNum = iBand;
	if (m_pBand != NULL)
	{
		delete m_pBand;
	}
	m_pBand = new bool[m_iBandNum];
	memset(m_pBand, true, m_iBandNum * sizeof(bool));
}

//���
DIMS::DIMS(DIMS& DimsCopy)
{
	//���ֹ��캯����һ��������ֵ��ʼ���������ֻ���������һ����ע��Ĭ�Ϲ��캯���е���䣬��DIMS dims = g_Zdims
	m_pBand = NULL;
	m_iBandNum = -1;
	*this = DimsCopy;
	if ( DimsCopy.m_pBand != NULL && 
		DimsCopy.m_iBandNum != -1)
	{
		m_iBandNum = DimsCopy.m_iBandNum;
		if ( m_pBand != NULL )
		{
			delete m_pBand;
		}
		m_pBand = new bool[m_iBandNum];
		memcpy(m_pBand, DimsCopy.m_pBand, m_iBandNum * sizeof(bool));
	}

}
DIMS::~DIMS()
{
	//ע��0��1�߼���ϵ
	if ( m_pBand )
	{
		delete[] m_pBand;
		m_pBand = NULL;
	}
}
DIMS& DIMS::operator= (const DIMS& DimsCopy)
{
	xStart = DimsCopy.xStart;
	xEnd = DimsCopy.xEnd;
	yStart = DimsCopy.yStart;
	yEnd = DimsCopy.yEnd;
	m_iBandNum = DimsCopy.m_iBandNum;
	band = DimsCopy.band;

	if (DimsCopy.m_pBand != NULL &&
		m_iBandNum != -1)
	{
		if (m_pBand != NULL)
		{
			delete m_pBand;
		}
		m_pBand = new bool[m_iBandNum];
		memcpy(m_pBand, DimsCopy.m_pBand, m_iBandNum * sizeof(bool));
	}
	setHeightWidth();
	return *this;
}

void DIMS::InitDIMS()
{
	SimDIMS::initDIMS();
	if ( !m_pBand)
	{
		delete[] m_pBand;
		m_pBand = NULL;
	}
	m_iBandNum = MAXBAND;
	m_pBand = new bool[MAXBAND];
}

void DIMS::InitDIMS(int iXStart, int iXEnd, int iYStart, int iYEnd, int iBand)
{
	if ( m_pBand )
	{
		delete[] m_pBand;
		m_pBand = NULL;
	}
	xStart = iXStart;
	xEnd = iXEnd;
	yStart = iYStart;
	yEnd = iYEnd;
	setHeightWidth();
	if ( iBand <= 0 )
	{
		return;
	}

	//m_iBandNum���ͼ����ܲ�����
	band = m_iBandNum = iBand;
	if ( m_pBand != NULL )
	{
		delete m_pBand;
	}
	m_pBand = new bool[m_iBandNum];
	memset(m_pBand, true, m_iBandNum * sizeof(bool));
}

int DIMS::getDIMSBands()
{
	if ( !m_pBand)
	{
		return -1;
	}
	int BandNumber = 0;
	for (size_t i = 0; i < m_iBandNum; i++)
	{
		if ( m_pBand[i])
		{
			BandNumber++;
		}
	}
	band = BandNumber;
	return BandNumber;
}

//����dims���λ��ͼ�񲨶α�ţ���0��ʼ
int DIMS::getImgBandNum(int iBandNo)
{
	if ( iBandNo >= getDIMSBands() || iBandNo < 0 )
	{
		return -1;
	}
	int iCount = -1;
	int ImageBandNumber = 0;
	for (ImageBandNumber = 0; ImageBandNumber < m_iBandNum; ImageBandNumber++)
	{
		if ( m_pBand[ImageBandNumber])
		{
			iCount++;
			if (iCount == iBandNo)
			{
				break;
			}
		}
	}
	return ImageBandNumber;
}

//����ͼ�񲨶α�ţ���ȡ��ʵ��������dims�еı�ţ���0��ʼ������ֵ��0��ʼ
int DIMS::getDIMSBandNum(int iRealBandNo)
{
	int i = 0, j = 0;
	for ( i = 0; i < m_iBandNum; i++)
	{
		if (m_pBand[i])
		{
			if ( i == iRealBandNo )
			{
				//band���dimsѡ�еĲ�����
				band = j;
				return j;
			}
			j++;
		}
	}
	return -1;
}
