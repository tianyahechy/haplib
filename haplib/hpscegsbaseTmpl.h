#pragma once

#include "GDALFileManager.h"
USING_NAMESPACE(HAPLIB)

//ͳһ����ͼ�����㷨ʹ�õĻ���
//ģ���ࡢģ�庯���ڱ�дʹ�õ�ʱ��һ����Ҫ��ʵ���������ܹ�ֱ���ҵ�ģ����Ķ��壬������ֻ�ҵ�����

class HAPCLASS HAPBEGBase
{
public:
	HAPBEGBase();
	virtual ~HAPBEGBase();

	CGDALFileManager * Get_MgrIn();
	CGDALFileManager * Get_MgrOut();

	//���ö��ļ�ָ��
	bool Set_MgrIn(CGDALFileManager * mgr, DIMS dims);
	//����д�ļ�ָ��
	bool Set_MgrOut(CGDALFileManager * mgr, const char * fileName);
	//�ر�дָ�뱣��
	void Close_MgrOut();
	//�رն�ָ�뱣��
	void Close_MgrIn();

	//ͨ��ͼ���С��в��κ�ֱ��ȷ���������ڵĿ�������Ϣ
	//���зֿ�
	void RGetBlockInfo(int col, int row, int band, SimDIMS * psBlockInfo);
	//���зֿ�
	void CGetBlockInfo(int col, int row, int band, SimDIMS * psBlockInfo);
	//�������ηֿ�
	void BGetBlockInfo(int col, int row, int band, SimDIMS * psBlockInfo);
	//�����ηֿ�
	void WGetBlockInfo(int col, int row, int band, SimDIMS * psBlockInfo);

	//������ָ�룬���ﰴ�տ���BSQ��ʽ����
	//Ĭ�ϰ��ж�ȡ
	virtual void * RRead(long column, long line, long band);
	//���ж�ȡ
	void * CRead(long column, long line, long band);
	//�����ȡBlock Read
	void * BRead(long column, long line, long band);
	//�����ζ�ȡ
	void * WRead(long column, long line, long band);

	//д����ֵ����������BSQ��bip����BIL��ʽ˳�򣬲������д��
	//д����
	virtual bool Write(void * pValue);
	//���ݲ��κţ�д������
	virtual bool WWrite(void * pValue, int band);
	//����DIMS��Ϣ
	DIMS m_DimsIn;
	//���DIMS��Ϣ
	DIMS m_DimsOut;
	//���������ʾ��Ϣ
	char pcErrMessage[64];

	//������仺��ռ䣬���ҷ��ػ���ָ�롣ÿһ��������������Ϊ���渳ֵ�����������õ�����
	virtual BYTE * RequestCache(SimDIMS * &pSBlockInf);
	//�����ļ�����ָ��
	CGDALFileManager * m_MgrIn;
	//����ļ�����ָ��
	CGDALFileManager * m_MgrOut;
	bool m_bIsReadSample;
	bool m_OutInMem;
	//�����ֽڳ��ȣ�pixel tO length
	int m_nPtLength;
	//������ݳ���
	int m_nPtLengthOut;

	int m_nOffsetY;
	int m_nOffsetX;

	//���뻺������С
	int m_InBufferSize;
	//�����������С
	int m_OutBufferSize;

	//����һ����������Ϣ
	SimDIMS m_SBlockInInf;
	//���������������Ϣ
	SimDIMS m_SBlockInInf2;

	//����һ��������
	SimDIMS m_SBlockIn;
	//�������������
	SimDIMS m_SBlockIn2;

	//����һ��������
	BYTE * m_bBlockIn;
	//�������������
	BYTE * m_bBlockIn2;
	//д���ڴ滺������һ��������
	BYTE * m_bBlockOut;
	//д���ڴ滺����������������
	BYTE * m_bBlockOut2;

	//���������Ԫ��,һ��д������״̬��ǣ�һ��д������״̬��ǣ�-1��ʾ������������Ч״̬
	int m_WriteDoneCount;
	//���������Ԫ��,����д������״̬��ǣ�һ��д������״̬��ǣ�-1��ʾ������������Ч״̬
	int m_WriteDoneCount2;

	//����ĺŻ�������������
	int m_iRejectCacheFlag;
	//0Ϊwrite,1Ϊwwrite
	int m_writeFlag;
	//�����ַ�����λ��
	int m_writePos;


};