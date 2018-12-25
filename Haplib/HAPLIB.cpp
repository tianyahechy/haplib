#include "haplib.h"
USING_NAMESPACE(HAPLIB)

DIMS HAPLIB::g_Zdims(0, 1, 0, 1);
//���³�ʼ��
void SimDIMS::InitDIMS()
{
	xStart = -1;
	xEnd = -1;
	yStart = -1;
	yEnd = -1;
	band = -1;
	width = 0;
	height = 0;
	bandSize = 0;
	bandOffset = 0;
	pData = NULL;
}
//���ÿ�߶ȡ����
inline void SimDIMS::SetHeightWidth()
{
	height = yEnd - yStart;
	width = xEnd - xStart;
	bandSize = height * width;
	bandOffset = yStart * width + xStart;
}

//�õ���߶ȡ����
inline int SimDIMS::GetHeight()
{
	return yEnd - yStart;
}

inline int SimDIMS::GetWidth()
{
	return xEnd - xStart;
}
//����ͼ�����кţ���ȡ�����λ��
inline long SimDIMS::GetCacheLoc(int column, int line)
{
	return (line - yStart) * width + (column - xStart);
}
//ͨ����dims�ڵĲ��κź����кţ���ȡ������λ�ã����׶�ȡģʽ
inline long SimDIMS::WGetCacheLoc(int iBand, int column, int line)
{
	return iBand * width * height + (line - yStart) * width + (column - xStart);
}

inline long SimDIMS::GetCacheSize()
{
	return width * height;
}

inline long SimDIMS::WGetCacheSize()
{
	return band * width * height;
}

DIMS::DIMS()
{
	m_pBand = NULL;
	m_iBandNum = -1;
}

DIMS::DIMS(int iXStart, int iXend, int iYstart, int iYend, int iBand)
{
	xStart = iXStart;
	xEnd = iXend;
	yStart = iYstart;
	yEnd = iYend;
	SetHeightWidth();

	m_pBand = NULL;
	m_iBandNum = -1;
	if (iBand <= 0 )
	{
		return;
	}
	//m_iBandNuM���ͼ����ܲ��Σ�
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
	//���ֹ��캯����һ��������ֵ��ʼ���������ֻ���������һ����
	m_pBand = NULL;
	m_iBandNum = -1;
	*this = DimsCopy;
	if (DimsCopy.m_pBand != NULL && DimsCopy.m_iBandNum != -1)
	{
		m_iBandNum = DimsCopy.m_iBandNum;
		if (m_pBand != NULL)
		{
			delete m_pBand;
		}
		m_pBand = new bool[m_iBandNum];
		memcpy(m_pBand, DimsCopy.m_pBand, m_iBandNum * sizeof(bool));

	}
}
DIMS::~DIMS()
{
	if (m_pBand)
	{
		delete[] m_pBand;
		m_pBand = NULL;
	}
}

//���
DIMS& DIMS::operator=(const DIMS& DimsCopy)
{
	xStart = DimsCopy.xStart;
	xEnd = DimsCopy.xEnd;
	yStart = DimsCopy.yStart;
	yEnd = DimsCopy.yEnd;
	m_iBandNum = DimsCopy.m_iBandNum;
	band = DimsCopy.band;
	if (DimsCopy.m_pBand != NULL && m_iBandNum != -1)
	{
		m_iBandNum = DimsCopy.m_iBandNum;
		if (m_pBand != NULL)
		{
			delete m_pBand;
		}
		m_pBand = new bool[m_iBandNum];
		memcpy(m_pBand, DimsCopy.m_pBand, m_iBandNum * sizeof(bool));

	}
	SetHeightWidth();
	return *this;
}

void DIMS::InitDIMS()
{
	SimDIMS::InitDIMS();
	if (m_pBand != NULL)
	{
		delete[] m_pBand;
		m_pBand = NULL;
	}
	m_iBandNum = MAXBAND;
	m_pBand = new bool[MAXBAND];
}


void DIMS::InitDIMS(int iXstart, int xEnd, int iYstart, int iYend, int iBand)
{
	if (m_pBand != NULL)
	{
		delete[] m_pBand;
		m_pBand = NULL;
	}
	xStart = iXstart;
	xEnd = xEnd;
	yStart = iYstart;
	yEnd = iYend;
	SetHeightWidth();
	if (iBand <= 0)
	{
		return;
	}
	band = m_iBandNum = iBand;
	if (m_pBand != NULL)
	{
		delete m_pBand;
	}

	m_pBand = new bool[MAXBAND];
	memset(m_pBand, true, m_iBandNum * sizeof(bool));
}

int DIMS::getDIMSBands()
{
	if (!m_pBand)
	{
		return -1;
	}
	int bandNumber = 0;
	for (int i = 0; i < m_iBandNum; i++)
	{
		if (m_pBand[i])
		{
			bandNumber++;
		}
	}
	band = bandNumber;
	return bandNumber;
}

//��Ϊ���ڲ���ѡ���û����ʱ�Ĳ��α�ſ�����ͼ��ʵ�ʵĲ��α�Ų�һ��
//֪��DIMS���α�ţ���ȡͼ�񲨶α�š�Ϊ�˷����ϲ��û���̣�����iBandNo��0��ʼ
int DIMS::GetImgBandNum(int iBandNo)
{
	if (iBandNo >= getDIMSBands() || iBandNo < 0)
	{
		return -1;
	}
	int iCount = -1;
	int imgBandNumber;
	for ( imgBandNumber = 0; imgBandNumber < m_iBandNum; imgBandNumber++)
	{
		if (m_pBand[imgBandNumber])
		{
			iCount++;
		}
		if (iCount == iBandNo)
		{
			break;

		}
	}
	//��1��ʼ
	return imgBandNumber;
}

//֪��ͼ�񲨶α�ţ���ȡ���ǲ�����dims�е�no��ţ�������0��ʼ������ֵ��0��ʼ
int DIMS::getDimsBandNo(int iRealBandNo)
{
	int i = 0;
	int j = 0;
	for ( i = 0; i < m_iBandNum; i++)
	{
		if (m_pBand[i])
		{
			if (i == iRealBandNo)
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