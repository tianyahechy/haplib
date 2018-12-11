#include "HAPBEGTBase.h"
#include <process.h>

bool HAPBEGTBase::m_bIOThreadRun = false;

HAPBEGTBase::HAPBEGTBase()
{
	m_MgrIn = NULL;
	m_MgrOut = NULL;
	m_bThreadAbnormal = false;
	m_iBlockOverlap = 0;
	//�����ֽڳ��ȣ�
	m_nPtLength = 1;
	//������ݳ���
	m_nPtLengthOut = 1;
	strcpy(pcErrMessage, "");
#ifdef _LXDEBUG
	if (ptf)
	{
		ptf = fopen("C:\\TraceFile2.txt", "w");
	}
#endif
}

HAPBEGTBase::HAPBEGTBase()
{
#ifdef _LXDEBUG
	if (ptf)
	{
		fclose(ptf);
	}
	ptf = NULL;
#endif
}

void * HAPBEGTBase::ReadL(long column, long line, long band)
{
	//���þ���ʽ�������Խ��
	if (column < 0)
	{
		column = -column;
	}
	if (column >= m_DimsIn.xWidth)
	{
		column = 2 * m_DimsIn.xWidth - column - 1;
	}
	if (line < 0)
	{
		line = -line;
	}
	if ( line >= m_DimsIn.yHeight )
	{
		line = 2 * m_DimsIn.yHeight - line - 1;
	}
	//���ڻ���û�ж���Read�������壬����ReadLʱ���Զ����ü̳����Read�������������̳��಻���ٶ���ReadL
	return Read(column, line, band);
}
