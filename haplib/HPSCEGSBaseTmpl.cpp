#include "hpscegsbaseTmpl.h"
#include <cmath>
#include "MyTimer.h"
#include <process.h>

//ȫ�ּ�ʱ����
extern CMyTimer g_CMT;

HAPBEGBase::HAPBEGBase()
{
	m_bBlockOut = NULL;
	m_bBlockOut2 = NULL;

	m_bBlockIn = NULL;
	m_bBlockIn2 = NULL;

	//Ĭ��Ϊ�������ģʽ
	m_OutInMem = false;
	m_MgrIn = NULL;
	m_MgrOut = NULL;

	m_nOffsetX = 0;
	m_nOffsetY = 0;
	m_iRejectCacheFlag = -1;

	m_nPtLength = 1;
	m_nPtLengthOut = -1;
	m_writeFlag = 0;
	m_writePos = 0;
}

HAPBEGBase::~HAPBEGBase()
{
	if (m_bBlockIn != NULL)
	{
		delete[] m_bBlockIn;
		m_bBlockIn = NULL;
	}

	if (m_bBlockIn2 != NULL)
	{
		delete[] m_bBlockIn2;
		m_bBlockIn2 = NULL;
	}

	if (m_bBlockOut != NULL)
	{
		delete[] m_bBlockOut;
		m_bBlockOut = NULL;
	}

	if (m_bBlockOut2 != NULL)
	{
		delete[] m_bBlockOut2;
		m_bBlockOut2 = NULL;
	}
}

bool HAPBEGBase::Set_MgrIn(CGDALFileManager * mgr, DIMS dims)
{
	if (m_MgrIn != NULL)
	{
		delete m_MgrIn;
	}
	if (
		mgr->m_header.m_nLines > dims.yEnd ||
		mgr->m_header.m_nSamples > dims.xEnd ||
		dims.band > mgr->m_header.m_nBands 
		)
	{
		strcpy(pcErrMessage, "DIMS����Խ��!");
		return false;
	}
	m_MgrIn = mgr;
	m_DimsIn = dims;
	m_nOffsetX = dims.xStart;
	m_nOffsetY = dims.yStart;

	//��Ϣ��ʼ��
	if (m_bBlockIn != NULL)
	{
		delete[] m_bBlockIn;
		m_bBlockIn = NULL;
	}
	if (m_bBlockIn2 != NULL)
	{
		delete[] m_bBlockIn2;
		m_bBlockIn2 = NULL;
	}

	m_nPtLength = m_MgrIn->m_header.getBytesPerPt();
	return true;
}

bool HAPBEGBase::Set_MgrOut(CGDALFileManager * mgr, const char* fileName)
{
	if (m_MgrOut != NULL)
	{
		delete m_MgrOut;
	}
	m_MgrOut = mgr;
	if (m_bBlockOut != NULL)
	{
		delete[] m_bBlockOut;
		m_bBlockOut = NULL;
	}
	m_nPtLengthOut = m_MgrOut->m_header.getBytesPerPt();
	//����ļ����ڴ�
	if (m_MgrOut->BoolIsImgInMem())
	{
		m_OutBufferSize = m_MgrOut->m_header.m_nSamples *
			m_MgrOut->m_header.m_nLines *
			m_MgrOut->m_header.m_nBands;
		//��Ϊ�ֽ�ģʽ�����������ռ�ֽ���
		m_bBlockOut = new BYTE[m_OutBufferSize * m_nPtLengthOut];
		m_OutInMem = NULL;
	}
	else
	{
		if (fileName == NULL )
		{
			return false;
		}
		if (!m_MgrOut->HpsSaveImageAs(fileName) )
		{
			return false;
		}
		m_OutInMem = NULL;
	}
	m_WriteDoneCount = 0;
	return true;
}

void HAPBEGBase::Close_MgrIn()
{
	if (m_bBlockIn != NULL)
	{
		delete[] m_bBlockIn;
		m_bBlockIn = NULL;
	}
	if (m_bBlockIn2 != NULL)
	{
		delete[] m_bBlockIn2;
		m_bBlockIn2 = NULL;
	}
	if (m_MgrIn != NULL)
	{
		m_MgrIn->Close();
		m_MgrIn = NULL;
	}
	m_SBlockInInf.initDIMS();
	m_SBlockInInf2.initDIMS();
}

