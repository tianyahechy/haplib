#pragma once
#include "HAPLib.h"
USING_NAMESPACE(HAPLIB)
#pragma warning(disable:4996)

class HAPCLASS CCacheQueue
{
public:
	 CCacheQueue();
	~ CCacheQueue(); //��������
	BYTE * requestCache(DIMS * &pSBlockInf);
	//����һ��������
	static BYTE * m_bBlockIn;
	//�������������
	static BYTE * m_bBlockIn2;
	//���������
	static BYTE * m_bBlockOut;
	//����һ����������Ϣ
	DIMS m_SBlockInInf;
	//���������������Ϣ
	DIMS m_SBlockInInf2;
	//�����������Ϣ
	DIMS m_SBlockOutInf;

protected:
	//�����һ�Ż��������ݼ�������
	int m_iFlag;
};
