#include "HAPBEGThreadW.h"
#include <process.h>

HAPBEGThreadW::HAPBEGThreadW()
{
	m_bBlockOut = NULL;
	m_bBlockOut2 = NULL;
	m_bBlockIn = NULL;
	m_bBlockIn2 = NULL;
	m_MgrIn = NULL;
	m_MgrOut = NULL;
	m_bNeedsDataNow = false;
	m_bThreadAbnormal = false;
	m_iCurrentBlockNo = -1;
	m_iCurrentOutBlockNo = -1;
	m_outBufferSize = 0;
	m_bAlarmInfo = true;
	m_iBlockOverlap = 0;
	strcpy(pcErrMessage, "");

}

HAPBEGThreadW::~HAPBEGThreadW()
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

bool HAPBEGThreadW::Set_MgrIn(CGDALFileManager * mgr, DIMS dims, int iBlockOverlap)
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
	m_MgrIn = mgr;
	//�ж�dims�Ƿ��ʼ��
	if (
		mgr->m_header.m_nLines < dims.yEnd ||
		mgr->m_header.m_nSamples < dims.xEnd ||
		dims.band < mgr->m_header.m_nBands
		)
	{
		strcpy(pcErrMessage, "DIMS����Խ��!");
		return false;
	}
	m_DimsIn = dims;
	
	//ĿǰHAPBEGThreadW����ʱ���趨����д˫������ɶ�̬���䣬�����ļ����뺯��(Set_MgrIn,Set_MgrOut)�￪�ٻ���
	//�ɷ�������set_MgrIn���ö�ȡͼ�񣬶�̬���仺��֧��ģʽ�У�
	//��ͼ����дͼ��ģʽ����ͳ��ֱ��ͼ��
	//��ͼ��дͼ��ģʽ�����洦��
	//��ͼ���ȡ��д��ͼ��ģʽ��������ѧ�㷨����ͼ��
	//�޶�ͼ����дͼ��ģʽ���ڲ��������ԣ�һ������ã�
	if ( !m_bBlockIn)
	{
		m_bBlockIn = new BYTE[g_BlockSize];
	}
	m_SBlockInInf.initDIMS();
	if (!m_bBlockIn2)
	{
		m_bBlockIn2 = new BYTE[g_BlockSize];
	}
	m_SBlockInInf2.initDIMS();

	m_nPtLength = m_MgrIn->m_header.getBytesPerPt();
	setBlockNumber();
	//������䣬���ڲ����㷨��Ҫ��ȡͼ�񣬻��濪�ٲ�������read���������
	m_bNeedsDataNow = true;
	return true;
}

bool HAPBEGThreadW::Set_MgrOut(CGDALFileManager * mgr, const char* fileName)
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
		//�رն�д�߳�
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