void HAPBEGBase::Close_MgrOut()
{
	if (m_MgrOut == NULL)
	{
		return;
	}
	if (m_bBlockOut != NULL )
	{
		//�Ż��ڴ濪�٣����������������
		if (m_WriteDoneCount == 0 )
		{
			delete[] m_bBlockOut;
		}
		else
		{
	
			if (m_writeFlag == 0 )
			{
				m_MgrOut->WriteBlock(m_bBlockOut, m_WriteDoneCount * m_nPtLength);
			}
			else
			{
				m_MgrOut->WWriteBlock2(m_bBlockOut, m_OutBufferSize * m_nPtLengthOut, m_WriteDoneCount * m_nPtLengthOut);

			}
			delete[] m_bBlockOut;
			//������ʱ
			g_CMT.pauseCountTime();
		}
		m_bBlockOut = NULL;
		m_WriteDoneCount = 0;
	}
	m_MgrOut->Close();
	m_MgrOut = NULL;
}

BYTE * HAPBEGBase::RequestCache(SimDIMS * &pSBlockInf)
{
	//����δ����
	if ( !m_bBlockIn )
	{
		m_bBlockIn = new BYTE[g_BlockSize];
		pSBlockInf = &m_SBlockInInf;
		m_iRejectCacheFlag = 2;
		return m_bBlockIn;
	}
	if (!m_bBlockIn2)
	{
		m_bBlockIn2 = new BYTE[g_BlockSize];
		pSBlockInf = &m_SBlockInInf2;
		m_iRejectCacheFlag = 1;
		return m_bBlockIn2;
	}
	//�����ѷ���
	if (m_iRejectCacheFlag == 1)
	{
		//��ʾ2�Ż��������ݼ�������
		m_iRejectCacheFlag = 2;
		pSBlockInf = &m_SBlockInInf;
		return m_bBlockIn;
	}

	if (m_iRejectCacheFlag == 2)
	{
		//��ʾ1�Ż��������ݼ�������
		m_iRejectCacheFlag = 1;
		pSBlockInf = &m_SBlockInInf2;
		return m_bBlockIn2;
	}
	return m_bBlockIn;

}

void HAPBEGBase::RGetBlockInfo(int col, int row, int band, SimDIMS * pSBlockInfo)
{
	if (m_DimsIn.xEnd == -1 || m_DimsIn.yEnd == -1 || pSBlockInfo == NULL )
	{
		return;
	}
	int iRow = m_DimsIn.getHeight();
	int iCol = m_DimsIn.getWidth();
	int iDT = m_nPtLength;
	
	pSBlockInfo->xStart = 0;
	pSBlockInfo->xEnd = iCol;

	int iLinesPerBlock = g_BlockSize / (iDT * iCol);
	int iNumOfBlockPerBand = int(iRow * 1.0 / iLinesPerBlock + 0.9999);
	if (iLinesPerBlock >= iRow )
	{
		pSBlockInfo->yStart = 0;
		pSBlockInfo->yEnd = iRow;
	}
	else
	{
		//row�����ڵĿ�ţ�0��ʼ
		int NOBlock = row / iLinesPerBlock;
		pSBlockInfo->yStart = NOBlock * iLinesPerBlock;
		//�������һ�����������СһЩ
		if (NOBlock == iNumOfBlockPerBand - 1)
		{
			pSBlockInfo->yEnd = iRow;
		}
		else
		{
			pSBlockInfo->yEnd = pSBlockInfo->yStart + iLinesPerBlock;
		}
		pSBlockInfo->band = band;
		//���ÿ�߶ȿ��
		pSBlockInfo->setHeightWidth();
	}
	return;
}

