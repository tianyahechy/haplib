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
	//��ȡ��
	int GetDistanceTime();
	//��ȡ����
	int GetDistantClock();
	//��ȡ�֡��롢����
	void GetDistantTime(int& Minute, int& Sec, int& MiliSec);
	//�뵽�����ת��
	static void GetDistantClockMSM(int milsec, int& Minute, int& Sec, int& LeftMiliSec);
	//��ʼ��ʱ
	void BeginCountTime();
	//������ʱ
	void ContinueCountTime();
	//��ͣ��ʱ
	void PauseCountTime();
	//��ȡ��ʱ�ۼ�ʱ��
	void GetSumTime(int& milSec, int& Minute, int& Sec);
	//������ָʾλ�ã�0-100֮�䣬-1����ʾ������������Ϊ10�ı�����ʱ����Ը��½�����������Ϣ
	int ProgPo;
	//���������㷨ִ��״̬��ʾ��Ϣ
	char pcAlgoStatus[256];

protected:
	time_t m_time0;
	clock_t m_clock0;
	_timeb m_tStruct0;
	//�ڲ���ʱ����
	long m_lSec;
	long m_lMiliSec;

};
//ȫ�ּ�ʱ����
extern HAPCLASS CMyTimer g_CMT;