void HAPBEGThreadW::independentInThread(void * dummy)
{
	HAPBEGThreadW * pH = (HAPBEGThreadW*)dummy;
	int states = -1;
	while (true)
	{
		states = pH->DiskDataToBlockIn();
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
void HAPBEGThreadW::IndependentOutThread(void * dummy)
{
	HAPBEGThreadW * pH = (HAPBEGThreadW*)dummy;
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

void * HAPBEGThreadW::Read(long column, long line, long band)
{
	while (true)
	{
		if ((line < m_SBlockInInf.yEnd) &&
			(line >= m_SBlockInInf.yStart))
		{
			return m_bBlockIn + ( band *m_SBlockInInf.bandSize + line * m_SBlockInInf.xWidth + column - m_SBlockInInf.bandOffset) * m_nPtLength;
		}
		else
		if ((line < m_SBlockInInf2.yEnd) &&
			(line >= m_SBlockInInf2.yStart) )
		{
			m_bNeedsDataNow = true;
			//�û�1��2������ָ���simDIMS����
			BYTE * pBBlockInTemp = m_bBlockIn;
			m_bBlockIn = m_bBlockIn2;
			m_bBlockIn2 = pBBlockInTemp;

			//������֧�������ȡ�����黺��ָ�뻻����ȥ�������Ƽ�����ʹ�ã���������overlap
			SimDIMS pSDBlockInfoTemp = m_SBlockInInf;
			m_SBlockInInf = m_SBlockInInf2;
			m_SBlockInInf2 = m_SBlockInInf;

			return m_bBlockIn + (band *m_SBlockInInf.bandSize + line * m_SBlockInInf.xWidth + column - m_SBlockInInf.bandOffset) * m_nPtLength;
		}
		else
		{
			if (m_bNeedsDataNow == true)
			{
				//IO���߳��Ѿ���������׼����ʼ
				if (m_iCurrentBlockNo == -1)
				{
					m_SBlockInInf.initDIMS();
					m_SBlockInInf2.initDIMS();
					m_bThreadAbnormal = false;
					m_iCurrentBlockNo = GetBlockIndex(column, line, band);
					if (m_iCurrentBlockNo == -1)
					{
						strcpy(pcErrMessage, "��ȡͼ��ΧԽ��!");
						m_bThreadAbnormal = true;
						return NULL;
					}
					_beginthread(independentInThread, 0, (void*) this);
				}
				else
				{
					//�ȴ�IO�߳��������׼��
					_sleep(5);
				}
			}
			else
			{
				//�û�д���㷨û�а��ն�ȡ˳����룬IO���߳�����װ���������������
				m_SBlockInInf.initDIMS();
				m_SBlockInInf2.initDIMS();
				m_bThreadAbnormal = false;
				m_iCurrentBlockNo = GetBlockIndex(column, line, band);
				if (m_iCurrentBlockNo == -1)
				{
					strcpy(pcErrMessage, "��ȡͼ��ΧԽ��!");
					m_bThreadAbnormal = true;
					return NULL;
				}
				m_bNeedsDataNow = true;
				if (m_bAlarmInfo)
				{
					strcpy(pcErrMessage, "���밴ָ��˳���ȡͼ�񣬶��̼߳������ִ�У���Ч���޷���֤��");
					m_bAlarmInfo = false;
				}
			}
		}
	}
}
bool HAPBEGThreadW::Write(void * pValue)
{
	while (true)
	{
		if (m_WriteDoneCount < m_outBufferSize)
		{
			int band = m_WriteDoneCount % m_SBlockOutInf.band;
			memcpy(m_bBlockOut + 
				( band * m_outBufferSize + m_WriteDoneCount) / m_SBlockInInf.band * m_nPtLengthOut, pValue, m_nPtLengthOut);
			m_WriteDoneCount++;
			return true;
		}
		else
		if (m_outBufferSize == 0)
		{
			m_iCurrentOutBlockNo = 0;
			//��ʼ���������Ϣ
			GetBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
			m_outBufferSize = m_SBlockOutInf.wGetCacheSize();
			_beginthread(IndependentOutThread, 0, (void*) this);
		}
		else
		if (m_outBufferSize2 == 0) //��ʾ2�Ż�����û��дִ������
		{
			//�û�1��2������ָ���simDIMS����
			BYTE * pBBlockOutTemp = m_bBlockOut;
			m_bBlockOut = m_bBlockOut2;
			m_bBlockOut2 = pBBlockOutTemp;
			//����ִ��д�����ˣ����ñ�����д�̵߳Ŀ��Ʊ�����������Щ��ֵ��
			m_outBufferSize2 = m_outBufferSize;
			m_iCurrentOutBlockNo++;
			//��ʼ���������Ϣ
			GetBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
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

void HAPBEGThreadW::Close_MgrIn()
{
	//�ȴ���ȡ�����������
	while (m_iCurrentBlockNo != -1)
	{
		_sleep(5);
	}
	m_SBlockInInf.initDIMS();
	m_SBlockInInf2.initDIMS();
	if (m_MgrIn != NULL)
	{
		m_MgrIn->Close();
		m_MgrIn = NULL;
	}
}

void HAPBEGThreadW::Close_MgrOut()
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
//ͨ��ͼ��������Ż�û�������������
int HAPBEGThreadW::getBlockLinesByIndex(int iIndex, bool InOrNot)
{
	if (
			m_DimsIn.xEnd == -1 ||
			m_DimsIn.yEnd == -1 ||
			iIndex < 0
		)
	{
		return -1;
	}		

	if (InOrNot)
	{
		int iRow = m_DimsIn.getHeight();
		int iCol = m_DimsIn.getWidth();
		int iBand = m_DimsIn.getDIMSBands();
		//ÿ��ӵ�е�����
		int iLinesPerBlock = g_BlockSize / (m_nPtLength * iCol * iBand);	
		//�����ȵ�������������
		if (iRow <= iLinesPerBlock)
		{
			return false;
		}
		else
		if (m_iBlockOverlap == 0)
		{
			int iNumOfBlock = int(iRow * 1.0 / iLinesPerBlock + 0.9999);
			//�������һ�����������СһЩ
			if (iIndex == iNumOfBlock - 1)
			{
				return iRow - iIndex * iLinesPerBlock;
			}
			else
			{
				return iLinesPerBlock;
			}
		}
		else
		{
			//�������һ�����������СһЩ
			if (iIndex == m_iBlockNum - 1)
			{
				return iRow - iIndex * (iLinesPerBlock - m_iBlockOverlap);
			}
			else
			{
				return iLinesPerBlock;
			}
		}
	}
	return -1;
}

//ͨ��ͼ��������Ż�ȡ�������Ϣ
void HAPBEGThreadW::GetBlockInfoByIndex(int iIndex, SimDIMS * pSBlockInfo, bool inOrOut)
{
	if (m_DimsIn.xEnd == -1 || m_DimsIn.yEnd == -1 || pSBlockInfo == NULL || iIndex < 0)
	{
		return;
	}
	if (inOrOut)
	{
		int iRow = m_DimsIn.getHeight();
		int iCol = m_DimsIn.getWidth();
		int iBand = m_DimsIn.band;
		pSBlockInfo->xStart = 0;
		pSBlockInfo->xEnd = iCol;
		pSBlockInfo->band = m_DimsIn.band;
		//ÿ��ӵ�е�����
		int iLinesPerBlock = g_BlockSize / (m_nPtLength * iCol * iBand);
		//�����ȵ�������������
		if (iLinesPerBlock >= iRow)
		{
			pSBlockInfo->yStart = 0;
			pSBlockInfo->yEnd = iRow;
		}
		else if (m_iBlockOverlap == 0)
		{
			int iNumOfBlock = int(iRow * 1.0 / iLinesPerBlock + 0.9999);
			pSBlockInfo->yStart = iIndex * iLinesPerBlock;
			//�������һ�����������СһЩ
			if (iIndex == iNumOfBlock - 1)
			{
				pSBlockInfo->yEnd = iRow;
			}
			else
			{
				pSBlockInfo->yEnd = pSBlockInfo->yStart + iLinesPerBlock;
			}
		}
		else
		{
			if (iIndex == 0)
			{
				pSBlockInfo->yStart = 0;
				pSBlockInfo->yEnd = iLinesPerBlock;
			}
			else
			{
				//���ڻ������ص����
				pSBlockInfo->yStart = iIndex * (iLinesPerBlock - m_iBlockOverlap);
				//�������һ�����������СһЩ
				if (iIndex == m_iBlockNum - 1)
				{
					pSBlockInfo->yEnd = iRow;
				}
				else
				{
					pSBlockInfo->yEnd = pSBlockInfo->yStart + iLinesPerBlock;
				}
			}
		}
	}
	else
	{
		int iDT = m_nPtLengthOut;
		int iRow = m_MgrOut->m_header.m_nLines;
		int iCol = m_MgrOut->m_header.m_nSamples;
		int iBand = m_MgrOut->m_header.m_nBands;
		pSBlockInfo->xStart = 0;
		pSBlockInfo->xEnd = iCol;

		pSBlockInfo->band = m_DimsIn.band;
		//ÿ��ӵ�е�����
		int iLinesPerBlock = g_BlockSizeW / (iDT * iCol * iBand);
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

	}
	//���ÿ�߶ȿ��
	pSBlockInfo->setHeightWidth();
}

int HAPBEGThreadW::GetBlockIndex(long column, long line, long band)
{
	if (column < 0 || column >= m_DimsIn.xWidth || line < 0 || line >= m_DimsIn.yHeight )
	{
		return -1;
	}
	int iRow = m_DimsIn.getHeight();
	int iCol = m_DimsIn.getWidth();
	int iBand = m_DimsIn.getDIMSBands();
	int iDT = m_nPtLength;
	//ÿ��ӵ�е�����
	int iLinesPerBlock = g_BlockSize / (iDT * iCol * iBand);
	int iIndex = -1;
	//�����ȵ�������������
	if (iLinesPerBlock >= iRow)
	{
		//index��0��ʼ������ֻ�ܴﵽiBlockNum-1
		iIndex = 0;
	}
	else if (m_iBlockOverlap == 0)
	{
		//line�����ڵĿ�����0��ʼ
		iIndex = line / iLinesPerBlock;
	}
	else
	{
		//iCurrentBlockNo��0��ʼ������ֻ���ﵽiBlockNum-1
		if (line < iLinesPerBlock)
		{
			iIndex = 0;
		}
		else
		{
			iIndex = (line - iLinesPerBlock) / (iLinesPerBlock - m_iBlockOverlap) + 1;
		}
	}
	return iIndex;
}

//��ȡͼ�������
int HAPBEGThreadW::getBlockNumber(bool inOrOut)
{
	return (inOrOut == true) ? m_iBlockNum : m_iBlockNumOut;
}
//����ͼ�������
void HAPBEGThreadW::setBlockNumber(bool inOrOut)
{
	//squareģʽֻ�����ʱ����index
	if (inOrOut)
	{
		int iRow = m_DimsIn.getHeight();
		int iCol = m_DimsIn.getWidth();
		int iBand = m_DimsIn.getDIMSBands();
		int iDT = m_nPtLength;
		//ÿ��ӵ�е�����
		int iLinesPerBlock = g_BlockSize / (iDT * iCol * iBand);
		int iIndex = -1;
		//�����ȵ�������������
		if (iLinesPerBlock >= iRow || m_iBlockOverlap == 0)
		{
			//dims�ܿ���
			m_iBlockNum = int(iRow * 1.0 / iLinesPerBlock + 0.9999);
		}
		else
		{
			//�������ص�ģʽ
			//�ص��������¼���
			int iRowWithOverlap = iRow - iLinesPerBlock;
			m_iBlockNum = int(iRowWithOverlap * 1.0 / (iLinesPerBlock - m_iBlockOverlap) + 0.9999) + 1;
		}
	}
	else
	{
		int iRow = m_MgrOut->m_header.m_nLines;
		int iCol = m_MgrOut->m_header.m_nSamples;
		int iBand = m_MgrOut->m_header.m_nBands;
		int iDT = m_nPtLengthOut;
		//g_BlockSize�������iCol*m_nPtLength,�����С���õ�ʱ��ע��
		//ÿ��ӵ�е�����
		int iLinesPerBlock = g_BlockSizeW / (iDT * iCol);
		//dims�ܿ���
		m_iBlockNumOut = int(iRow * 1.0 / iLinesPerBlock + 0.99999999);	
	}

}
int HAPBEGThreadW::DiskDataToBlockIn()
{
	//�������������
	//ע����Ϊ�����̵߳�ԭ������Read������ͨ��m_SBlockInInf�жϻ������ݴ����Ƿ�λ�ģ����m_SBlockInInf���������ڻ��������ֵ
	//������Ϣ�״μ���
	SimDIMS SDBlckInfoTemp;
	if (m_bThreadAbnormal ||
		m_MgrIn == NULL		//˵��close����������
		)
	{
		m_iCurrentBlockNo = -1;
		return false;
	}
	if (!m_bIOThreadRun && m_bNeedsDataNow)
	{
		if (m_SBlockInInf.band == -1)
		{
			//Ϊ��֤��ȡ����ʼ����ȷ�������ȸ�����鸳ֵ���ٸ�������Ϣ��ֵ��
			//����������ʼ�ն�����ǰ���ģ���Ϊ��ʹ������Ϣ��ʱû�ж��ϣ��㷨��ȡ��ʱ��Ҳ������Ҫ��Щ��ʱ��������
			GetBlockInfoByIndex(m_iCurrentBlockNo, &SDBlckInfoTemp);
			//����IO�߳���
			m_bIOThreadRun = true;
			m_MgrIn->WGetDIMSDataByBlockInfo(&SDBlckInfoTemp, &m_DimsIn, m_bBlockIn);
			//�ͷ�IO�߳���
			m_bIOThreadRun = false;
			//���ڲ�Ϊ�����־��Ϣ��ֵ
			m_SBlockInInf = SDBlckInfoTemp;
			m_iCurrentBlockNo++;
		}
		else
		{
			//�����ظ������ص����򣬸��죬ʤ��!
			GetBlockInfoByIndex(m_iCurrentBlockNo, &SDBlckInfoTemp);
			//����IO�߳���
			m_bIOThreadRun = true;
			m_MgrIn->WGetDIMSDataByBlockInfo(&SDBlckInfoTemp, &m_DimsIn, m_bBlockIn2);
			//�ͷ�IO�߳���
			m_bIOThreadRun = false;
			//���ڲ�Ϊ�����־��Ϣ��ֵ
			m_SBlockInInf2 = SDBlckInfoTemp;
			m_iCurrentBlockNo++;
			//��������鶼�Ѿ�װ������
			m_bNeedsDataNow = false;
		}
		//m_iCurrentBlockNo����Խ�磬��������ͼ�����
		if (m_iCurrentBlockNo == m_iBlockNum)
		{
			m_iCurrentBlockNo = -1;
			return false;
		}
		//������
		return -1;
	}
	return true;
}

int HAPBEGThreadW::blockOutToDiskData()
{
	//�߳��쳣���߳̽�������������
	if (m_bThreadAbnormal || m_MgrOut == NULL)
	{
		m_iCurrentBlockNo = -1;
		return false;
	}
	if (!m_bIOThreadRun)
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