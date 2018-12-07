#pragma once
#include "HAPBEGTBase.h"
USING_NAMESPACE(HAPLIB)
//Ӱ���Զ��ֿ��д�ӿ�ģ�飬���ö��̷߳�ʽʵ��IO��������߳�ͬ����
//���������ηֿ飬������У������ö��̣߳������㷨�����������ɶ�ȡ���������
//д�������̣߳�˫���棬���зֿ�
class HAPCLASS HAPBEGThreadS : public HAPBEGTBase
{
public:
	HAPBEGThreadS();
	virtual ~HAPBEGThreadS();
	//���ö��ļ�ָ��
	bool Set_MgrIn(CGDALFileManager * mgr, DIMS dims = g_Zdims, int iBlockOverlap = 0);
	//����д�ļ�ָ��
	bool Set_MgrOut(CGDALFileManager * mgr, const char* fileName);
	//�ر�дָ��
	void Close_MgrOut();
	//�رն�ָ��
	void Close_MgrIn();
	//ʹ�ö���IO�߳��Ż��㷨��ǰ�᣺
	//�û�����֪���㷨��ȡͼ�����ݵ�˳����Ϊ�����֧��˳���ȡ
	//Ĭ�ϰ��ж�ȡ
	void * Read(long column, long line, long band) ;
	//˳��д������
	bool Write(void * pValue);
protected:
	//ͨ��ͼ��������Ż�ȡ�������Ϣ
	void GetBlockInfoByIndex(
		int iIndex,				//������ţ���0��ʼ����
		SimDIMS* pSBlockInfo,	//���صĻ������Ϣ�����ȴ�������
		bool InOrOut = true);
	//��ȡͼ�������
	void setBlockNumber(bool InOrOut = false);
	//ͨ�����в��κŻ�ȡͼ�����
	int GetBlockIndex(long column, long line, long band, SimDIMS * pSBlockInfo);
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
private:


};
