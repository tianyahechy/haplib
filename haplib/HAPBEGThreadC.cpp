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
	//默认缓存队列是16块
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

	//也有可能图像太小，m_bBlockOut2空间太小，就没用上，但是目前版本要求必须开辟写双缓存
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
	//最高支持9*9模板
	if (iBlockOverlap > 8 || iBlockOverlap < 0 )
	{
		strcpy(pcErrMessage, "缓存块重叠区设置越界!");
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
		strcpy(pcErrMessage, "DIMS设置越界!");
		return false;
	}
	m_MgrIn = mgr;
	m_DimsIn = dims;
	
	m_nPtLength = m_MgrIn->m_header.getBytesPerPt();
	int iBand = m_DimsIn.band;

	//如果是同对象第二次调用Set_MgrIn，则可以不再重复开辟内存
	//每块缓存字节大小
	int squareBlockSize = iBand * g_SquareBorderSize * g_SquareBorderSize * m_nPtLength;
	if (!m_pbBlockIn && !m_ppSBlockInInf)
	{
		//必须大于1
		m_QueueLength = g_BlockSize * 2 / squareBlockSize;
		m_pbBlockIn = new BYTE[g_BlockSize * 2];
		m_ppSBlockInInf = new SimDIMS*[m_QueueLength];
		for (int i = 0; i < m_QueueLength; i++)
		{
			m_ppSBlockInInf[i] = new SimDIMS;
			m_ppSBlockInInf[i]->initDIMS();
			//分配缓存指针位置
			m_ppSBlockInInf[i]->pData = m_pbBlockIn + i * squareBlockSize;
		}
	}
	else
	{
		for (int i = 0; i < m_QueueLength; i++)
		{
			m_ppSBlockInInf[i]->initDIMS();
			//分配缓存指针位置
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
		strcpy(pcErrMessage, "输出文件路径必须输入！");
		m_bThreadAbnormal = true;
		return false;
	}
	if (!m_MgrOut->HpsSaveImageAs(fileName))
	{
		strcpy(pcErrMessage, "输出文件读取出错！");
		m_bThreadAbnormal = true;
		return false;
	}
	m_nPtLengthOut = m_MgrOut->m_header.getBytesPerPt();
	//统计输出块数
	setBlockNumber(false);

	//目前只支持图像输出至磁盘模式
	//多线程变量初始化
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
	//缓存信息队列地址轮换一圈，开始的时候转小圈，最后转大圈
	if (m_iCurrentBlockNo < m_QueueLength)
	{
		m_iCurrentBlockNo++;
	}
	
	//缓存信息队列地址轮换一圈，开始的时候转小圈，最后转大圈
	SimDIMS * pSDIMSTemp = m_ppSBlockInInf[m_iCurrentBlockNo - 1];
	for (int i = m_iCurrentBlockNo-2; i>= 0; i--)
	{
		m_ppSBlockInInf[i + 1] = m_ppSBlockInInf[i];
	}
	//从0入队，保证数据访问的高效
	if (!getBlockInfo(column, line,band, m_ppSBlockInInf[0]))
	{
		strcpy(pcErrMessage, "读取图像范围越界！");
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
			//初始化缓存块信息
			getBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
			m_outBufferSize = m_SBlockOutInf.wGetCacheSize();
			_beginthread(IndependentOutThread, 0, (void*) this);
		}
		else
		if ( m_outBufferSize2 == 0) //表示2号缓存区没有执行
		{
			//置换1，2缓冲区指针和simDIMS变量
			BYTE * pBBlockOutTemp = m_bBlockOut;
			m_bBlockOut = m_bBlockOut2;
			m_bBlockOut2 = pBBlockOutTemp;
			//可以执行写任务了，（该变量是写线程的控制变量，尽量晚些赋值）
			m_outBufferSize2 = m_outBufferSize;
			//几何校正类算法IO必须置换，滤波融合类算法不需要
			//m_SBlockOutInf2 = m_SBlockOutInf;
			m_iCurrentOutBlockNo++;
			//初始化缓存块信息
			getBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
			m_outBufferSize = m_SBlockOutInf.wGetCacheSize();
			memcpy(m_bBlockOut, pValue, m_nPtLengthOut);
			m_WriteDoneCount = 1;
			return true;
		}
		else
		{
			//等待IO线程完成数据准备，时间设定的短一些对性能可能有毫秒级的影响
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
		//缓存块比单波段数据量大
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
			//考虑最后一块的行数可能小一些
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
		//缓存块比单波段数据量大
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
			//考虑最后一块的行数可能小一些
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
	//设置块高度宽度
	pSBlockInfo->setHeightWidth();
	pSBlockInfo->band = iBand;
#ifdef _LXDEBUG
	fprintf(ptf, "当前波段:%d,当前分块编号:%d\n", pSBlockInfo->band, iIndexInRow * numberInCol + iIndexInCol);
#endif
	return true;
}

//通过图像块索引号获取缓存块信息
void HAPBEGThreadC::getBlockInfoByIndex(int iIndex, SimDIMS * pSBlockInfo, bool inOrOut)
{
	if (m_DimsIn.xEnd == -1 || m_DimsIn.yEnd == -1 || pSBlockInfo == NULL || iIndex < 0)
	{
		return;
	}
	//square模式只需输出时计算index
	if (inOrOut)
	{
		return;
	}
	
	int iRow = m_MgrOut->m_header.m_nLines;
	int iCol = m_MgrOut->m_header.m_nSamples;
	int iBand = m_MgrOut->m_header.m_nBands;
	int iDT = m_nPtLengthOut;
	//每块拥有的行数
	int iLinesPerBlock = g_BlockSizeW / (iDT * iCol * iBand);
	pSBlockInfo->xStart = 0;
	pSBlockInfo->xEnd = iCol;
	pSBlockInfo->band = iBand;
	//g_BlockSize必须大于iCol*m_nPtLength,缓存大小设置的时候注意
	//缓存块比单波段数据量大
	if (iLinesPerBlock >= iRow)
	{
		pSBlockInfo->yStart = 0;
		pSBlockInfo->yEnd = iRow;
	}
	else
	{
		pSBlockInfo->yStart = iIndex * iLinesPerBlock;
		//考虑最后一块的行数可能小一些
		if (iIndex == m_iBlockNumOut - 1)
		{
			pSBlockInfo->yEnd = iRow;
		}
		else
		{
			pSBlockInfo->yEnd = pSBlockInfo->yStart + iLinesPerBlock;
		}

	}
	
	//设置块高度宽度
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
	//每块拥有的行数
	int iLinesPerBlock = g_BlockSize / (iDT * iCol);
	int iIndex = -1;
	//缓存块比单波段数据量大
	if (iLinesPerBlock >= iRow)
	{
		iIndex = band;
	}
	else if (m_iBlockOverlap == 0)
	{
		int iNumOfBlockPerBand = int(iRow * 1.0 / iLinesPerBlock + 0.99999999);
		//iIndex从0计数，只能够达到iBlockNum-1
		//line行所在的块数，0开始
		int iIndexInBand = line / iLinesPerBlock;
		iIndex = band * iNumOfBlockPerBand + iIndexInBand;

	}
	else
	{
		//重叠区域重新计算
		int iRowWithOverlap = iRow - iLinesPerBlock;
		int iNumOfBlockPerBand = int(iRowWithOverlap * 1.0 / (iLinesPerBlock - m_iBlockOverlap) + 0.9999) + 1;
		
		//iCurrentBlockNo从0开始计数，只够达到iBlockNum-1
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

//获取图像块总数
void HAPBEGThreadC::setBlockNumber(bool inOrOut)
{
	//square模式只需输出时计算index
	if (inOrOut)
	{
		return;
	}
	
	int iRow = m_MgrOut->m_header.m_nLines;
	int iCol = m_MgrOut->m_header.m_nSamples;
	int iBand = m_MgrOut->m_header.m_nBands;
	int iDT = m_nPtLengthOut;
	//g_BlockSize必须大于iCol*m_nPtLength,缓存大小设置的时候注意
	//每块拥有的行数
	int iLinesPerBlock = g_BlockSizeW / (iDT * iCol * iBand);
	//dims总块数
	m_iBlockNumOut = int(iRow * 1.0 / iLinesPerBlock + 0.9999);
	
}
int HAPBEGThreadC::blockOutToDiskData()
{
	//线程异常或线程结束的条件满足
	if (m_bThreadAbnormal || m_MgrOut == NULL )
	{
		m_iCurrentBlockNo = -1;
		return false;
	}
	if (!m_bIOThreadRun )
	{
		if (m_outBufferSize2)
		{
			//开启IO线程锁
			m_bIOThreadRun = true;
			m_MgrOut->WWriteBlock(m_bBlockOut2, m_outBufferSize2 * m_nPtLengthOut);
			//释放IO线程锁
			m_bIOThreadRun = false;
			//一定要最后置0
			m_outBufferSize2 = 0;
			//不休眠
			return -1;
		}
		else
		if (m_WriteDoneCount == m_outBufferSize && m_iCurrentOutBlockNo == m_iBlockNumOut - 1)
		{
			//开启IO线程锁
			m_bIOThreadRun = true;
			m_MgrOut->WWriteBlock(m_bBlockOut, m_outBufferSize * m_nPtLengthOut);
			//释放IO线程锁
			m_bIOThreadRun = false;
			m_iCurrentOutBlockNo = -1;
			//不休眠
			return false;
		}
	}
	return true;
}