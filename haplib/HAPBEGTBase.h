//Ӱ���Զ��ֿ��д�ӿ�ģ����࣬���ö��̷߳�ʽʵ��IO,������߳�ͬ��
#pragma once
#include "GDALFileManager.h"

USING_NAMESPACE(HAPLIB)

class  HAPCLASS HAPBEGTBase
{
public:
	 HAPBEGTBase();
	virtual ~HAPBEGTBase();
	//���ö��ļ�ָ��
	virtual bool Set_MgrIn(CGDALFileManager * mgr, DIMS dims = g_Zdims, int iBlockOverlap = 0) = 0;
	//����д�ļ�ָ��
	virtual bool Set_MgrOut(CGDALFileManager * mgr, const char* fileName) = 0;
	//�ر�дָ��
	virtual void Close_MgrOut() = 0;
	//�رն�ָ��
	virtual void Close_MgrIn() = 0;
	//Ĭ�ϰ��ж�ȡ
	virtual void * Read(long column, long line, long band) = 0;
	//�麯����ò�ҪΪ��������
	inline void * ReadL(long column, long line, long band);
	//˳��д������
	virtual bool Write(void * pValue) = 0;
	//����DIMS��Ϣ
	DIMS m_DimsIn;
	//���������ʾ��Ϣ�����ϲ����
	char pcErrMessage[64];
private:
	//�����ļ�����ָ��
	CGDALFileManager * m_MgrIn;
	//����ļ�����ָ��
	CGDALFileManager * m_MgrOut;
	//�����ֽڳ���
	int m_nPtLength;
	//������ݳ���
	int m_nPtLengthOut;
	//0�������ص������ڣ�1-3������������1-3�������ص�������n*n��������ģ����ֵ�˲�������Ҫn-1�������ص�����֤����齻������ȱ�����ݡ�
	//��nΪż��������Ҫn�С��ص�����Ϊż�����������������λ�ü��㣬���Զ԰��
	//�������û��������ص�������ͼ�����㷨��Ҫ�õ������˲�����ֵ�㷨��
	int m_iBlockOverlap;
	//������Ϣ������ʾ�û�һ��
	bool m_bAlarmInfo;
	//�߳��쳣��ǿ�ȸ����߳��˳�
	bool m_bThreadAbnormal;
	//���ͼ��IO�߳����ڹ�������֪�����̲߳�Ҫ���д���IO����
	static bool m_bIOThreadRun;
	//���������Ϣ
	FILE * ptf;

};