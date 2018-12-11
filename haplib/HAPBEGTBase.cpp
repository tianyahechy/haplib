#include "HAPBEGTBase.h"
#include <process.h>

bool HAPBEGTBase::m_bIOThreadRun = false;

HAPBEGTBase::HAPBEGTBase()
{
	m_MgrIn = NULL;
	m_MgrOut = NULL;
	m_bThreadAbnormal = false;
	m_iBlockOverlap = 0;
	//数字字节长度，
	m_nPtLength = 1;
	//输出数据长度
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
	//采用镜像方式避免访问越界
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
	//由于基类没有定义Read函数定义，调用ReadL时会自动调用继承类的Read函数，这样，继承类不用再定义ReadL
	return Read(column, line, band);
}
