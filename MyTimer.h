#include "HAPLib.h"
#include <sys/timeb.h>
#include <time.h>
#include <string>
using namespace std;

#pragma once
#pragma warning( disable: 4996)

#if _MSC_VER >= 1000
#pragma once
#endif

USING_NAMESPACE(HAPLIB)

class  HAPCLASS CMyTimer
{
public:
	  CMyTimer();
	~ CMyTimer();
	char * getCurTimeString();

	//获取秒
	int getDistantTime();
	//获取毫秒
	int getDistantClock();
	//获取分、秒、毫秒
	void getDistantTime(int& minute, int& sec, int& miliSec);
	//秒到分秒的转换
	static void getDistantClockMSM(int milSec, int& minute, int& sec, int& leftMiliSec);
	//开始计时
	void beginCountTime();
	//继续计时
	void continueCountTime();
	//暂停计时
	void pauseCountTime();
	//获取计时累计时间
	void getSUMTime(int& milSec, int& minute, int& sec); 
	//进度条指示位置，0-100之间吗，-1表示进度条结束，为10的倍数的时候可以更新进度条文字信息
	int progPo;
	//进度条上算法执行状态提示信息
	char pcAlgoStatus[256];

protected:
	time_t m_time0;
	clock_t m_clock0;
	_timeb m_tstruct0;
	//内部计时变量，用于统计分段计时
	long m_lSec;
	long m_lMiliSec;

private:

};

//全局计时工具
extern HAPCLASS CMyTimer g_CMT;