void HAPBEGBase::BGetBlockInfo(int col, int row, int band, SimDIMS * pSBlockInfo)
{
	if (m_DimsIn.xEnd == -1 || m_DimsIn.yEnd == -1 || pSBlockInfo == NULL)
	{
		return;
	}
	int iRow = m_DimsIn.getHeight();
	int iCol = m_DimsIn.getWidth();
	int iDT = m_nPtLength;

	//��Ĵ�СΪƽ��
	int iBlockWidth = sqrt((double)g_BlockSize);
	int iBlockHeight = iBlockWidth;
	if (iBlockWidth > iCol )
	{
		iBlockWidth = iCol;
	}
	if (iBlockHeight > iRow )
	{
		iBlockHeight = iRow;
	}
	
	//��ȡ�к��б��ֵķ���
	int iNumOfWidth = ceil((double)iCol / (double)iBlockWidth);
	int iNumOfHeight = ceil((double)iRow / (double)iBlockHeight);
	//�ܿ���
	int iNumOfBlock = iNumOfWidth * iNumOfHeight;

	//��ȡ�к��е�ָ��
	int indexOfWidth = col / iBlockWidth;
	int indexOfHeight = row / iBlockHeight;

	//��ȡ�����ʼ��ֵ
	pSBlockInfo->xStart = m_DimsIn.xStart + iBlockWidth * indexOfWidth;
	pSBlockInfo->yStart = m_DimsIn.yStart + iBlockHeight * indexOfHeight;

	if (indexOfWidth + 1 == iNumOfWidth)
	{
		pSBlockInfo->xEnd = m_DimsIn.xEnd;
	}
	else
	{
		pSBlockInfo->xEnd = pSBlockInfo->xStart + iBlockWidth;
	}

	if (indexOfHeight + 1 == iNumOfHeight )
	{
		pSBlockInfo->yEnd = m_DimsIn.yEnd;
	}
	else
	{
		pSBlockInfo->yEnd = pSBlockInfo->yStart + iBlockHeight;
	}
	
	pSBlockInfo->band = band;
	//���ÿ�߶ȿ��
	pSBlockInfo->setHeightWidth();
	
	return;
}

void HAPBEGBase::CGetBlockInfo(int col, int row, int band, SimDIMS * pSBlockInfo)
{
	if (m_DimsIn.xEnd == -1 || m_DimsIn.yEnd == -1 || pSBlockInfo == NULL)
	{
		return;
	}
	int iRow = m_DimsIn.getHeight();
	int iCol = m_DimsIn.getWidth();
	int iDT = m_nPtLength;

	pSBlockInfo->yStart = m_DimsIn.yStart;
	pSBlockInfo->yEnd = m_DimsIn.yEnd;

	//ÿ������ÿ����Ŀ��

	int iWidthPerBlock = g_BlockSize / (iDT * iRow);
	int iNumOfBlockPerBand = ceil( (double)(iCol * 1.0 / (double)iWidthPerBlock) );
	if (iWidthPerBlock > iCol)
	{
		pSBlockInfo->xStart = m_DimsIn.xStart;
		pSBlockInfo->xEnd = m_DimsIn.xEnd;
	}
	else
	{
		//row�����ڵĿ�ţ�0��ʼ
		int NOBlock = row / iWidthPerBlock;
		pSBlockInfo->xStart = m_DimsIn.xStart + NOBlock * iWidthPerBlock;
		//�������һ�����������СһЩ
		if (NOBlock == iNumOfBlockPerBand - 1)
		{
			pSBlockInfo->xEnd = m_DimsIn.xEnd;
		}
		else
		{
			pSBlockInfo->xEnd = pSBlockInfo->xStart + iWidthPerBlock;
		}
		//��ȡ��ʵ������
		pSBlockInfo->band = m_DimsIn.getImgBandNum(band);
		//���ÿ�߶ȿ��
		pSBlockInfo->setHeightWidth();
	}
	return;
}

void HAPBEGBase::WGetBlockInfo(int col, int row, int band, SimDIMS * pSBlockInfo)
{
	if (m_DimsIn.xEnd == -1 || m_DimsIn.yEnd == -1 || pSBlockInfo == NULL)
	{
		return;
	}
	int iRow = m_DimsIn.getHeight();
	int iCol = m_DimsIn.getWidth();
	int iBands = m_DimsIn.getDIMSBands();
	int iDT = m_nPtLength;

	pSBlockInfo->xStart = m_DimsIn.xStart;
	pSBlockInfo->xEnd = m_DimsIn.xEnd;

	//ÿ������ÿ����Ŀ��
	int iLineWidthstep = iCol * iDT * iBands;
	int iBlockHeight = int(g_BlockSize / iLineWidthstep);
	if (iBlockHeight > iRow)
	{
		iBlockHeight = iRow;
	}
	//�ֿ���Ŀ
	int iBlockCount = ceil((double)iRow / (double)iBlockHeight);
	//��ȡָ��
	int index = int(row / iBlockHeight);
	//����blockinfo���
	pSBlockInfo->yStart = index * iBlockHeight;
	//��������һ��
	if (index + 1 == iBlockCount )
	{
		int _t = index * iBlockHeight;
		iBlockHeight = iRow - _t;
	}
	pSBlockInfo->yEnd = pSBlockInfo->yStart + iBlockHeight;
	//���ÿ�߶ȿ��
	pSBlockInfo->setHeightWidth();
	//���Ƚ�������Ϊ-1
	pSBlockInfo->band = m_DimsIn.band;

	return;
}

