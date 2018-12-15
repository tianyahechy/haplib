#pragma once
#include "HAPBEGTBase.h"
USING_NAMESPACE(HAPLIB)
//Ӱ���Զ��ֿ��д�ӿ�ģ�飬���ö��̷߳�ʽʵ��IO�������ζ�ȡ��������߳�ͬ����W��wave
//�������������зֿ飬�����̣߳�˫����
//д�������̣߳�˫���棬���������ηֿ����.д��ʱ����for���Σ���for����
class HAPCLASS HAPBEGThreadW : public HAPBEGTBase
{
public:
	HAPBEGThreadW();
	virtual ~HAPBEGThreadW();
	//���������ֻ��д��.h�ļ���
	HAPBEGThreadW& operator= (const HAPBEGThreadW& otherImgIO)
	{
		if (this == &otherImgIO)
		{
			return *this;
		}
		memcpy(this, &otherImgIO, sizeof(HAPBEGThreadW));
		//ֻ���������
		m_bBlockIn = new BYTE[g_BlockSize];
		m_bBlockIn2 = new BYTE[g_BlockSize];
		//˫���������ʱ���ã���ʽ
		m_bBlockOut = new BYTE[10];
		m_bBlockOut2 = new BYTE[10];
		return *this;
	}
	//���ö��ļ�ָ��
	bool Set_MgrIn(CGDALFileManager * mgr, DIMS dims, int iBlockOverlap = 0);
	//����д�ļ�ָ��
	bool Set_MgrOut(CGDALFileManager * mgr, const char* fileName);
	//�ر�дָ��
	void Close_MgrOut();
	//�رն�ָ��
	void Close_MgrIn();
	//��Ϊ���г�Ա�������˼������
	//ͨ�����в��κŻ�ȡͼ�����
	int GetBlockIndex(long column, long line, long band);
	//��ȡͼ�������
	int getBlockNumber(bool InOrOut);
	int getBlockLinesByIndex(int iIndex, bool InOrOut);
	//ʹ�ö���IO�߳��Ż��㷨��ǰ�᣺
	//�û�����֪���㷨��ȡͼ�����ݵ�˳����Ϊ�����֧��˳���ȡ
	//Ĭ�ϰ��ж�ȡ
	void * Read(long column, long line, long band) ;
	//BSQ������˳��д������
	bool Write(void * pValue);
protected:
	//ͨ��ͼ��������Ż�ȡ�������Ϣ
	void GetBlockInfoByIndex(
		int iIndex,				//������ţ���0��ʼ����
		SimDIMS* pSBlockInfo,	//���صĻ������Ϣ�����ȴ�������
		bool InOrOut = true);
	//��ȡͼ�������
	void setBlockNumber(bool InOrOut = false);
	//����д���̣߳�ִ�й�����������̻߳���
	//�ڶ��߳��и������������ֵ���ܱ�����Ա�������ⲿ�޷�����
	static void independentInThread(void * dummy);
	int DiskDataToBlockIn();
	//��������̣߳�ִ�й�������д���̻߳���
	//��д�߳��и���д������������ܱ�����Ա�������ⲿ�޷�����
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
	//0:�������ص������ڣ�1-3������������1-3�������ص�������n*n��������ģ����ֵ�˲�������Ҫ��n-1�������ص�����֤����齻������ȱ�����ݡ�
	//��nΪż��������Ҫn��
	//�ص�����Ϊż�����������������λ�ü��㣬���Զ԰��
	//���ǰm_QueueLengthͼ�����
	//ĿǰIO���̶߳�ȡ��ͼ��飬Ҳ�Ƕ��߳�ִ�еı�ǣ���0��ʼ������-1��ʾִ�����
	int m_iCurrentBlockNo;
	//ĿǰIOд�̶߳�ȡ��ͼ��飬Ҳ�Ƕ��߳�ִ�еı�ǣ���0��ʼ������-1��ʾִ�����
	int m_iCurrentOutBlockNo;
	//ͼ������������ڿ����߳̽���
	int m_iBlockNum;
	int m_iBlockNumOut;
	//����ͼ��IO��ȡ�߳�Ŀǰ���Զ�����
	bool m_bNeedsDataNow;

};
