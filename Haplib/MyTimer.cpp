#include "MyTimer.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//全局计时工具，
CMyTimer g_CMT;

CMyTimer::CMyTimer()
{
	time(&m_time0);
	m_clock0 = clock();
	_ftime(&m_tStruct0);
	m_lSec = 0;
	m_lMiliSec = 0;
	strcpy(pcAlgoStatus, "");
	ProgPo = 0;
}

CMyTimer::~CMyTimer()
{

}
int CMyTimer::GetDistanceTime()
{
	time_t time1;
	time(&time1);
	int sec = int(time1 - m_time0);
	return sec;
}

int CMyTimer::GetDistantClock()
{
	clock_t clock1 = clock();
	int milSec = clock1 - m_clock0;
	return milSec;
}

void CMyTimer::GetCurTimeString(char* s)
{
	char dbuffer[9];
	char tbuffer[9];
	_strdate(dbuffer);
	_strtime(tbuffer);
	sprintf(s, "%s,%s", tbuffer, dbuffer);
}

void CMyTimer::GetDistantTime(int& Minute, int& sec, int& milliSec)
{
	_timeb tstruct1;
	_ftime(&tstruct1);
	sec = int(tstruct1.time - m_tStruct0.time);
	milliSec = tstruct1.millitm - m_tStruct0.millitm;
	if (milliSec < 0)
	{
		sec--;
		milliSec = 1000 + milliSec;
	}
	else if (milliSec > 1000)
	{
		sec += milliSec / 1000;
		milliSec %= 1000;
	}
	Minute = sec / 60;
	sec = sec % 60;
}

void CMyTimer::GetDistantClockMSM(int milSec, int& minute, int&sec, int&leftMiliSec)
{
	minute = (milSec / 1000) / 60;
	sec = milSec / 1000 - minute * 60;
	leftMiliSec = milSec - (sec + minute * 60) * 1000;
}

void CMyTimer::BeginCountTime()
{
	_ftime(&m_tStruct0);
	m_lSec = 0;
	m_lMiliSec = 0;
}

void CMyTimer::ContinueCountTime()
{
	_ftime(&m_tStruct0);
}
void CMyTimer::PauseCountTime()
{
	_timeb tstruct1;
	_ftime(&tstruct1);

	m_lSec += tstruct1.time - m_tStruct0.time;
	m_lMiliSec += tstruct1.millitm - m_tStruct0.millitm;
	if (m_lMiliSec < 0)
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

void CMyTimer::GetSumTime(int& minute, int& sec, int& milsec)
{
	minute = m_lSec / 60;
	sec = m_lSec % 60;
	milsec = m_lMiliSec;
}