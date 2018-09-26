#include "MyTimer.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//全局计时工具
CMyTimer g_CMT;

CMyTimer::CMyTimer()
{
	time(&m_time0);
	m_clock0 = clock();
	_ftime(&m_tstruct0);

	m_lSec = 0;
	m_lMiliSec = 0;
	strcpy(pcAlgoStatus, "");
	progPo = 0;
}

CMyTimer::~CMyTimer()
{

}

int CMyTimer::getDistantTime()
{
	time_t time1;
	time(&time1);

	int sec = int(time1 - m_time0);
	return sec;
}

int CMyTimer::getDistantClock()
{
	clock_t clock1 = clock();
	int milSec = clock1 - m_clock0;
	return milSec;
}

char * CMyTimer::getCurTimeString()
{
	char s[20];
	char dbuffer[9];
	char tbuffer[9];
	_strdate(dbuffer);
	_strtime(tbuffer);
	sprintf(s, "%s, %s", tbuffer, dbuffer);
	return s;
}

void CMyTimer::getDistantTime(int& minute, int& sec, int& miliSec)
{
	_timeb tstruct1;
	_ftime(&tstruct1);
	sec = int(tstruct1.time - m_tstruct0.time);
	miliSec = tstruct1.millitm - m_tstruct0.millitm;
	if (miliSec)
	{
		sec--;
		miliSec = 1000 + miliSec;
	}
	else
	if (miliSec > 1000)
	{
		sec += miliSec / 1000;
		miliSec %= 1000;
	}
	minute = sec / 60;
	sec = sec % 60;
}

void CMyTimer::getDistantClockMSM(int milSec, int& minute, int& sec, int& leftMiliSec)
{
	minute = (milSec / 1000) / 60;
	sec = milSec / 1000 - minute * 60;
	leftMiliSec = milSec - (sec + minute * 60) * 1000;
}

void CMyTimer::beginCountTime()
{
	_ftime(&m_tstruct0);
	m_lSec = 0;
	m_lMiliSec = 0;
}

void CMyTimer::continueCountTime()
{
	_ftime(&m_tstruct0);
}

void CMyTimer::pauseCountTime()
{
	_timeb tstruct1;
	_ftime(&tstruct1);

	m_lSec += tstruct1.time - m_tstruct0.time;
	m_lMiliSec += tstruct1.millitm - m_tstruct0.millitm;
	if (m_lMiliSec < 0 )
	{
		m_lSec--;
		m_lMiliSec += 1000;
	}
	else if (m_lMiliSec > 1000)
	{
		m_lSec += m_lMiliSec / 1000;
		m_lMiliSec %= 1000;
	}
}

void CMyTimer::getSUMTime(int& minute, int& sec, int& milSec)
{
	minute += m_lSec / 60;
	sec = m_lSec % 60;
	milSec = m_lMiliSec;
}