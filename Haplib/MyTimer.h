#pragma once
#include "haplib.h"
#include <sys\timeb.h>
#include <time.h>
#include <string>
using namespace std;
#pragma warning(disalbe:4996)

USING_NAMESPACE(HAPLIB)
class HAPCLASS CMyTimer
{
public:
	 CMyTimer();
	virtual~ CMyTimer();
	void GetCurTimeString(char* s);
	//获取秒
	int GetDistanceTime();
	//获取毫秒
	int GetDistantClock();
	//获取分、秒、毫秒
	void GetDistantTime(int& Minute, int& Sec, int& MiliSec);
	//秒到分秒的转换
	static void GetDistantClockMSM(int milsec, int& Minute, int& Sec, int& LeftMiliSec);
	//开始计时
	void BeginCountTime();
	//继续计时
	void ContinueCountTime();
	//暂停计时
	void PauseCountTime();
	//获取计时累计时间
	void GetSumTime(int& milSec, int& Minute, int& Sec);
	//进度条指示位置，0-100之间，-1：表示进度条结束，为10的倍数的时候可以更新进度条文字信息
	int ProgPo;
	//进度条上算法执行状态提示信息
	char pcAlgoStatus[256];

protected:
	time_t m_time0;
	clock_t m_clock0;
	_timeb m_tStruct0;
	//内部计时变量
	long m_lSec;
	long m_lMiliSec;

};
//全局计时工具
extern HAPCLASS CMyTimer g_CMT;