void * HAPBEGBase::WRead(long column, long line, long band)
{
	//�ϲ��û����Ƿ�Χѡ��ֱ�Ӵ�0��ʼ���д�㷨
	column += m_nOffsetX;
	line += m_nOffsetY;

	int iBandInDims = m_DimsIn.getImgBandNum(band);
	if ((line < m_SBlockInInf.yEnd) && (line >= m_SBlockInInf.yStart))
	{
		return &m_bBlockIn[m_SBlockInInf.wGetCacheLoc(band, column, line) * m_nPtLength];
	}
	else
	if ((line < m_SBlockInInf2.yEnd) && (line >= m_SBlockInInf2.yStart))
	{
		return &m_bBlockIn2[m_SBlockInInf2.wGetCacheLoc(band, column, line) * m_nPtLength];
	}
	else
	{
		//�������ݣ�ָ��ָ��׼�����϶��������ݿ�
		SimDIMS * pSDBlockInfo = NULL;
		BYTE * pBBlockIn = RequestCache(pSDBlockInfo);
		WGetBlockInfo(column, line, band, pSDBlockInfo);
		m_MgrIn->WGetDIMSDataByBlockInfo(pSDBlockInfo, &m_DimsIn, pBBlockIn);
		return &pBBlockIn[pSDBlockInfo->wGetCacheLoc(band, column, line) * m_nPtLength];
	}

}


void * HAPBEGBase::BRead(long column, long line, long band)
{
	if (
		(column < m_SBlockInInf.xEnd) &&
		(column >= m_SBlockInInf.xStart) &&
		(line < m_SBlockInInf.yEnd) && 
		(line >= m_SBlockInInf.yStart) &&
		(band == m_SBlockInInf.band)
		)
	{
		return &m_bBlockIn[m_SBlockInInf.getCacheLoc(column, line) * m_nPtLength];
	}
	else	
	if (
		(column < m_SBlockInInf2.xEnd) &&
		(column >= m_SBlockInInf2.xStart) &&
		(line < m_SBlockInInf2.yEnd) &&
		(line >= m_SBlockInInf2.yStart) &&
		(band == m_SBlockInInf2.band)
		)
	{
		return &m_bBlockIn2[m_SBlockInInf2.getCacheLoc(column, line) * m_nPtLength];
	}
	else
	{
		//�������ݣ�ָ��ָ��׼�����϶��������ݿ�
		SimDIMS * pSDBlockInfo = NULL;
		BYTE * pBBlockIn = RequestCache(pSDBlockInfo);
		BGetBlockInfo(column, line, band, pSDBlockInfo);
		m_MgrIn->GetDIMSDataByBlockInfo(pSDBlockInfo, pBBlockIn);
		return &pBBlockIn[pSDBlockInfo->getCacheLoc(column, line) * m_nPtLength];
	}

}

void * HAPBEGBase::CRead(long column, long line, long band)
{
	//�ϲ��û����Ƿ�Χѡ��ֱ�Ӵ�0��ʼ���д�㷨
	column += m_nOffsetX;
	line += m_nOffsetY;

	if (
		(column < m_SBlockInInf.xEnd) && 
		(column >= m_SBlockInInf.xStart) &&
		(band == m_SBlockInInf.band)
		)
	{
		return &m_bBlockIn[m_SBlockInInf.getCacheLoc(column, line) * m_nPtLength];
	}
	else
	if (
		(column < m_SBlockInInf2.xEnd) &&
		(column >= m_SBlockInInf2.xStart) &&
		(band == m_SBlockInInf2.band)
		)
	{
		return &m_bBlockIn2[m_SBlockInInf2.getCacheLoc(column, line) * m_nPtLength];
	}
	else
	{
		//�������ݣ�ָ��ָ��׼�����϶��������ݿ�
		SimDIMS * pSDBlockInfo = NULL;
		BYTE * pBBlockIn = RequestCache(pSDBlockInfo);
		CGetBlockInfo(column, line, band, pSDBlockInfo);
		m_MgrIn->GetDIMSDataByBlockInfo(pSDBlockInfo, pBBlockIn);
		return &pBBlockIn[pSDBlockInfo->getCacheLoc(column, line) * m_nPtLength];
	}
}

