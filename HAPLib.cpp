// HAPLib.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "HAPLib.h"

USING_NAMESPACE(HAPLIB)

DIMS HAPLIB::g_Zdims(0, 1, 0, 1);

//重新初始化
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

//设置块高度、宽度
inline void SimDIMS::setHeightWidth()
{
	yHeight = yEnd - yStart;
	xWidth = xEnd - xStart;
	bandSize = yHeight * xWidth;
	bandOffset = yStart * xWidth + xStart;
}

//得到块的高度和宽度
inline int SimDIMS::getHeight()
{
	return yEnd - yStart;
}

inline int SimDIMS::getWidth()
{
	return xEnd - xStart;
}

//依据图像行列号，获取缓存块位置
inline long SimDIMS::getCacheLoc(int column, int line)
{
	return (line - yStart) * xWidth + (column - xStart);
}

//通过在dims内的波段号和行列号，获取缓存块的位置，光谱读取模式
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

//DIMS函数定义
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

//深拷贝
DIMS::DIMS(DIMS& DimsCopy)
{
	//各种构造函数在一部创建赋值初始化的情况下只会调用其中一个，注意默认构造函数中的语句，如DIMS dims = g_Zdims
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
	//注意0，1逻辑关系
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

	//m_iBandNum存放图像的总波段数
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

//根据dims波段获得图像波段编号，从0开始
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

//根据图像波段编号，获取真实波段数在dims中的编号，从0开始，返回值从0开始
int DIMS::getDIMSBandNum(int iRealBandNo)
{
	int i = 0, j = 0;
	for ( i = 0; i < m_iBandNum; i++)
	{
		if (m_pBand[i])
		{
			if ( i == iRealBandNo )
			{
				//band存放dims选中的波段数
				band = j;
				return j;
			}
			j++;
		}
	}
	return -1;
}
