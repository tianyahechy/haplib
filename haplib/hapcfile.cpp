#include "hapcfile.h"

USING_NAMESPACE(HAPLIB)

inline int HAPCFileHeader::getBytesPerPt()
{
	switch (m_nDataType)
	{
	case HAP_Byte:
		return 1;
	case HAP_UInt16:
		return 2;
	case HAP_Int16:
		return 2;
	case HAP_UInt32:
		return 4;
	case HAP_Int32:
		return 4;
	case HAP_Float32:
		return 4;
	case HAP_Float64:
		return 8;			
	default:
		return -1;
	}
}

HAPCFileHeader::HAPCFileHeader()
{
	//文件描述
	memset( m_cDescription,0,512);
	//宽度(head4)
	m_nSamples = -1;
	//长度(head5)
	m_nLines = -1;
	//波段数(head6)
	m_nBands = -1;
	//文件类型
	memset( m_cFileType, 0,32);
	//数据类型(head7)
	m_nDataType = HAP_Byte;
	//插入类型
	memset(m_cInterLeave, 0, 8);
	//传感类型
	memset(m_cSensorType, 0, 32);
	memset(m_sStretchType, 0, 16);

	//(head8)
	m_nByteOrder = -1;
	//X轴起始值
	m_dXStart = 0;
	//Y轴起始值
	m_dYStart = 0;
	//区域平均值
	m_fPlotAverage = -1;
	//拉伸
	m_nStretch = -1;
	m_dxOffset = -1;
	m_dyOffset = -1;
	m_cSpectraName = NULL;
	//主要版本信息(head2)
	m_nVersionMajor = -1;
	//次要版本信息(head3)
	m_nVersionMinor = -1;
	//系统类型ID（head1)
	m_dwID = 16435;
	m_nHeaderOffset = -1;
	//默认波段
	for (size_t i = 0; i < 3; i++)
	{
		m_nDefaultBands[i] = -1;
	}
	//区域范围
	for (size_t i = 0; i < 2; i++)
	{
		m_dPlotRange[i] = -1;
	}
	//区域标题
	for (size_t i = 0; i < 2; i++)
	{
		m_cPlotTitle[i] = new char[32];
		memset(m_cPlotTitle[i], 0, 32);
	}
	//各波段名称
	//各波段波长
	for (size_t i = 0; i < MAXBAND; i++)
	{
		m_cBandNames[i] = new char[NAMELEN];
		memset(m_cBandNames[i], 0, NAMELEN);
		m_dWaveLength[i] = m_dFwhm[i] = -1;
	}
	for (int i = 0; i < 2; i++)
	{
		m_cPlotTitle[i] = new char[32];
		memset(m_cPlotTitle[i], 0, 32);
	}
	//地图信息
	m_MapInfo = NULL;
	//投影信息
	m_ProjectInfo = NULL;
}

HAPCFileHeader& HAPCFileHeader::operator= (const HAPCFileHeader & header)
{
	if ( m_cSpectraName )
	{
		for (int i = 0; i < m_nLines; i++)
		{
			delete[] m_cSpectraName[i];
		}
		delete[] m_cSpectraName;
		m_cSpectraName = NULL;
	}
	strcpy(m_cDescription, header.m_cDescription);
	m_nSamples = header.m_nSamples;
	m_nLines = header.m_nLines;
	m_nBands = header.m_nBands;
	m_nHeaderOffset = header.m_nHeaderOffset;
	strcpy(m_cFileType, header.m_cFileType);
	m_nDataType = header.m_nDataType;
	strcpy(m_cInterLeave, header.m_cInterLeave);
	strcpy(m_cSensorType, header.m_cSensorType);
	strcpy(m_sStretchType, header.m_sStretchType);
	m_nByteOrder = header.m_nByteOrder;
	m_dXStart = header.m_dXStart;
	m_dYStart = header.m_dYStart;
	m_dxOffset = header.m_dxOffset;
	m_dyOffset = header.m_dyOffset;
	if (header.m_cSpectraName)
	{
		m_cSpectraName = new char*[m_nLines];
		for (size_t i = 0; i < m_nLines; i++)
		{
			m_cSpectraName[i] = new char[NAMELEN];
			strcpy(m_cSpectraName[i], header.m_cSpectraName[i]);
		}
	}
	for (size_t i = 0; i < 3; i++)
	{
		m_nDefaultBands[i] = header.m_nDefaultBands[i];
	}
	for (size_t i = 0; i < 2; i++)
	{
		m_dPlotRange[i] = header.m_dPlotRange[i];
	}
	m_fPlotAverage = header.m_fPlotAverage;
	for (size_t i = 0; i < 2; i++)
	{
		strcpy(m_cPlotTitle[i], header.m_cPlotTitle[i]);
	}
	m_nStretch = header.m_nStretch;
	for (size_t i = 0; i < MAXBAND; i++)
	{
		strcpy(m_cBandNames[i], header.m_cBandNames[i]);
	}
	for (size_t i = 0; i < MAXBAND; i++)
	{
		m_dWaveLength[i] = header.m_dWaveLength[i];
	}
	for (size_t i = 0; i < 64; i++)
	{
		m_dFwhm[i] = header.m_dFwhm[i];
	}
	if ( header.m_nSamples)
	{
		m_MapInfo = new MapInfo();
		*m_MapInfo = *(header.m_MapInfo);
	}
	if (header.m_ProjectInfo)
	{
		m_ProjectInfo = new ProjectInfo;
		*m_ProjectInfo = *(header.m_ProjectInfo);
	}
	m_nVersionMajor = header.m_nVersionMajor;
	m_nVersionMinor = header.m_nVersionMinor;
	m_dwID = header.m_dwID;
	return *this;
}

