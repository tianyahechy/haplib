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

bool HAPBEGThread::Set_MgrIn(CGDALFileManager * mgr, DIMS dims, int iBlockOverlap)
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

	//目前HAPBEGThread调用时，设定读、写双缓存均可动态分配,故在文件载入函数（set_mgrin,set_mgrout)里开辟缓存
	//可反复调用set_mgrin设置读取图像，动态分配缓存文件模式有：
	//读图像：无写图像模式（如，统计直方图）
	//读图像：写图像模式，（常规处理）；
	//无图像读取，写出图像模式（依据数学算法创建图像）
	//无读图像，无写图像模式（内部函数测试，一般很少用)
	//信息初始化
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
	//缓存分配，由于部分算法需要反复读取图像，缓存开辟不适宜在Read函数中完成
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
//如果没有独立写线程，写数据的请求经常会与读数据请求相冲突，导致性能下降
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
			//置换1，2缓冲区指针和simDIMS变量
			BYTE * pBBlockInTemp = m_bBlockIn;
			m_bBlockIn = m_bBlockIn2;
			m_bBlockIn2 = pBBlockInTemp;

			//理论上支持乱序读取，两块缓存指针换来换去，但不推荐这样使用，还是设置overlap
			SimDIMS pSDBlockInfoTemp = m_SBlockInInf;
			m_SBlockInInf = m_SBlockInInf2;
			m_SBlockInInf2 = m_SBlockInInf;

			//通知IO线程赶紧装载图像数据，老方法不好设置判断语句，如果设置必然影响效率。
			//这样做虽然导致做波段运算时不能发挥并行的性能，但对于普通的按行处理算法的加速是明显的。
			//因为通过缓存地址置换，每次只用作一组判断就可以获取数据了，建议波段运算算法采用按波段读取方式执行
			m_bNeedsDataNow = true;
			return m_bBlockIn + (line * m_SBlockInInf.xWidth + column - m_SBlockInInf.bandOffset) * m_nPtLength;
		}
		else
		{
			if (m_bNeedsDataNow == true )
			{
				//IO读线程已经结束或者准备开始
				if (m_iCurrentBlockNo == -1)
				{
					m_SBlockInInf.initDIMS();
					m_SBlockInInf2.initDIMS();
					m_bThreadAbnormal = false;
					m_iCurrentBlockNo = getBlockIndex(column, line, band);
					if (m_iCurrentBlockNo == -1)
					{
						strcpy(pcErrMessage, "读取图像范围越界!");
						m_bThreadAbnormal = true;
						return NULL;
					}
					_beginthread(independentInThread, 0, (void*) this);
				}
				else
				{
					//等待IO线程完成数据准备
					_sleep(5);
				}
			}
			else
			{
				//用户写的算法没有按照读取顺序读入，IO读线程重新装载两个缓存的数据
				m_SBlockInInf.initDIMS();
				m_SBlockInInf2.initDIMS();
				m_bThreadAbnormal = false;
				m_iCurrentBlockNo = getBlockIndex(column, line, band);
				if (m_iCurrentBlockNo == -1)
				{
					strcpy(pcErrMessage, "读取图像范围越界!");
					m_bThreadAbnormal = true;
					return NULL;
				}
				m_bNeedsDataNow = true;
				if (m_bAlarmInfo)
				{
					strcpy(pcErrMessage, "必须按指定顺序读取图像，多线程计算继续执行，但效率无法保证！");
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
			//初始化缓存块信息
			getBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
			m_outBufferSize = m_SBlockOutInf.getCacheSize();
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
			m_iCurrentOutBlockNo++;
			//初始化缓存块信息
			getBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
			m_outBufferSize = m_SBlockOutInf.getCacheSize();
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

//通过图像块索引号获取缓存块信息
void HAPBEGThread::getBlockInfoByIndex(int iIndex, SimDIMS * pSBlockInfo, bool inOrOut)
{
	if (inOrOut)
	{
		int iRow = m_DimsIn.getHeight();
		int iCol = m_DimsIn.getWidth();
		int iDT = m_nPtLength;
		pSBlockInfo->xStart = 0;
		pSBlockInfo->xEnd = iCol;
		//每块拥有的行数
		int iLinesPerBlock = g_BlockSize / (iDT * iCol);
		//缓存块比单波段数据量大
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
			//index在该波段的块序号
			int NoBlock = iIndex % iNumOfBlockPerBand;
			pSBlockInfo->yStart = NoBlock * iLinesPerBlock;
			//考虑最后一块的行数可能小一些
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
			//重叠区域重新计算
			int iRowWithOverlap = iRow - iLinesPerBlock;
			int iNumOfBlockPerBand = int(iRowWithOverlap * 1.0 / (iLinesPerBlock - m_iBlockOverlap) + 0.9999) + 1;
			pSBlockInfo->band = iIndex / iNumOfBlockPerBand;
			//index在该波段的块序号
			int NoBlock = iIndex % iNumOfBlockPerBand;
			if (NoBlock )
			{
				pSBlockInfo->yStart = 0;
				pSBlockInfo->yEnd = iLinesPerBlock;
			}
			else
			{
				//存在缓冲区重叠情况
				pSBlockInfo->yStart = NoBlock * (iLinesPerBlock - m_iBlockOverlap);
				//考虑最后一块的行数可能小一些
				if (NoBlock == iNumOfBlockPerBand - 1)
				{
					pSBlockInfo->yEnd = iRow;
				}
				else
				{
					//为了保证滤波有效，写出缓存块两头都比输入块小于
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
		//g_BlockSize必须大于iCol*m_nPtLength,缓存大小设置的时候注意
		//每块拥有的行数
		int iLinesPerBlock = g_BlockSizeW / (iDT * iCol);
		//缓存块比单波段数据量大
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
			//index在该波段的块序号
			int NoBlock = iIndex % iNumOfBlockPerBand;
			pSBlockInfo->yStart = NoBlock * iLinesPerBlock;
			//考虑最后一块的行数可能小一些
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
	//设置块高度宽度
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
void HAPBEGThread::setBlockNumber(bool inOrOut)
{
	if (inOrOut)
	{
		int iRow = m_DimsIn.getHeight();
		int iCol = m_DimsIn.getWidth();
		int iDT = m_nPtLength;
		int iBand = m_DimsIn.getDIMSBands();
		//每块拥有的行数
		int iLinesPerBlock = g_BlockSize / (iDT * iCol);
		//缓存块比单波段数据量大
		if (iLinesPerBlock >= iRow ||
			m_iBlockOverlap == 0
			)
		{
			int iNumOfBlockPerBand = int(iRow * 1.0 / iLinesPerBlock + 0.99999999);
			//dims总块数
			m_iBlockNum = iNumOfBlockPerBand * iBand;

		}
		else
		{
			//重叠区域重新计算
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
		//g_BlockSize必须大于iCol*m_nPtLength,缓存大小设置的时候注意
		//每块拥有的行数
		int iLinesPerBlock = g_BlockSizeW / (iDT * iCol);
		//缓存块比单波段数据量大
		int iNumOfBlockPerBand = int(iRow * 1.0 / iLinesPerBlock  + 0.9999);
		//dims总块数
		m_iBlockNumOut = iNumOfBlockPerBand * iBand;
	}
}

int HAPBEGThread::DiskDataToBlockIn()
{
	//先填充两块数据
	//注：因为独立线程的原因，由于Read函数是通过m_SBlockInInf判断缓存数据处理是否到位的，因此m_SBlockInInf变量必须在缓存就绪后赋值
	//缓存信息首次加载
	SimDIMS SDBlckInfoTemp;
	if (m_bThreadAbnormal || 
		m_MgrIn == NULL		//说明close函数调用了
		)
	{
		m_iCurrentBlockNo = -1;
		return false;
	}
	if (!m_bIOThreadRun && m_bNeedsDataNow )
	{
		if (m_SBlockInInf.band == -1)
		{
			//为保证读取数据始终正确，必须先给缓存块赋值，再给缓存信息赋值，
			//由于数据流始终都是在前进的，因为即使缓存信息暂时没有对上，算法读取的时候也不会需要那些过时的数据了
			getBlockInfoByIndex(m_iCurrentBlockNo, &SDBlckInfoTemp);
#ifdef _LXDEBUG
			fprintf(ptf, "band:%d, start line:%d\n", SDBlckInfoTemp.band, SDBlckInfoTemp.Ystart);
#endif
			//开启IO线程锁
			m_bIOThreadRun = true;
			m_MgrIn->GetDIMSDataByBlockInfo(&SDBlckInfoTemp, &m_DimsIn, m_bBlockIn);
			m_bIOThreadRun = false;
			//现在才为缓存标志信息赋值
			m_SBlockInInf = SDBlckInfoTemp;
			m_iCurrentBlockNo++;
			//两个缓存块都已经装载数据
			m_bNeedsDataNow = false;
		}
		//m_iCurrentBlockNo计数越界，遍历输入图像完毕
		if (m_iCurrentBlockNo == m_iBlockNum)
		{
			m_iCurrentBlockNo = -1;
			return false;
		}
		//不休眠
		return -1;
	}
	return true;
}

int HAPBEGThread::blockOutToDiskData()
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
			m_MgrOut->WriteBlock(m_bBlockOut2, m_outBufferSize2 * m_nPtLengthOut);
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
			m_MgrOut->WriteBlock(m_bBlockOut, m_outBufferSize * m_nPtLengthOut);
			//释放IO线程锁
			m_bIOThreadRun = false;
			m_iCurrentOutBlockNo = -1;
			//不休眠
			return false;
		}
	}
	return true;
}