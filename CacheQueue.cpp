#include "CacheQueue.h"

CCacheQueue::CCacheQueue()
{

}
CCacheQueue::CCacheQueue()
{

}
//����һ��������
BYTE * CCacheQueue::m_bBlockIn = NULL;
//�������������
BYTE * CCacheQueue::m_bBlockIn2 = NULL;
//���������
BYTE * CCacheQueue::m_bBlockOut = NULL;

BYTE * CCacheQueue::requestCache(DIMS* &psBlockInf)
{
	//����δ����
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

	//�����ѷ���
	if ( m_iFlag == 1)
	{
		//��ʾ2�Ż��������ݼ�������
		m_iFlag = 2;
		psBlockInf = &m_SBlockInInf;
		return m_bBlockIn;
	}
	else
	{
		//��ʾ1�Ż��������ݼ�������
		m_iFlag = 1;
		psBlockInf = &m_SBlockInInf2;
		return m_bBlockIn2;
	}
	return CCacheQueue::m_bBlockIn;
}