void * HAPBEGBase::RRead(long column, long line, long band)
{
	if (
		(line < m_SBlockInInf.yEnd) &&
		(line >= m_SBlockInInf.yStart) &&
		(band == m_SBlockInInf.band)
		)
	{
		return &m_bBlockIn[m_SBlockInInf.getCacheLoc(column, line) * m_nPtLength];
	}
	else
	if (
		(line < m_SBlockInInf2.yEnd) &&
		(line >= m_SBlockInInf2.yStart) &&
		(band == m_SBlockInInf2.band)
		)
	{
		return &m_bBlockIn2[m_SBlockInInf2.getCacheLoc(column, line) * m_nPtLength];
	}
	else
	{
		//�������ݣ�ָ��ָ��׼�����϶��������ݿ�
		SimDIMS * pSDBlockInfo = NULL;
		BYTE * pBBlockIn = RequestCache(pSDBlockInfo);
		RGetBlockInfo(column, line, band, pSDBlockInfo);
		m_MgrIn->GetDIMSDataByBlockInfo(pSDBlockInfo, &m_DimsIn, pBBlockIn);
		return &pBBlockIn[pSDBlockInfo->getCacheLoc(column, line) * m_nPtLength];
	}
}

bool HAPBEGBase::Write(void * pValue)
{
	if (m_MgrOut == NULL)
	{
		return false;
	}
	m_writeFlag = 0;
	//��������������ڴ�
	if (m_OutInMem)
	{
		if (m_WriteDoneCount < m_OutBufferSize)
		{
			memcpy(&m_bBlockOut[m_WriteDoneCount*m_nPtLengthOut], pValue, m_nPtLength);
			m_WriteDoneCount++;
			return true;
		}
		else
		{
			return false;
		}
	}

	if (m_bBlockOut == NULL)
	{
		m_OutBufferSize = g_BlockSize / m_nPtLength;
		m_bBlockOut = new BYTE[m_OutBufferSize * m_nPtLength];
		m_WriteDoneCount = 0;
	}

	memcpy(m_bBlockOut + m_WriteDoneCount * m_nPtLength, pValue, m_nPtLength);
	m_WriteDoneCount++;
	if (m_WriteDoneCount == m_OutBufferSize)
	{
		//д���
		m_MgrOut->WriteBlock(m_bBlockOut, m_OutBufferSize * m_nPtLength);
		m_WriteDoneCount = 0;
	}
	return true;
}


bool HAPBEGBase::WWrite(void * pValue, int band)
{
	m_writeFlag = 1;
	//��ȡ������
	int nBands = m_MgrOut->m_header.m_nBands;
	if (m_bBlockOut == NULL)
	{
		m_OutBufferSize = int(g_BlockSize / m_nPtLength / nBands) * nBands;
		m_bBlockOut = new BYTE[m_OutBufferSize * m_nPtLength];
		m_WriteDoneCount = 0;
		m_writePos = 0;
	}
	//��ȡÿ��������m_OutBufferSize�е��ֽڳ���
	int bytesPerBand = (m_OutBufferSize* m_nPtLength) / nBands;
	//��������������ڴ�
	if (m_OutInMem)
	{
		if (m_WriteDoneCount < bytesPerBand)
		{
			memcpy(&m_bBlockOut[m_WriteDoneCount*m_nPtLengthOut], pValue, m_nPtLength);
			m_WriteDoneCount++;
			return true;
		}
		else
		{
			return false;
		}
	}

	if (m_bBlockOut == NULL)
	{
		m_OutBufferSize = g_BlockSize / m_nPtLength;
		m_bBlockOut = new BYTE[m_OutBufferSize * m_nPtLength];
		m_WriteDoneCount = 0;
	}

	memcpy(m_bBlockOut + m_WriteDoneCount * m_nPtLength, pValue, m_nPtLength);
	m_WriteDoneCount++;
	if (m_WriteDoneCount == m_OutBufferSize)
	{
		//д���
		m_MgrOut->WriteBlock(m_bBlockOut, m_OutBufferSize * m_nPtLength);
		m_WriteDoneCount = 0;
	}
	return true;
}

CGDALFileManager * HAPBEGBase::Get_MgrIn()
{
	return m_MgrIn;
}

CGDALFileManager * HAPBEGBase::Get_MgrOut()
{
	return m_MgrOut;
}

