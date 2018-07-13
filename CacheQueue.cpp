#include "CacheQueue.h"

CCacheQueue::CCacheQueue()
{

}
CCacheQueue::CCacheQueue()
{

}
//输入一级缓冲区
BYTE * CCacheQueue::m_bBlockIn = NULL;
//输入二级缓冲区
BYTE * CCacheQueue::m_bBlockIn2 = NULL;
//输出缓冲区
BYTE * CCacheQueue::m_bBlockOut = NULL;

BYTE * CCacheQueue::requestCache(DIMS* &psBlockInf)
{
	//缓存未分配
	if ( !m_bBlockIn )
	{
		m_bBlockIn = new BYTE[g_BlockSize];
		psBlockInf = &m_SBlockInInf;
		m_iFlag = 1;
		return CCacheQueue::m_bBlockIn;
	}

	if ( !m_bBlockIn2 )
	{
		m_bBlockIn2 = new BYTE[g_BlockSize];
		psBlockInf = &m_SBlockInInf2;
		m_iFlag = 2;
		return m_bBlockIn2;
	}

	//缓存已分配
	if ( m_iFlag == 1)
	{
		//表示2号缓冲区数据即将丢弃
		m_iFlag = 2;
		psBlockInf = &m_SBlockInInf;
		return m_bBlockIn;
	}
	else
	{
		//表示1号缓冲区数据即将丢弃
		m_iFlag = 1;
		psBlockInf = &m_SBlockInInf2;
		return m_bBlockIn2;
	}
	return CCacheQueue::m_bBlockIn;
}