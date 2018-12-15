#include "HAPBEGThreadC.h"
#include <process.h>

HAPBEGThreadC::HAPBEGThreadC()
{
	m_bBlockOut = NULL;
	m_bBlockOut2 = NULL;
	m_pbBlockIn = NULL;
	m_ppSBlockInInf = NULL;
	m_MgrIn = NULL;
	m_MgrOut = NULL;
	m_bNeedsDataNow = false;
	m_bIOThreadRun = false;
	m_iCurrentBlockNo = -1;
	m_iCurrentOutBlockNo = -1;
	m_outBufferSize = 0;
	m_iBlockOverlap = 0;
	//Ĭ�ϻ��������16��
	m_QueueLength = 16;
	strcpy(pcErrMessage, "");

}

HAPBEGThreadC::~HAPBEGThreadC()
{
	if (m_pbBlockIn != NULL)
	{
		delete[] m_pbBlockIn;
		m_pbBlockIn = NULL;
	}

	if (m_ppSBlockInInf != NULL)
	{
		delete[] m_ppSBlockInInf;
		m_ppSBlockInInf = NULL;
	}

	//Ҳ�п���ͼ��̫С��m_bBlockOut2�ռ�̫С����û���ϣ�����Ŀǰ�汾Ҫ����뿪��д˫����
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

bool HAPBEGThreadC::Set_MgrIn(CGDALFileManager * mgr, DIMS dims, int iBlockOverlap)
{
	//���֧��9*9ģ��
	if (iBlockOverlap > 8 || iBlockOverlap < 0 )
	{
		strcpy(pcErrMessage, "������ص�������Խ��!");
		return false;
	}
	m_iBlockOverlap = iBlockOverlap;
	if (m_MgrIn != NULL)
	{
		m_MgrIn->Close();
	}
	if (
		mgr->m_header.m_nLines < dims.yEnd ||
		mgr->m_header.m_nSamples < dims.xEnd ||
		dims.band < mgr->m_header.m_nBands
		)
	{
		strcpy(pcErrMessage, "DIMS����Խ��!");
		return false;
	}
	m_MgrIn = mgr;
	m_DimsIn = dims;
	
	m_nPtLength = m_MgrIn->m_header.getBytesPerPt();
	int iBand = m_DimsIn.band;

	//�����ͬ����ڶ��ε���Set_MgrIn������Բ����ظ������ڴ�
	//ÿ�黺���ֽڴ�С
	int squareBlockSize = iBand * g_SquareBorderSize * g_SquareBorderSize * m_nPtLength;
	if (!m_pbBlockIn && !m_ppSBlockInInf)
	{
		//�������1
		m_QueueLength = g_BlockSize * 2 / squareBlockSize;
		m_pbBlockIn = new BYTE[g_BlockSize * 2];
		m_ppSBlockInInf = new SimDIMS*[m_QueueLength];
		for (int i = 0; i < m_QueueLength; i++)
		{
			m_ppSBlockInInf[i] = new SimDIMS;
			m_ppSBlockInInf[i]->initDIMS();
			//���仺��ָ��λ��
			m_ppSBlockInInf[i]->pData = m_pbBlockIn + i * squareBlockSize;
		}
	}
	else
	{
		for (int i = 0; i < m_QueueLength; i++)
		{
			m_ppSBlockInInf[i]->initDIMS();
			//���仺��ָ��λ��
			m_ppSBlockInInf[i]->pData = m_pbBlockIn + i * squareBlockSize;
		}
	}
	m_iCurrentBlockNo = 0;
	return true;
}

bool HAPBEGThreadC::Set_MgrOut(CGDALFileManager * mgr, const char* fileName)
{
	if (m_MgrOut != NULL)
	{
		m_MgrOut->Close();
		m_MgrOut = NULL;
	}
	m_MgrOut = mgr;
	if (fileName == NULL)
	{
		strcpy(pcErrMessage, "����ļ�·���������룡");
		m_bThreadAbnormal = true;
		return false;
	}
	if (!m_MgrOut->HpsSaveImageAs(fileName))
	{
		strcpy(pcErrMessage, "����ļ���ȡ����");
		m_bThreadAbnormal = true;
		return false;
	}
	m_nPtLengthOut = m_MgrOut->m_header.getBytesPerPt();
	//ͳ���������
	setBlockNumber(false);

	//Ŀǰֻ֧��ͼ�����������ģʽ
	//���̱߳�����ʼ��
	if (m_bBlockOut == NULL)
	{
		m_bBlockOut = new BYTE[g_BlockSizeW];
	}
	m_WriteDoneCount = 0;
	m_outBufferSize = 0;
	if (m_bBlockOut2 == NULL)
	{
		m_bBlockOut2 = new BYTE[g_BlockSizeW];
	}
	m_outBufferSize2 = 0;
	return true;
}

void HAPBEGThreadC::IndependentOutThread(void * dummy)
{
	HAPBEGThreadC * pH = (HAPBEGThreadC*)dummy;
	int states = -1;
	while (true)
	{
		states = pH->blockOutToDiskData();
		if (states == 1)
		{
			_sleep(5);
		}
		else if (states == 0)
		{
			return;
		}
	}
}
void * HAPBEGThreadC::Read(long column, long line, long band)
{
	for (int i = 0; i < m_iCurrentBlockNo; i++)
	{
		if ((line < m_ppSBlockInInf[i]->yEnd) &&
			(line >= m_ppSBlockInInf[i]->yStart) &&
			(column < m_ppSBlockInInf[i]->xEnd ) &&
			(column >= m_ppSBlockInInf[i]->xStart) 
			)
		{
			return m_ppSBlockInInf[i]->pData + 
				m_ppSBlockInInf[i]->bandSize * band +
				(line * m_ppSBlockInInf[i]->xWidth + column - m_ppSBlockInInf[i]->bandOffset) * m_nPtLength;
		}
	}
	//������Ϣ���е�ַ�ֻ�һȦ����ʼ��ʱ��תСȦ�����ת��Ȧ
	if (m_iCurrentBlockNo < m_QueueLength)
	{
		m_iCurrentBlockNo++;
	}
	
	//������Ϣ���е�ַ�ֻ�һȦ����ʼ��ʱ��תСȦ�����ת��Ȧ
	SimDIMS * pSDIMSTemp = m_ppSBlockInInf[m_iCurrentBlockNo - 1];
	for (int i = m_iCurrentBlockNo-2; i>= 0; i--)
	{
		m_ppSBlockInInf[i + 1] = m_ppSBlockInInf[i];
	}
	//��0��ӣ���֤���ݷ��ʵĸ�Ч
	if (!getBlockInfo(column, line,band, m_ppSBlockInInf[0]))
	{
		strcpy(pcErrMessage, "��ȡͼ��ΧԽ�磡");
		m_bThreadAbnormal = true;
		return NULL;
	}
	while ( m_bIOThreadRun)
	{
		_sleep(5);
	}
	m_bIOThreadRun = true;
	m_MgrIn->WGetDIMSDataByBlockInfo(m_ppSBlockInInf[0], &m_DimsIn, &m_ppSBlockInInf[0]->pData);
	m_bIOThreadRun = false;
	return m_ppSBlockInInf[0]->pData +
		m_ppSBlockInInf[0]->bandSize * band +
		(line * m_ppSBlockInInf[0]->xWidth + column - m_ppSBlockInInf[0]->bandOffset) * m_nPtLength;
}
bool HAPBEGThreadC::Write(void * pValue)
{
	while (true)
	{
		if (m_WriteDoneCount < m_outBufferSize)
		{
			int band = m_WriteDoneCount % m_SBlockOutInf.band;
			memcpy(m_bBlockOut + (band * m_outBufferSize + m_WriteDoneCount ) / m_SBlockOutInf.band * m_nPtLengthOut, pValue, m_nPtLengthOut);
			m_WriteDoneCount++;
			return true;
		}
		else
		if (m_outBufferSize == 0)
		{
			m_iCurrentOutBlockNo = 0;
			//��ʼ���������Ϣ
			getBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
			m_outBufferSize = m_SBlockOutInf.wGetCacheSize();
			_beginthread(IndependentOutThread, 0, (void*) this);
		}
		else
		if ( m_outBufferSize2 == 0) //��ʾ2�Ż�����û��ִ��
		{
			//�û�1��2������ָ���simDIMS����
			BYTE * pBBlockOutTemp = m_bBlockOut;
			m_bBlockOut = m_bBlockOut2;
			m_bBlockOut2 = pBBlockOutTemp;
			//����ִ��д�����ˣ����ñ�����д�̵߳Ŀ��Ʊ�����������Щ��ֵ��
			m_outBufferSize2 = m_outBufferSize;
			//����У�����㷨IO�����û����˲��ں����㷨����Ҫ
			//m_SBlockOutInf2 = m_SBlockOutInf;
			m_iCurrentOutBlockNo++;
			//��ʼ���������Ϣ
			getBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
			m_outBufferSize = m_SBlockOutInf.wGetCacheSize();
			memcpy(m_bBlockOut, pValue, m_nPtLengthOut);
			m_WriteDoneCount = 1;
			return true;
		}
		else
		{
			//�ȴ�IO�߳��������׼����ʱ���趨�Ķ�һЩ�����ܿ����к��뼶��Ӱ��
			_sleep(5);
		}
	}
}

void HAPBEGThreadC::Close_MgrIn()
{
	for (int i = 0; i < m_QueueLength; i++)
	{
		m_ppSBlockInInf[i]->initDIMS();
	}
	m_iCurrentBlockNo = -1;
	if (m_MgrIn != NULL)
	{
		m_MgrIn->Close();
		m_MgrIn = NULL;
	}
}

void HAPBEGThreadC::Close_MgrOut()
{
	while (m_iCurrentOutBlockNo != -1)
	{
		_sleep(5);
	}
	m_SBlockOutInf.initDIMS();
	m_WriteDoneCount = 0;
	m_outBufferSize = 0;
	if (m_MgrOut != NULL)
	{
		m_MgrOut->Close();
		m_MgrOut = NULL;
	}
}
bool HAPBEGThreadC::getBlockInfo(long column, long line, long band, SimDIMS* pSBlockInfo)
{
	if (
		column < 0 ||
		column >= m_DimsIn.xWidth ||
		line < 0 ||
		line >= m_DimsIn.yHeight ||
		band < 0 ||
		band >= m_DimsIn.band
		)
	{
		return false;
	}		
	int iRow = m_DimsIn.getHeight();
	int iCol = m_DimsIn.getWidth();
	int iBand = m_DimsIn.band;
	int iIndexInRow = 0;
	int iIndexInCol = 0;
	int numberInCol = 0;
	int numberInRow = 0;
	if (iCol <= g_SquareBorderSize )
	{
		numberInCol = 1;
		pSBlockInfo->xStart = 0;
		pSBlockInfo->xEnd = iCol;
	}
	else
	{
		numberInCol = int((iCol - g_SquareBorderSize) * 1.0 / (g_SquareBorderSize - m_iBlockOverlap) + 0.9999) + 1;
		//�����ȵ�������������
		if (column < g_SquareBorderSize )
		{
			iIndexInCol = 0;
			pSBlockInfo->xStart = 0;
			pSBlockInfo->xEnd = g_SquareBorderSize;
		}
		else
		{
			iIndexInCol = (column - g_SquareBorderSize) / (g_SquareBorderSize - m_iBlockOverlap) + 1;
			pSBlockInfo->xStart = iIndexInCol * (g_SquareBorderSize - m_iBlockOverlap);
			//�������һ�����������СһЩ
			if (iIndexInCol == numberInCol - 1)
			{
				pSBlockInfo->xEnd = iCol;
			}
			else
			{
				pSBlockInfo->xEnd = pSBlockInfo->xStart + g_SquareBorderSize;
			}
		}
	}
	if (iRow <= g_SquareBorderSize)
	{
		numberInRow = 1;
		pSBlockInfo->yStart = 0;
		pSBlockInfo->yEnd = iRow;
	}
	else
	{
		numberInRow = int((iRow - g_SquareBorderSize) * 1.0 / (g_SquareBorderSize - m_iBlockOverlap) + 0.9999) + 1;
		//�����ȵ�������������
		if (line < g_SquareBorderSize)
		{
			iIndexInRow = 0;
			pSBlockInfo->yStart = 0;
			pSBlockInfo->yEnd = g_SquareBorderSize;
		}
		else
		{
			iIndexInRow = (line - g_SquareBorderSize) / (g_SquareBorderSize - m_iBlockOverlap) + 1;
			pSBlockInfo->yStart = iIndexInRow * (g_SquareBorderSize - m_iBlockOverlap);
			//�������һ�����������СһЩ
			if (iIndexInRow == numberInRow - 1)
			{
				pSBlockInfo->yEnd = iRow;
			}
			else
			{
				pSBlockInfo->yEnd = pSBlockInfo->yStart + g_SquareBorderSize;
			}
		}
	}
	//���ÿ�߶ȿ��
	pSBlockInfo->setHeightWidth();
	pSBlockInfo->band = iBand;
#ifdef _LXDEBUG
	fprintf(ptf, "��ǰ����:%d,��ǰ�ֿ���:%d\n", pSBlockInfo->band, iIndexInRow * numberInCol + iIndexInCol);
#endif
	return true;
}

//ͨ��ͼ��������Ż�ȡ�������Ϣ
void HAPBEGThreadC::getBlockInfoByIndex(int iIndex, SimDIMS * pSBlockInfo, bool inOrOut)
{
	if (m_DimsIn.xEnd == -1 || m_DimsIn.yEnd == -1 || pSBlockInfo == NULL || iIndex < 0)
	{
		return;
	}
	//squareģʽֻ�����ʱ����index
	if (inOrOut)
	{
		return;
	}
	
	int iRow = m_MgrOut->m_header.m_nLines;
	int iCol = m_MgrOut->m_header.m_nSamples;
	int iBand = m_MgrOut->m_header.m_nBands;
	int iDT = m_nPtLengthOut;
	//ÿ��ӵ�е�����
	int iLinesPerBlock = g_BlockSizeW / (iDT * iCol * iBand);
	pSBlockInfo->xStart = 0;
	pSBlockInfo->xEnd = iCol;
	pSBlockInfo->band = iBand;
	//g_BlockSize�������iCol*m_nPtLength,�����С���õ�ʱ��ע��
	//�����ȵ�������������
	if (iLinesPerBlock >= iRow)
	{
		pSBlockInfo->yStart = 0;
		pSBlockInfo->yEnd = iRow;
	}
	else
	{
		pSBlockInfo->yStart = iIndex * iLinesPerBlock;
		//�������һ�����������СһЩ
		if (iIndex == m_iBlockNumOut - 1)
		{
			pSBlockInfo->yEnd = iRow;
		}
		else
		{
			pSBlockInfo->yEnd = pSBlockInfo->yStart + iLinesPerBlock;
		}

	}
	
	//���ÿ�߶ȿ��
	pSBlockInfo->setHeightWidth();
}

int HAPBEGThreadC::getBlockIndex(long column, long line, long band)
{
	if ( m_DimsIn.xEnd == -1 || m_DimsIn.yEnd == -1 )
	{
		return -1;
	}
	int iRow = m_DimsIn.getHeight();
	int iCol = m_DimsIn.getWidth();
	int iBand = m_DimsIn.getDIMSBands();
	int iDT = m_nPtLength;
	//ÿ��ӵ�е�����
	int iLinesPerBlock = g_BlockSize / (iDT * iCol);
	int iIndex = -1;
	//�����ȵ�������������
	if (iLinesPerBlock >= iRow)
	{
		iIndex = band;
	}
	else if (m_iBlockOverlap == 0)
	{
		int iNumOfBlockPerBand = int(iRow * 1.0 / iLinesPerBlock + 0.99999999);
		//iIndex��0������ֻ�ܹ��ﵽiBlockNum-1
		//line�����ڵĿ�����0��ʼ
		int iIndexInBand = line / iLinesPerBlock;
		iIndex = band * iNumOfBlockPerBand + iIndexInBand;

	}
	else
	{
		//�ص��������¼���
		int iRowWithOverlap = iRow - iLinesPerBlock;
		int iNumOfBlockPerBand = int(iRowWithOverlap * 1.0 / (iLinesPerBlock - m_iBlockOverlap) + 0.9999) + 1;
		
		//iCurrentBlockNo��0��ʼ������ֻ���ﵽiBlockNum-1
		if (line < iLinesPerBlock)
		{
			iIndex = band * iNumOfBlockPerBand;
		}
		else
		{
			int iIndexInBand = (line - iLinesPerBlock) / (iLinesPerBlock - m_iBlockOverlap) + 1;
			iIndex = band * iNumOfBlockPerBand + iIndexInBand;
		}
	}
	return iIndex;
}

//��ȡͼ�������
void HAPBEGThreadC::setBlockNumber(bool inOrOut)
{
	//squareģʽֻ�����ʱ����index
	if (inOrOut)
	{
		return;
	}
	
	int iRow = m_MgrOut->m_header.m_nLines;
	int iCol = m_MgrOut->m_header.m_nSamples;
	int iBand = m_MgrOut->m_header.m_nBands;
	int iDT = m_nPtLengthOut;
	//g_BlockSize�������iCol*m_nPtLength,�����С���õ�ʱ��ע��
	//ÿ��ӵ�е�����
	int iLinesPerBlock = g_BlockSizeW / (iDT * iCol * iBand);
	//dims�ܿ���
	m_iBlockNumOut = int(iRow * 1.0 / iLinesPerBlock + 0.9999);
	
}
int HAPBEGThreadC::blockOutToDiskData()
{
	//�߳��쳣���߳̽�������������
	if (m_bThreadAbnormal || m_MgrOut == NULL )
	{
		m_iCurrentBlockNo = -1;
		return false;
	}
	if (!m_bIOThreadRun )
	{
		if (m_outBufferSize2)
		{
			//����IO�߳���
			m_bIOThreadRun = true;
			m_MgrOut->WWriteBlock(m_bBlockOut2, m_outBufferSize2 * m_nPtLengthOut);
			//�ͷ�IO�߳���
			m_bIOThreadRun = false;
			//һ��Ҫ�����0
			m_outBufferSize2 = 0;
			//������
			return -1;
		}
		else
		if (m_WriteDoneCount == m_outBufferSize && m_iCurrentOutBlockNo == m_iBlockNumOut - 1)
		{
			//����IO�߳���
			m_bIOThreadRun = true;
			m_MgrOut->WWriteBlock(m_bBlockOut, m_outBufferSize * m_nPtLengthOut);
			//�ͷ�IO�߳���
			m_bIOThreadRun = false;
			m_iCurrentOutBlockNo = -1;
			//������
			return false;
		}
	}
	return true;
}