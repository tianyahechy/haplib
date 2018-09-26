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

	//��ȡ��
	int getDistantTime();
	//��ȡ����
	int getDistantClock();
	//��ȡ�֡��롢����
	void getDistantTime(int& minute, int& sec, int& miliSec);
	//�뵽�����ת��
	static void getDistantClockMSM(int milSec, int& minute, int& sec, int& leftMiliSec);
	//��ʼ��ʱ
	void beginCountTime();
	//������ʱ
	void continueCountTime();
	//��ͣ��ʱ
	void pauseCountTime();
	//��ȡ��ʱ�ۼ�ʱ��
	void getSUMTime(int& milSec, int& minute, int& sec); 
	//������ָʾλ�ã�0-100֮����-1��ʾ������������Ϊ10�ı�����ʱ����Ը��½�����������Ϣ
	int progPo;
	//���������㷨ִ��״̬��ʾ��Ϣ
	char pcAlgoStatus[256];

protected:
	time_t m_time0;
	clock_t m_clock0;
	_timeb m_tstruct0;
	//�ڲ���ʱ����������ͳ�Ʒֶμ�ʱ
	long m_lSec;
	long m_lMiliSec;

private:

};

//ȫ�ּ�ʱ����
extern HAPCLASS CMyTimer g_CMT;