HAPCFileHeader::~HAPCFileHeader()
{
	for (size_t i = 0; i < MAXBAND; i++)
	{
		delete[] m_cBandNames[i];
		m_cBandNames[i] = NULL;
	}
	for (size_t i = 0; i < 2; i++)
	{
		delete[] m_cPlotTitle[i];
		m_cPlotTitle[i] = NULL;
	}
	if ( m_cSpectraName )
	{
		for (size_t i = 0; i < m_nLines; i++)
		{
			delete[] m_cSpectraName[i];
		}
		delete[] m_cSpectraName;
		m_cSpectraName = NULL;
	}
	if (m_MapInfo)
	{
		delete m_MapInfo;
	}
	if (m_ProjectInfo)
	{
		delete m_ProjectInfo;
	}
}

//通过DIMS获取图像信息
void HAPCFileHeader::putInfoByDIMS(DIMS dims, HAPCFileHeader &header)
{
	strcpy(header.m_cDescription, m_cDescription);
	header.m_nSamples = dims.xEnd - dims.xStart;
	header.m_nLines = dims.yEnd - dims.yStart;
	header.m_nBands = dims.getDIMSBands();
	header.m_nHeaderOffset = m_nHeaderOffset;
	strcpy(header.m_cFileType, m_cFileType);
	header.m_nDataType = m_nDataType;
	strcpy(header.m_cInterLeave, m_cInterLeave);
	strcpy(header.m_cSensorType, m_cSensorType);
	strcpy(header.m_sStretchType, m_sStretchType);
	header.m_nByteOrder = m_nByteOrder;
	header.m_dXStart = m_dXStart;
	header.m_dYStart = m_dYStart;
	header.m_dxOffset = m_dxOffset;
	header.m_dyOffset = m_dyOffset;
	for (size_t i = 0; i < 2; i++)
	{
		header.m_dPlotRange[i] = m_dPlotRange[i];
	}
	header.m_fPlotAverage = m_fPlotAverage;
	for (size_t i = 0; i < 2; i++)
	{
		strcpy(header.m_cPlotTitle[i], m_cPlotTitle[i]);
	}
	header.m_nStretch = m_nStretch;
	int index = 0;
	for (size_t i = 0; i < MAXBAND; i++)
	{
		if (dims.m_pBand[i] != 1)
		{
			continue;
		}
		strcpy(header.m_cBandNames[index], m_cBandNames[i]);
		header.m_dWaveLength[index] = m_dWaveLength[i];
		header.m_dFwhm[index] = m_dFwhm[i];
		index++;
	}
	if ( m_MapInfo)
	{
		if (!header.m_MapInfo)
		{
			header.m_MapInfo = new MapInfo;
		}
		*header.m_MapInfo = *m_MapInfo;
	}
	if ( header.m_ProjectInfo)
	{
		if (!header.m_ProjectInfo)
		{
			header.m_ProjectInfo = new ProjectInfo;
		}
		*header.m_ProjectInfo = *m_ProjectInfo;
	}
	header.m_nVersionMajor = m_nVersionMajor;
	header.m_nVersionMinor = m_nVersionMinor;
	header.m_dwID = m_dwID;
}