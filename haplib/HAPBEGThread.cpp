#include "HAPBEGThread.h"
#include <process.h>

HAPBEGThread::HAPBEGThread()
{
	m_bBlockOut = NULL;
	m_bBlockOut2 = NULL;
	m_bBlockIn = NULL;
	m_bBlockIn2 = NULL;
	m_MgrIn = NULL;
	m_MgrOut = NULL;
	m_bNeedsDataNow = false;
	m_bIOThreadRun = false;
	m_iCurrentBlockNo = -1;
	m_iCurrentOutBlockNo = -1;
	m_outBufferSize = 0;
	m_bAlarmInfo = true;
	m_iBlockOverlap = 0;
	strcpy(pcErrMessage, "");

}

HAPBEGThread::~HAPBEGThread()
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

bool HAPBEGThread::Set_MgrIn(CGDALFileManager * mgr, DIMS dims, int iBlockOverlap)
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

	//ĿǰHAPBEGThread����ʱ���趨����д˫������ɶ�̬����,�����ļ����뺯����set_mgrin,set_mgrout)�￪�ٻ���
	//�ɷ�������set_mgrin���ö�ȡͼ�񣬶�̬���仺���ļ�ģʽ�У�
	//��ͼ����дͼ��ģʽ���磬ͳ��ֱ��ͼ��
	//��ͼ��дͼ��ģʽ�������洦����
	//��ͼ���ȡ��д��ͼ��ģʽ��������ѧ�㷨����ͼ��
	//�޶�ͼ����дͼ��ģʽ���ڲ��������ԣ�һ�������)
	//��Ϣ��ʼ��
	if (m_bBlockIn != NULL)
	{
		delete[] m_bBlockIn;
		m_bBlockIn = new BYTE[g_BlockSize];
	}
	m_SBlockInInf.initDIMS();
	if (m_bBlockIn2 != NULL)
	{
		delete[] m_bBlockIn2;
		m_bBlockIn2 = new BYTE[g_BlockSize];
	}
	m_SBlockInInf2.initDIMS();
	
	m_nPtLength = m_MgrIn->m_header.getBytesPerPt();
	setBlockNumber();
	//������䣬���ڲ����㷨��Ҫ������ȡͼ�񣬻��濪�ٲ�������Read���������
	m_bNeedsDataNow = true;
	return true;
}

bool HAPBEGThread::Set_MgrOut(CGDALFileManager * mgr, const char* fileName)
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

