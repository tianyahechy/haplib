#pragma once
#include "HAPBEGTBase.h"
USING_NAMESPACE(HAPLIB)
//Ӱ���Զ��ֿ��д�ӿ�ģ�飬���ö��̷߳�ʽʵ��IO��������߳�ͬ����
//���������ηֿ飬������У���ʹ�ö��̣߳������㷨�����������ɶ�ȡ���������
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
	//ͨ�����в��κŻ�ȡͼ��鷶Χ��Ϣ���޷���ȡ��Χ
	bool getBlockInfo(long column, long line, long band, SimDIMS * pSBlockInfo);
	//ͨ�����в��κŻ�ȡͼ�����
	int GetBlockIndex(long column, long line, long band);
	//����д���̣߳�ִ�й�����������̻߳���
	//�ڶ��߳��и������������ֵ���ܱ�����Ա�������ⲿ�޷�����
	static void independentInThread(void * dummy);
	int DiskDataToBlockIn();
	//��������̣߳�ִ�й�������д���̻߳���
	static void IndependentOutThread(void * dummy);
	int blockOutToDiskData();
	//���г���
	int m_QueueLength;
	//�����黺������Ϣ��ʹ�ö���ָ�룬��֤�ڶ��н�������ַ�û�ʱ�ĸ�Ч��
	SimDIMS** m_ppSBlockInInf;
	//�����黺���������ڼ���
	BYTE * m_pbBlockIn;
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
	int m_iCurrentBlockNo;
	//ĿǰIOд�̶߳�ȡ��ͼ��飬Ҳ�Ƕ��߳�ִ�еı�ǣ���0��ʼ������-1��ʾִ�����
	int m_iCurrentOutBlockNo;
	//ͼ������������ڿ����߳̽���
	int m_iBlockNum;
	int m_iBlockNumOut;
	//����ͼ��IO��ȡ�߳�Ŀǰ���Զ�����
	bool m_bNeedsDataNow;

};
