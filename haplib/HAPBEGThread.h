//Ӱ���Զ����зֿ��д�ӿ�ģ�飬���ö��̷߳�ʽʵ��IO��������߳�ͬ��
//�������зֿ飬�����̣߳�˫����
//д�������̣߳�˫����
#pragma once 
#include "HAPBEGTBase.h"

USING_NAMESPACE(HAPLIB)

class HAPCLASS HAPBEGThread : public HAPBEGTBase
{
public:
	HAPBEGThread();
	virtual ~HAPBEGThread();

private:
	//���ö��ļ�ָ��
	bool Set_MgrIn(CGDALFileManager * mgr, DIMS dims, int iBlockOverlap = 0);
	//����д�ļ�ָ��
	bool Set_MgrOut(CGDALFileManager * mgr, const char* fileName) = 0;
	//�ر�дָ��
	void Close_MgrOut();
	//�رն�ָ��
	void Close_MgrIn();
	//ʹ�ö���IO�߳��Ż��㷨��ǰ��
	//�û�����֪���㷨��ȡͼ�����ݵ�˳�������֧�ְ��ж�ȡ
	void * Read(long column, long line, long band);
	//˳��д������
	bool Write(void * pValue);

protected:
	//ͨ��ͼ��������Ż�ȡ�������Ϣ
	//����������ţ���ȡ��Ӧ�������Ϣ
	void getBlockInfoByIndex(
		int iIndex,				//������ţ���0��ʼ����
		SimDIMS * pSBlockInfo,	//���صĻ�����Ϣ�����ȴ�������
		bool InOrOut = true
		);
	//��ȡ����ͼ������ͼ��ֿ�����
	void setBlockNumber(bool inOrOut = true);
	//ͨ�����в��κŻ�ȡͼ�����
	int getBlockIndex(long column, long line, long band);
	//����д���̣߳�ִ�й�����������̻߳���
	//�ڶ��߳��и������������ֵ���ܱ�����Ա�������ⲿ�޷�����
	static void independentInThread(void * dummy);
	int DiskDataToBlockIn();
	//��������̣߳�ִ�й�������д���̻߳���
	static void IndependentOutThread(void * dummy);
	int blockOutToDiskData();
	//����һ����������Ϣ
	SimDIMS m_SBlockInInf;
	//���������������Ϣ
	SimDIMS m_SBlockInInf2;
	//����һ�������������ڼ���
	BYTE * m_bBlockIn;
	//������������������ڴ����ڴ潻��
	BYTE * m_bBlockIn2;
	//д���ڴ滺������һ��������
	BYTE * m_bBlockOut;
	//������̻�����������������
	BYTE * m_bBlockOut2;
	//���뻺������С����ʱû��
	int m_InBufferSize;
	//���һ����������С��Ҳ�ǻ�����״̬���
	int m_outBufferSize;
	//���������������С��Ҳ�ǻ�����״̬���
	int m_outBufferSize2;
	//�Ѿ��������Ԫ����һ��д������״̬��ǣ�-1��ʾ������������Ч״̬
	int m_WriteDoneCount;
	//��������̵Ļ�������Ϣ
	SimDIMS m_SBlockOutInf;
	//ĿǰIO��ȡ��ͼ��飬Ҳ�Ƕ��߳�ִ�еı�ǣ���0��ʼ������-1��ʾִ��
	int m_iCurrentBlockNo;
	//ĿǰIOд�̶߳�ȡ��ͼ��飬Ҳ�Ƕ��߳�ִ�еı�ǣ���0��ʼ������-1��ʾִ�����
	int m_iCurrentOutBlockNo;
	//ͼ������������ڿ����߳̽���
	int m_iBlockNum;
	int m_iBlockNumOut;
	//����ͼ��IO��ȡ�߳�Ŀǰ��Ҫ������
	bool m_bNeedsDataNow;
};