void HAPBEGThread::IndependentOutThread(void * dummy)
{
	HAPBEGThread * pH = (HAPBEGThread*)dummy;
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
//���û�ж���д�̣߳�д���ݵ����󾭳�����������������ͻ�����������½�
void HAPBEGThread::IndependentOutThread(void * dummy)
{
	HAPBEGThread * pH = (HAPBEGThread*)dummy;

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
void * HAPBEGThread::Read(long column, long line, long band)
{
	while (true)
	{
		if ((line < m_SBlockInInf.yEnd) &&
			(line >= m_SBlockInInf.yStart) &&
			(band == m_SBlockInInf.band))
		{
			return m_bBlockIn + (line * m_SBlockInInf.xWidth + column - m_SBlockInInf.bandOffset) * m_nPtLength;
		}
		else 
		if((line < m_SBlockInInf2.yEnd) &&
			(line >= m_SBlockInInf2.yStart) &&
			(band == m_SBlockInInf2.band))
		{
			//�û�1��2������ָ���simDIMS����
			BYTE * pBBlockInTemp = m_bBlockIn;
			m_bBlockIn = m_bBlockIn2;
			m_bBlockIn2 = pBBlockInTemp;

			//������֧�������ȡ�����黺��ָ�뻻����ȥ�������Ƽ�����ʹ�ã���������overlap
			SimDIMS pSDBlockInfoTemp = m_SBlockInInf;
			m_SBlockInInf = m_SBlockInInf2;
			m_SBlockInInf2 = m_SBlockInInf;

			//֪ͨIO�̸߳Ͻ�װ��ͼ�����ݣ��Ϸ������������ж���䣬������ñ�ȻӰ��Ч�ʡ�
			//��������Ȼ��������������ʱ���ܷ��Ӳ��е����ܣ���������ͨ�İ��д����㷨�ļ��������Եġ�
			//��Ϊͨ�������ַ�û���ÿ��ֻ����һ���жϾͿ��Ի�ȡ�����ˣ����鲨�������㷨���ð����ζ�ȡ��ʽִ��
			m_bNeedsDataNow = true;
			return m_bBlockIn + (line * m_SBlockInInf.xWidth + column - m_SBlockInInf.bandOffset) * m_nPtLength;
		}
		else
		{
			if (m_bNeedsDataNow == true )
			{
				//IO���߳��Ѿ���������׼����ʼ
				if (m_iCurrentBlockNo == -1)
				{
					m_SBlockInInf.initDIMS();
					m_SBlockInInf2.initDIMS();
					m_bThreadAbnormal = false;
					m_iCurrentBlockNo = getBlockIndex(column, line, band);
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
				m_iCurrentBlockNo = getBlockIndex(column, line, band);
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
bool HAPBEGThread::Write(void * pValue)
{
	while (true)
	{
		if (m_WriteDoneCount < m_outBufferSize)
		{
			memcpy(m_bBlockOut + m_WriteDoneCount * m_nPtLengthOut, pValue, m_nPtLengthOut);
			m_WriteDoneCount++;
			return true;
		}
		else
		if (m_outBufferSize == 0)
		{
			m_iCurrentOutBlockNo = 0;
			//��ʼ���������Ϣ
			getBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
			m_outBufferSize = m_SBlockOutInf.getCacheSize();
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
			m_iCurrentOutBlockNo++;
			//��ʼ���������Ϣ
			getBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
			m_outBufferSize = m_SBlockOutInf.getCacheSize();
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

void HAPBEGThread::Close_MgrIn()
{
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

void HAPBEGThread::Close_MgrOut()
{
	while (m_iCurrentOutBlockNo != -1)
	{
		_sleep(5);
	}
	m_SBlockInInf.initDIMS();
	m_WriteDoneCount = 0;
	m_outBufferSize = 0;
	if (m_MgrOut != NULL)
	{
		m_MgrOut->Close();
		m_MgrOut = NULL;
	}
}

//ͨ��ͼ��������Ż�ȡ�������Ϣ
void HAPBEGThread::getBlockInfoByIndex(int iIndex, SimDIMS * pSBlockInfo, bool inOrOut)
{
	if (inOrOut)
	{
		int iRow = m_DimsIn.getHeight();
		int iCol = m_DimsIn.getWidth();
		int iDT = m_nPtLength;
		pSBlockInfo->xStart = 0;
		pSBlockInfo->xEnd = iCol;
		//ÿ��ӵ�е�����
		int iLinesPerBlock = g_BlockSize / (iDT * iCol);
		//�����ȵ�������������
		if (iLinesPerBlock >= iRow )
		{
			pSBlockInfo->yStart = 0;
			pSBlockInfo->yEnd = iRow;
			pSBlockInfo->band = iIndex;
		}
		else if (m_iBlockOverlap == 0)
		{
			int iNumOfBlockPerBand = int(iRow * 1.0 / iLinesPerBlock + 0.99999999);
			pSBlockInfo->band = iIndex / iNumOfBlockPerBand;
			//index�ڸò��εĿ����
			int NoBlock = iIndex % iNumOfBlockPerBand;
			pSBlockInfo->yStart = NoBlock * iLinesPerBlock;
			//�������һ�����������СһЩ
			if (NoBlock == iNumOfBlockPerBand - 1)
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
			//�ص��������¼���
			int iRowWithOverlap = iRow - iLinesPerBlock;
			int iNumOfBlockPerBand = int(iRowWithOverlap * 1.0 / (iLinesPerBlock - m_iBlockOverlap) + 0.9999) + 1;
			pSBlockInfo->band = iIndex / iNumOfBlockPerBand;
			//index�ڸò��εĿ����
			int NoBlock = iIndex % iNumOfBlockPerBand;
			if (NoBlock )
			{
				pSBlockInfo->yStart = 0;
				pSBlockInfo->yEnd = iLinesPerBlock;
			}
			else
			{
				//���ڻ������ص����
				pSBlockInfo->yStart = NoBlock * (iLinesPerBlock - m_iBlockOverlap);
				//�������һ�����������СһЩ
				if (NoBlock == iNumOfBlockPerBand - 1)
				{
					pSBlockInfo->yEnd = iRow;
				}
				else
				{
					//Ϊ�˱�֤�˲���Ч��д���������ͷ���������С��
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
		//g_BlockSize�������iCol*m_nPtLength,�����С���õ�ʱ��ע��
		//ÿ��ӵ�е�����
		int iLinesPerBlock = g_BlockSizeW / (iDT * iCol);
		//�����ȵ�������������
		if (iLinesPerBlock >= iRow)
		{
			pSBlockInfo->yStart = 0;
			pSBlockInfo->yEnd = iRow;
			pSBlockInfo->band = iIndex;
		}
		else
		{
			int iNumOfBlockPerBand = int(iRow * 1.0 / iLinesPerBlock + 0.99999999);
			pSBlockInfo->band = iIndex / iNumOfBlockPerBand;
			//index�ڸò��εĿ����
			int NoBlock = iIndex % iNumOfBlockPerBand;
			pSBlockInfo->yStart = NoBlock * iLinesPerBlock;
			//�������һ�����������СһЩ
			if (NoBlock == iNumOfBlockPerBand - 1)
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

int HAPBEGThread::getBlockIndex(long column, long line, long band )
{
	if (column < 0 || 
		column >= m_DimsIn.xWidth || 
		line < 0 || 
		line >= m_DimsIn.yHeight ||
		band < 0 ||
		band >= m_DimsIn.band
		)
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
void HAPBEGThread::setBlockNumber(bool inOrOut)
{
	if (inOrOut)
	{
		int iRow = m_DimsIn.getHeight();
		int iCol = m_DimsIn.getWidth();
		int iDT = m_nPtLength;
		int iBand = m_DimsIn.getDIMSBands();
		//ÿ��ӵ�е�����
		int iLinesPerBlock = g_BlockSize / (iDT * iCol);
		//�����ȵ�������������
		if (iLinesPerBlock >= iRow ||
			m_iBlockOverlap == 0
			)
		{
			int iNumOfBlockPerBand = int(iRow * 1.0 / iLinesPerBlock + 0.99999999);
			//dims�ܿ���
			m_iBlockNum = iNumOfBlockPerBand * iBand;

		}
		else
		{
			//�ص��������¼���
			int iRowWithOverlap = iRow - iLinesPerBlock;
			int iNumOfBlockPerBand = int(iRowWithOverlap * 1.0 / (iLinesPerBlock - m_iBlockOverlap) + 0.9999) + 1;
			m_iBlockNum = iNumOfBlockPerBand * iBand;
		}
	}
	else
	{
		int iDT = m_nPtLengthOut;
		int iRow = m_MgrOut->m_header.m_nLines;
		int iCol = m_MgrOut->m_header.m_nSamples;
		int iBand = m_MgrOut->m_header.m_nBands;
		//g_BlockSize�������iCol*m_nPtLength,�����С���õ�ʱ��ע��
		//ÿ��ӵ�е�����
		int iLinesPerBlock = g_BlockSizeW / (iDT * iCol);
		//�����ȵ�������������
		int iNumOfBlockPerBand = int(iRow * 1.0 / iLinesPerBlock  + 0.9999);
		//dims�ܿ���
		m_iBlockNumOut = iNumOfBlockPerBand * iBand;
	}
}

int HAPBEGThread::DiskDataToBlockIn()
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
	if (!m_bIOThreadRun && m_bNeedsDataNow )
	{
		if (m_SBlockInInf.band == -1)
		{
			//Ϊ��֤��ȡ����ʼ����ȷ�������ȸ�����鸳ֵ���ٸ�������Ϣ��ֵ��
			//����������ʼ�ն�����ǰ���ģ���Ϊ��ʹ������Ϣ��ʱû�ж��ϣ��㷨��ȡ��ʱ��Ҳ������Ҫ��Щ��ʱ��������
			getBlockInfoByIndex(m_iCurrentBlockNo, &SDBlckInfoTemp);
#ifdef _LXDEBUG
			fprintf(ptf, "band:%d, start line:%d\n", SDBlckInfoTemp.band, SDBlckInfoTemp.Ystart);
#endif
			//����IO�߳���
			m_bIOThreadRun = true;
			m_MgrIn->GetDIMSDataByBlockInfo(&SDBlckInfoTemp, &m_DimsIn, m_bBlockIn);
			m_bIOThreadRun = false;
			//���ڲ�Ϊ�����־��Ϣ��ֵ
			m_SBlockInInf = SDBlckInfoTemp;
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

int HAPBEGThread::blockOutToDiskData()
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
			m_MgrOut->WriteBlock(m_bBlockOut2, m_outBufferSize2 * m_nPtLengthOut);
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
			m_MgrOut->WriteBlock(m_bBlockOut, m_outBufferSize * m_nPtLengthOut);
			//�ͷ�IO�߳���
			m_bIOThreadRun = false;
			m_iCurrentOutBlockNo = -1;
			//������
			return false;
		}
	}
	return true;
}