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

bool HAPBEGThreadW::Set_MgrIn(CGDALFileManager * mgr, DIMS dims, int iBlockOverlap)
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
	m_MgrIn = mgr;
	//判断dims是否初始化
	if (
		mgr->m_header.m_nLines < dims.yEnd ||
		mgr->m_header.m_nSamples < dims.xEnd ||
		dims.band < mgr->m_header.m_nBands
		)
	{
		strcpy(pcErrMessage, "DIMS设置越界!");
		return false;
	}
	m_DimsIn = dims;
	
	//目前HAPBEGThreadW调用时，设定读、写双缓存均可动态分配，故在文件载入函数(Set_MgrIn,Set_MgrOut)里开辟缓存
	//可反复调用set_MgrIn设置读取图像，动态分配缓存支持模式有：
	//读图像、无写图像模式（如统计直方图）
	//读图像，写图像模式（常规处理）
	//无图像读取，写出图像模式（依据数学算法创建图像）
	//无读图像，无写图像模式（内部函数测试，一般很少用）
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
	//缓存分配，由于部分算法需要读取图像，缓存开辟不适宜在read函数中完成
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
		strcpy(pcErrMessage, "输出文件路径必须输入！");
		m_bThreadAbnormal = true;
		return false;
	}
	if (!m_MgrOut->HpsSaveImageAs(fileName))
	{
		strcpy(pcErrMessage, "输出文件读取出错！");
		//关闭读写线程
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
			//置换1，2缓冲区指针和simDIMS变量
			BYTE * pBBlockInTemp = m_bBlockIn;
			m_bBlockIn = m_bBlockIn2;
			m_bBlockIn2 = pBBlockInTemp;

			//理论上支持乱序读取，两块缓存指针换来换去，但不推荐这样使用，还是设置overlap
			SimDIMS pSDBlockInfoTemp = m_SBlockInInf;
			m_SBlockInInf = m_SBlockInInf2;
			m_SBlockInInf2 = m_SBlockInInf;

			return m_bBlockIn + (band *m_SBlockInInf.bandSize + line * m_SBlockInInf.xWidth + column - m_SBlockInInf.bandOffset) * m_nPtLength;
		}
		else
		{
			if (m_bNeedsDataNow == true)
			{
				//IO读线程已经结束或者准备开始
				if (m_iCurrentBlockNo == -1)
				{
					m_SBlockInInf.initDIMS();
					m_SBlockInInf2.initDIMS();
					m_bThreadAbnormal = false;
					m_iCurrentBlockNo = GetBlockIndex(column, line, band);
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
				m_iCurrentBlockNo = GetBlockIndex(column, line, band);
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
			//初始化缓存块信息
			GetBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
			m_outBufferSize = m_SBlockOutInf.wGetCacheSize();
			_beginthread(IndependentOutThread, 0, (void*) this);
		}
		else
		if (m_outBufferSize2 == 0) //表示2号缓存区没有写执行任务
		{
			//置换1，2缓冲区指针和simDIMS变量
			BYTE * pBBlockOutTemp = m_bBlockOut;
			m_bBlockOut = m_bBlockOut2;
			m_bBlockOut2 = pBBlockOutTemp;
			//可以执行写任务了，（该变量是写线程的控制变量，尽量晚些赋值）
			m_outBufferSize2 = m_outBufferSize;
			m_iCurrentOutBlockNo++;
			//初始化缓存块信息
			GetBlockInfoByIndex(m_iCurrentOutBlockNo, &m_SBlockOutInf, false);
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

void HAPBEGThreadW::Close_MgrIn()
{
	//等待读取数据任务完毕
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
//通过图像块索引号获得缓存块包含的行数
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
		//每块拥有的行数
		int iLinesPerBlock = g_BlockSize / (m_nPtLength * iCol * iBand);	
		//缓存块比单波段数据量大
		if (iRow <= iLinesPerBlock)
		{
			return false;
		}
		else
		if (m_iBlockOverlap == 0)
		{
			int iNumOfBlock = int(iRow * 1.0 / iLinesPerBlock + 0.9999);
			//考虑最后一块的行数可能小一些
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
			//考虑最后一块的行数可能小一些
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

//通过图像块索引号获取缓存块信息
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
		//每块拥有的行数
		int iLinesPerBlock = g_BlockSize / (m_nPtLength * iCol * iBand);
		//缓存块比单波段数据量大
		if (iLinesPerBlock >= iRow)
		{
			pSBlockInfo->yStart = 0;
			pSBlockInfo->yEnd = iRow;
		}
		else if (m_iBlockOverlap == 0)
		{
			int iNumOfBlock = int(iRow * 1.0 / iLinesPerBlock + 0.9999);
			pSBlockInfo->yStart = iIndex * iLinesPerBlock;
			//考虑最后一块的行数可能小一些
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
				//存在缓冲区重叠情况
				pSBlockInfo->yStart = iIndex * (iLinesPerBlock - m_iBlockOverlap);
				//考虑最后一块的行数可能小一些
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
		//每块拥有的行数
		int iLinesPerBlock = g_BlockSizeW / (iDT * iCol * iBand);
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

	}
	//设置块高度宽度
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
	//每块拥有的行数
	int iLinesPerBlock = g_BlockSize / (iDT * iCol * iBand);
	int iIndex = -1;
	//缓存块比单波段数据量大
	if (iLinesPerBlock >= iRow)
	{
		//index从0开始计数，只能达到iBlockNum-1
		iIndex = 0;
	}
	else if (m_iBlockOverlap == 0)
	{
		//line行所在的块数，0开始
		iIndex = line / iLinesPerBlock;
	}
	else
	{
		//iCurrentBlockNo从0开始计数，只够达到iBlockNum-1
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

//获取图像块总数
int HAPBEGThreadW::getBlockNumber(bool inOrOut)
{
	return (inOrOut == true) ? m_iBlockNum : m_iBlockNumOut;
}
//设置图像块总数
void HAPBEGThreadW::setBlockNumber(bool inOrOut)
{
	//square模式只需输出时计算index
	if (inOrOut)
	{
		int iRow = m_DimsIn.getHeight();
		int iCol = m_DimsIn.getWidth();
		int iBand = m_DimsIn.getDIMSBands();
		int iDT = m_nPtLength;
		//每块拥有的行数
		int iLinesPerBlock = g_BlockSize / (iDT * iCol * iBand);
		int iIndex = -1;
		//缓存块比单波段数据量大
		if (iLinesPerBlock >= iRow || m_iBlockOverlap == 0)
		{
			//dims总块数
			m_iBlockNum = int(iRow * 1.0 / iLinesPerBlock + 0.9999);
		}
		else
		{
			//缓冲区重叠模式
			//重叠区域重新计算
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
		//g_BlockSize必须大于iCol*m_nPtLength,缓存大小设置的时候注意
		//每块拥有的行数
		int iLinesPerBlock = g_BlockSizeW / (iDT * iCol);
		//dims总块数
		m_iBlockNumOut = int(iRow * 1.0 / iLinesPerBlock + 0.99999999);	
	}

}
int HAPBEGThreadW::DiskDataToBlockIn()
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
	if (!m_bIOThreadRun && m_bNeedsDataNow)
	{
		if (m_SBlockInInf.band == -1)
		{
			//为保证读取数据始终正确，必须先给缓存块赋值，再给缓存信息赋值，
			//由于数据流始终都是在前进的，因为即使缓存信息暂时没有对上，算法读取的时候也不会需要那些过时的数据了
			GetBlockInfoByIndex(m_iCurrentBlockNo, &SDBlckInfoTemp);
			//开启IO线程锁
			m_bIOThreadRun = true;
			m_MgrIn->WGetDIMSDataByBlockInfo(&SDBlckInfoTemp, &m_DimsIn, m_bBlockIn);
			//释放IO线程锁
			m_bIOThreadRun = false;
			//现在才为缓存标志信息赋值
			m_SBlockInInf = SDBlckInfoTemp;
			m_iCurrentBlockNo++;
		}
		else
		{
			//磁盘重复拷贝重叠区域，更快，胜出!
			GetBlockInfoByIndex(m_iCurrentBlockNo, &SDBlckInfoTemp);
			//开启IO线程锁
			m_bIOThreadRun = true;
			m_MgrIn->WGetDIMSDataByBlockInfo(&SDBlckInfoTemp, &m_DimsIn, m_bBlockIn2);
			//释放IO线程锁
			m_bIOThreadRun = false;
			//现在才为缓存标志信息赋值
			m_SBlockInInf2 = SDBlckInfoTemp;
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

int HAPBEGThreadW::blockOutToDiskData()
{
	//线程异常或线程结束的条件满足
	if (m_bThreadAbnormal || m_MgrOut == NULL)
	{
		m_iCurrentBlockNo = -1;
		return false;
	}
	if (!m_bIOThreadRun)
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